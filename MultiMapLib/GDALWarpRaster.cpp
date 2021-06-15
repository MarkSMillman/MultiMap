#pragma warning ( disable : 4251 )
#include "MultiMap.h"
#include "GDALWarpRaster.h"
#include "gdal_priv.h"
#include "gdal_alg.h"
#include "gdalwarper.h"
#include "cpl_string.h"
#include "ogr_spatialref.h"
#include "ogr_api.h"

#ifdef _DEBUG
#pragma warning ( disable : 4189 )
#endif
#pragma warning ( disable : 4996 ) // ignore POSIX deprecation warning

MULTIMAP_API GDALWarpRaster::~GDALWarpRaster(void) {
}

MULTIMAP_API GDALWarpRaster::GDALWarpRaster(void) {
	// public:
	hSrcDS = 0;
	hDstDS = 0;
	hDstDriver = 0;
	pszFormat = "GTiff";
	papszSrcFiles = NULL;
	pszDstFilename = NULL;
	bCreateOutput = FALSE;
	hTransformArg = NULL;
	hGenImgProjArg=NULL;
	hApproxArg=NULL;
	papszWarpOptions = NULL;
	dfErrorThreshold = 0.125;
	dfWarpMemoryLimit = 0.0;
	pfnTransformer = NULL;
	papszCreateOptions = NULL;
	eOutputType = GDT_Unknown;
	eWorkingType = GDT_Unknown; 
	eResampleAlg = GRA_CubicSpline;
	pszSrcNodata = NULL;
	pszDstNodata = NULL;
	bMulti = FALSE;
	papszTO = NULL;
	pszCutlineDSName = NULL;
	pszCLayer = NULL;
	pszCWHERE = NULL;
	pszCSQL = NULL;
	hCutline = NULL;
	bHasGotErr = FALSE;
	bCropToCutline = FALSE;
	bOverwrite = FALSE;

	// private:
	dfMinX=0.0, dfMinY=0.0, dfMaxX=0.0, dfMaxY=0.0;
	dfXRes=0.0, dfYRes=0.0;
	bTargetAlignedPixels = FALSE;
	nForcePixels=0, nForceLines=0, bQuiet = TRUE;
	bEnableDstAlpha = FALSE, bEnableSrcAlpha = FALSE;
	bVRT = FALSE;
	bInitDestSetForFirst = FALSE;
}
MULTIMAP_API int GDALWarpRaster::WarpRaster(void) {
/* -------------------------------------------------------------------- */
/*      If not, we need to create it.                                   */
/* -------------------------------------------------------------------- */
   
    if( hDstDS == NULL )
    {
        hDstDS = GDALWarpCreateOutput( papszSrcFiles, pszDstFilename,pszFormat,
                                       papszTO, &papszCreateOptions, eOutputType );
        bCreateOutput = TRUE;

        if( CSLFetchNameValue( papszWarpOptions, "INIT_DEST" ) == NULL && pszDstNodata == NULL )
        {
            papszWarpOptions = CSLSetNameValue(papszWarpOptions, "INIT_DEST", "0");
            bInitDestSetForFirst = TRUE;
        }
        else if( CSLFetchNameValue( papszWarpOptions, "INIT_DEST" ) == NULL )
        {
            papszWarpOptions = CSLSetNameValue(papszWarpOptions, "INIT_DEST", "NO_DATA" );
            bInitDestSetForFirst = TRUE;
        }

        CSLDestroy( papszCreateOptions );
        papszCreateOptions = NULL;
    }

    if( hDstDS == NULL )
        return 1;

/* -------------------------------------------------------------------- */
/*      Loop over all source files, processing each in turn.            */
/* -------------------------------------------------------------------- */
	if ( hSrcDS != NULL ) {
		ProcessSourceDS(hSrcDS, -1);
	} else {
		int iSrc;

		for( iSrc = 0; papszSrcFiles[iSrc] != NULL; iSrc++ )
		{

			/* -------------------------------------------------------------------- */
			/*      Open this file.                                                 */
			/* -------------------------------------------------------------------- */
			hSrcDS = GDALOpen( papszSrcFiles[iSrc], GA_ReadOnly );

			if( hSrcDS == NULL )
				return 2;
#ifdef _DEBUG
			int eErr = 
#endif
				ProcessSourceDS(hSrcDS, iSrc);

			GDALClose( hSrcDS );
		}
	}
/* -------------------------------------------------------------------- */
/*      Final Cleanup.                                                  */
/* -------------------------------------------------------------------- */
    CPLErrorReset();
    GDALFlushCache( hDstDS );

    if( CPLGetLastErrorType() != CE_None )
        bHasGotErr = TRUE;

	//const char* szDescr = ((GDALDataset*)hDstDS)->GetDescription();
	//if ( strcmp(szDescr,"MEM") != 0 ) {
	//	GDALClose(hDstDS);
	//}
    
    //CPLFree( pszDstFilename );
    CSLDestroy( papszSrcFiles ); papszSrcFiles = NULL;
    CSLDestroy( papszWarpOptions ); papszWarpOptions = NULL;
    CSLDestroy( papszTO ); papszTO = NULL;

    GDALDumpOpenDatasets( stderr );

    //GDALDestroyDriverManager();
    
#ifdef OGR_ENABLED
    if( hCutline != NULL )
        OGR_G_DestroyGeometry( (OGRGeometryH) hCutline );
    OGRCleanupAll();
#endif

    return (bHasGotErr) ? 1 : 0;
}
MULTIMAP_API void GDALWarpRaster::CloseDstDS(void) {
	if ( hDstDS != 0 ) {
		GDALClose(hDstDS);
		hDstDS = 0;
	}
}
MULTIMAP_API int GDALWarpRaster::WarpRaster(GDALDriver* pDstDriver, std::string dstFile, std::string dstWKT, GDALDataset* pSrcDS, GDALResampleAlg eResampleAlg) {

	setDstDriver(pDstDriver);
	setDstFile((char*)dstFile.c_str());
	if ( dstWKT.length() > 0 ) {
	    setTSrs((char*)dstWKT.c_str());
	} else {
		setTSrs((char*)pSrcDS->GetProjectionRef());
	}
	setSrcDataset((GDALDatasetH)pSrcDS);
	setR(eResampleAlg);

	if ( dfXRes == 0.0 || dfYRes == 0.0  ) {
		// Retain resolution change size
		setTr((GDALDatasetH)pSrcDS);
	} else if ( dfXRes < 0.0 || dfYRes < 0.0  ) {
		// Retain size change resolution
		dfXRes = 0.0;
		dfYRes = 0.0;
	}

	if ( pszDstNodata == NULL ) {
		setDstNodata("\"-32767\"");
	}

	return processWarp();
}

void GDALWarpRaster::Usage() {
    printf( 
        "Usage: gdalwarp [--help-general] [--formats]\n"
        "    [-s_srs srs_def] [-t_srs srs_def] [-to \"NAME=VALUE\"]\n"
        "    [-order n] [-tps] [-rpc] [-geoloc] [-et err_threshold]\n"
        "    [-te xmin ymin xmax ymax] [-tr xres yres] [-tap] [-ts width height]\n"
        "    [-wo \"NAME=VALUE\"] [-ot Byte/Int16/...] [-wt Byte/Int16]\n"
        "    [-srcnodata \"value [value...]\"] [-dstnodata \"value [value...]\"] -dstalpha\n" 
        "    [-r resampling_method] [-wm memory_in_mb] [-multi] [-q]\n"
        "    [-cutline datasource] [-cl layer] [-cwhere expression]\n"
        "    [-csql statement] [-cblend dist_in_pixels]\n"
        "    [-of format] [-co \"NAME=VALUE\"]* [-overwrite]\n"
        "    srcfile* dstfile\n"
        "\n"
        "Available resampling methods:\n"
        "    near (default), bilinear, cubic, cubicspline, lanczos.\n" );
}

char* GDALWarpRaster::SanitizeSRS( const char *pszUserInput ) {
    OGRSpatialReferenceH hSRS;
    char *pszResult = NULL;

    CPLErrorReset();
    
    hSRS = OSRNewSpatialReference( NULL );
    if( OSRSetFromUserInput( hSRS, pszUserInput ) == OGRERR_NONE )
        OSRExportToWkt( hSRS, &pszResult );
    else
    {
        CPLError( CE_Failure, CPLE_AppDefined, "Translating source or target SRS failed:%s", pszUserInput );
        return NULL;
    }
    
    OSRDestroySpatialReference( hSRS );

    return pszResult;
}

MULTIMAP_API int GDALWarpRaster::processCutline(void) {
	if ( bCropToCutline && hCutline != NULL )
	{
		OGRGeometryH hCutlineGeom = OGR_G_Clone( (OGRGeometryH) hCutline );
		OGRSpatialReferenceH hCutlineSRS = OGR_G_GetSpatialReference( hCutlineGeom );
		const char *pszThisTargetSRS = CSLFetchNameValue( papszTO, "DST_SRS" );
		OGRCoordinateTransformationH hCT = NULL;
		if (hCutlineSRS == NULL)
		{
			/* We suppose it is in target coordinates */
		}
		else if (pszThisTargetSRS != NULL)
		{
			OGRSpatialReferenceH hTargetSRS = OSRNewSpatialReference(NULL);
			if( OSRImportFromWkt( hTargetSRS, (char **)&pszThisTargetSRS ) != CE_None )
			{
				CPLError( CE_Failure, CPLE_AppDefined, "Cannot compute bounding box of cutline.\n");
				return 1;
			}

			hCT = OCTNewCoordinateTransformation(hCutlineSRS, hTargetSRS);

			OSRDestroySpatialReference(hTargetSRS);
		}
		else if (pszThisTargetSRS == NULL)
		{
			if (papszSrcFiles[0] != NULL || hSrcDS != NULL)
			{
				bool srcDSCreated = false;
				if ( hSrcDS == NULL ) {
					hSrcDS = GDALOpen(papszSrcFiles[0], GA_ReadOnly);
					if (hSrcDS == NULL)
					{
						CPLError( CE_Failure, CPLE_AppDefined, "Cannot compute bounding box of cutline.\n");
						return 1;
					}
					srcDSCreated = true;
				}
				OGRSpatialReferenceH  hRasterSRS = NULL;
				const char *pszProjection = NULL;

				if( GDALGetProjectionRef( hSrcDS ) != NULL && strlen(GDALGetProjectionRef( hSrcDS )) > 0 )
					pszProjection = GDALGetProjectionRef( hSrcDS );
				else if( GDALGetGCPProjection( hSrcDS ) != NULL )
					pszProjection = GDALGetGCPProjection( hSrcDS );

				if( pszProjection == NULL )
				{
					CPLError( CE_Failure, CPLE_AppDefined, "Cannot compute bounding box of cutline.\n");
					return 1;
				}

				hRasterSRS = OSRNewSpatialReference(NULL);
				if( OSRImportFromWkt( hRasterSRS, (char **)&pszProjection ) != CE_None )
				{
					CPLError( CE_Failure, CPLE_AppDefined, "Cannot compute bounding box of cutline.\n");
					return 1;
				}

				hCT = OCTNewCoordinateTransformation(hCutlineSRS, hRasterSRS);

				OSRDestroySpatialReference(hRasterSRS);

				if ( srcDSCreated ) 
					GDALClose(hSrcDS);
			}
			else
			{
				CPLError( CE_Failure, CPLE_AppDefined, "Cannot compute bounding box of cutline.\n");
				return 1;
			}
		}

		if (hCT)
		{
			OGR_G_Transform( hCutlineGeom, hCT );
			OCTDestroyCoordinateTransformation(hCT);
		}

		OGREnvelope sEnvelope;
		OGR_G_GetEnvelope(hCutlineGeom, &sEnvelope);

		dfMinX = sEnvelope.MinX;
		dfMinY = sEnvelope.MinY;
		dfMaxX = sEnvelope.MaxX;
		dfMaxY = sEnvelope.MaxY;

		OGR_G_DestroyGeometry(hCutlineGeom);
	}
    return 0;
}

MULTIMAP_API int GDALWarpRaster::processWarp() {
	/* -------------------------------------------------------------------- *//*      Does the output dataset already exist?                          */
	/* -------------------------------------------------------------------- */
	CPLPushErrorHandler( CPLQuietErrorHandler );
	hDstDS = GDALOpen( pszDstFilename, GA_Update );
	CPLPopErrorHandler();

	if( hDstDS != NULL && bOverwrite )
	{
		GDALClose(hDstDS);
		hDstDS = NULL;
	}

	if( hDstDS != NULL && bCreateOutput )
	{
		CPLError( CE_Failure, CPLE_IllegalArg, 
			"Output dataset %s exists,\n"
			"but some commandline options were provided indicating a new dataset\n"
			"should be created.  Please delete existing dataset and run again.\n",
			pszDstFilename );
		return 1;
	}

	/* Avoid overwriting an existing destination file that cannot be opened in */
	/* update mode with a new GTiff file */
	if ( hDstDS == NULL && !bOverwrite )
	{
		CPLPushErrorHandler( CPLQuietErrorHandler );
		hDstDS = GDALOpen( pszDstFilename, GA_ReadOnly );
		CPLPopErrorHandler();

		if (hDstDS)
		{
			CPLError( CE_Failure, CPLE_IllegalArg, 
				"Output dataset %s exists, but cannot be opened in update mode\n",
				pszDstFilename );
			GDALClose(hDstDS);
			return 1;
		}
	}

	/* -------------------------------------------------------------------- */
	/*      If we have a cutline datasource read it and attach it in the    */
	/*      warp options.                                                   */
	/* -------------------------------------------------------------------- */
	if( pszCutlineDSName != NULL )
	{
		LoadCutline( pszCutlineDSName, pszCLayer, pszCWHERE, pszCSQL, &hCutline );
	}

#ifdef OGR_ENABLED
	processCutline();
#endif
	return WarpRaster();
}
int GDALWarpRaster::ProcessSourceDS(GDALDatasetH hSourceDS, int _iSrc) {
	int iSrc = _iSrc;
	/*
	GDALDataset* pSrcDataset = (GDALDataset*)hSourceDS;
	char** srcMetadata = pSrcDataset->GetMetadata("");
	int ii=0;
	char* m = NULL;
	while ( srcMetadata[ii] ) {
		m = srcMetadata[ii];
	}

	char** srcFileList = pSrcDataset->GetFileList();
	char* f = NULL;
	ii = 0;
	while ( srcFileList[ii] ) {
		f = srcFileList[ii];
	}
	*/
/* -------------------------------------------------------------------- */
/*      Check that there's at least one raster band                     */
/* -------------------------------------------------------------------- */
        if ( GDALGetRasterCount(hSrcDS) == 0 )
        {
            CPLError( CE_Failure, CPLE_AppDefined, "Input file %s has no raster bands.\n", papszSrcFiles[iSrc] );
            return 1;
        }

        if( !bQuiet )
            printf( "Processing input file %s.\n", papszSrcFiles[iSrc] );

/* -------------------------------------------------------------------- */
/*      Warns if the file has a color table and something more          */
/*      complicated than nearest neighbour resampling is asked          */
/* -------------------------------------------------------------------- */

        if ( eResampleAlg != GRA_NearestNeighbour &&
             GDALGetRasterColorTable(GDALGetRasterBand(hSrcDS, 1)) != NULL)
        {
            if( !bQuiet )
                CPLError( CE_Failure, CPLE_AppDefined, "Warning: Input file %s has a color table, which will likely lead to "
                        "bad results when using a resampling method other than "
                        "nearest neighbour. Converting the dataset prior to 24/32 bit "
                        "is advised.\n", papszSrcFiles[iSrc] );
        }

/* -------------------------------------------------------------------- */
/*      Do we have a source alpha band?                                 */
/* -------------------------------------------------------------------- */
        if( GDALGetRasterColorInterpretation( GDALGetRasterBand(hSrcDS,GDALGetRasterCount(hSrcDS)) ) == GCI_AlphaBand && !bEnableSrcAlpha )
        {
            bEnableSrcAlpha = TRUE;
            if( !bQuiet )
                printf( "Using band %d of source image as alpha.\n", 
                        GDALGetRasterCount(hSrcDS) );
        }

/* -------------------------------------------------------------------- */
/*      Create a transformation object from the source to               */
/*      destination coordinate system.                                  */
/* -------------------------------------------------------------------- */
        hTransformArg = hGenImgProjArg = GDALCreateGenImgProjTransformer2( hSrcDS, hDstDS, papszTO );
        
        if( hTransformArg == NULL )
            return 1;
        
        pfnTransformer = GDALGenImgProjTransform;

/* -------------------------------------------------------------------- */
/*      Warp the transformer with a linear approximator unless the      */
/*      acceptable error is zero.                                       */
/* -------------------------------------------------------------------- */
        if( dfErrorThreshold != 0.0 )
        {
            hTransformArg = hApproxArg = GDALCreateApproxTransformer( GDALGenImgProjTransform, hGenImgProjArg, dfErrorThreshold);
            pfnTransformer = GDALApproxTransform;
        }

/* -------------------------------------------------------------------- */
/*      Clear temporary INIT_DEST settings after the first image.       */
/* -------------------------------------------------------------------- */
        if( bInitDestSetForFirst && iSrc == 1 )
            papszWarpOptions = CSLSetNameValue( papszWarpOptions, "INIT_DEST", NULL );

/* -------------------------------------------------------------------- */
/*      Setup warp options.                                             */
/* -------------------------------------------------------------------- */
        GDALWarpOptions *psWO = GDALCreateWarpOptions();

        psWO->papszWarpOptions = CSLDuplicate(papszWarpOptions);
        psWO->eWorkingDataType = eWorkingType;
        psWO->eResampleAlg = eResampleAlg;

        psWO->hSrcDS = hSrcDS;
        psWO->hDstDS = hDstDS;

        psWO->pfnTransformer = pfnTransformer;
        psWO->pTransformerArg = hTransformArg;

        if( !bQuiet )
            psWO->pfnProgress = GDALTermProgress;

        if( dfWarpMemoryLimit != 0.0 )
            psWO->dfWarpMemoryLimit = dfWarpMemoryLimit;

/* -------------------------------------------------------------------- */
/*      Setup band mapping.                                             */
/* -------------------------------------------------------------------- */
        if( bEnableSrcAlpha )
            psWO->nBandCount = GDALGetRasterCount(hSrcDS) - 1;
        else
            psWO->nBandCount = GDALGetRasterCount(hSrcDS);

        psWO->panSrcBands = (int *) CPLMalloc(psWO->nBandCount*sizeof(int));
        psWO->panDstBands = (int *) CPLMalloc(psWO->nBandCount*sizeof(int));

        for( i = 0; i < psWO->nBandCount; i++ )
        {
            psWO->panSrcBands[i] = i+1;
            psWO->panDstBands[i] = i+1;
        }

/* -------------------------------------------------------------------- */
/*      Setup alpha bands used if any.                                  */
/* -------------------------------------------------------------------- */
        if( bEnableSrcAlpha )
            psWO->nSrcAlphaBand = GDALGetRasterCount(hSrcDS);

        if( !bEnableDstAlpha 
            && GDALGetRasterCount(hDstDS) == psWO->nBandCount+1 
            && GDALGetRasterColorInterpretation( GDALGetRasterBand(hDstDS,GDALGetRasterCount(hDstDS))) == GCI_AlphaBand )
        {
            if( !bQuiet )
                printf( "Using band %d of destination image as alpha.\n", GDALGetRasterCount(hDstDS) );
                
            bEnableDstAlpha = TRUE;
        }

        if( bEnableDstAlpha )
            psWO->nDstAlphaBand = GDALGetRasterCount(hDstDS);

/* -------------------------------------------------------------------- */
/*      Setup NODATA options.                                           */
/* -------------------------------------------------------------------- */
        if( pszSrcNodata != NULL && !EQUALN(pszSrcNodata,"n",1) )
        {
            char **papszTokens = CSLTokenizeString( pszSrcNodata );
            int  nTokenCount = CSLCount(papszTokens);

            psWO->padfSrcNoDataReal = (double *) CPLMalloc(psWO->nBandCount*sizeof(double));
            psWO->padfSrcNoDataImag = (double *) CPLMalloc(psWO->nBandCount*sizeof(double));

            for( i = 0; i < psWO->nBandCount; i++ )
            {
                if( i < nTokenCount )
                {
                    CPLStringToComplex( papszTokens[i], 
                                        psWO->padfSrcNoDataReal + i,
                                        psWO->padfSrcNoDataImag + i );
                }
                else
                {
                    psWO->padfSrcNoDataReal[i] = psWO->padfSrcNoDataReal[i-1];
                    psWO->padfSrcNoDataImag[i] = psWO->padfSrcNoDataImag[i-1];
                }
            }

            CSLDestroy( papszTokens );

            psWO->papszWarpOptions = CSLSetNameValue(psWO->papszWarpOptions,
                                               "UNIFIED_SRC_NODATA", "YES" );
        }

/* -------------------------------------------------------------------- */
/*      If -srcnodata was not specified, but the data has nodata        */
/*      values, use them.                                               */
/* -------------------------------------------------------------------- */
        if( pszSrcNodata == NULL )
        {
            int bHaveNodata = FALSE;
            double dfReal = 0.0;

            for( i = 0; !bHaveNodata && i < psWO->nBandCount; i++ )
            {
                GDALRasterBandH hBand = GDALGetRasterBand( hSrcDS, i+1 );
                dfReal = GDALGetRasterNoDataValue( hBand, &bHaveNodata );
            }

            if( bHaveNodata )
            {
                if( !bQuiet )
                {
                    if (CPLIsNan(dfReal))
                        printf( "Using internal nodata values (eg. nan) for image %s.\n",
                                papszSrcFiles[iSrc] );
                    else
                        printf( "Using internal nodata values (eg. %g) for image %s.\n",
                                dfReal, papszSrcFiles[iSrc] );
                }
                psWO->padfSrcNoDataReal = (double *) 
                    CPLMalloc(psWO->nBandCount*sizeof(double));
                psWO->padfSrcNoDataImag = (double *) 
                    CPLMalloc(psWO->nBandCount*sizeof(double));
                
                for( i = 0; i < psWO->nBandCount; i++ )
                {
                    GDALRasterBandH hBand = GDALGetRasterBand( hSrcDS, i+1 );

                    dfReal = GDALGetRasterNoDataValue( hBand, &bHaveNodata );

                    if( bHaveNodata )
                    {
                        psWO->padfSrcNoDataReal[i] = dfReal;
                        psWO->padfSrcNoDataImag[i] = 0.0;
                    }
                    else
                    {
                        psWO->padfSrcNoDataReal[i] = -123456.789;
                        psWO->padfSrcNoDataImag[i] = 0.0;
                    }
                }
            }
        }

/* -------------------------------------------------------------------- */
/*      If the output dataset was created, and we have a destination    */
/*      nodata value, go through marking the bands with the information.*/
/* -------------------------------------------------------------------- */
        if( pszDstNodata != NULL )
        {
            char **papszTokens = CSLTokenizeString( pszDstNodata );
            int  nTokenCount = CSLCount(papszTokens);

            psWO->padfDstNoDataReal = (double *) 
                CPLMalloc(psWO->nBandCount*sizeof(double));
            psWO->padfDstNoDataImag = (double *) 
                CPLMalloc(psWO->nBandCount*sizeof(double));

            for( i = 0; i < psWO->nBandCount; i++ )
            {
                if( i < nTokenCount )
                {
                    CPLStringToComplex( papszTokens[i], 
                                        psWO->padfDstNoDataReal + i,
                                        psWO->padfDstNoDataImag + i );
                }
                else
                {
                    psWO->padfDstNoDataReal[i] = psWO->padfDstNoDataReal[i-1];
                    psWO->padfDstNoDataImag[i] = psWO->padfDstNoDataImag[i-1];
                }
                
                GDALRasterBandH hBand = GDALGetRasterBand( hDstDS, i+1 );
                int bClamped = FALSE, bRounded = FALSE;

#define CLAMP(val,type,minval,maxval) \
    do { if (val < minval) { bClamped = TRUE; val = minval; } \
    else if (val > maxval) { bClamped = TRUE; val = maxval; } \
    else if (val != (type)val) { bRounded = TRUE; val = (type)(val + 0.5); } } \
    while(0)

                switch(GDALGetRasterDataType(hBand))
                {
                    case GDT_Byte:
                        CLAMP(psWO->padfDstNoDataReal[i], GByte,
                              0.0, 255.0);
                        break;
                    case GDT_Int16:
                        CLAMP(psWO->padfDstNoDataReal[i], GInt16,
                              -32768.0, 32767.0);
                        break;
                    case GDT_UInt16:
                        CLAMP(psWO->padfDstNoDataReal[i], GUInt16,
                              0.0, 65535.0);
                        break;
                    case GDT_Int32:
                        CLAMP(psWO->padfDstNoDataReal[i], GInt32,
                              -2147483648.0, 2147483647.0);
                        break;
                    case GDT_UInt32:
                        CLAMP(psWO->padfDstNoDataReal[i], GUInt32,
                              0.0, 4294967295.0);
                        break;
                    default:
                        break;
                }
                    
                if (bClamped)
                {
                    printf( "for band %d, destination nodata value has been clamped "
                           "to %.0f, the original value being out of range.\n",
                           i + 1, psWO->padfDstNoDataReal[i]);
                }
                else if(bRounded)
                {
                    printf("for band %d, destination nodata value has been rounded "
                           "to %.0f, %s being an integer datatype.\n",
                           i + 1, psWO->padfDstNoDataReal[i],
                           GDALGetDataTypeName(GDALGetRasterDataType(hBand)));
                }

                if( bCreateOutput )
                {
                    GDALSetRasterNoDataValue( 
                        GDALGetRasterBand( hDstDS, psWO->panDstBands[i] ), 
                        psWO->padfDstNoDataReal[i] );
                }
            }

            CSLDestroy( papszTokens );
        }

/* -------------------------------------------------------------------- */
/*      If we have a cutline, transform it into the source              */
/*      pixel/line coordinate system and insert into warp options.      */
/* -------------------------------------------------------------------- */
        if( hCutline != NULL )
        {
            TransformCutlineToSource( hSrcDS, hCutline, 
                                      &(psWO->papszWarpOptions), 
                                      papszTO );
        }

/* -------------------------------------------------------------------- */
/*      If we are producing VRT output, then just initialize it with    */
/*      the warp options and write out now rather than proceeding       */
/*      with the operations.                                            */
/* -------------------------------------------------------------------- */
        if( bVRT )
        {
            if( GDALInitializeWarpedVRT( hDstDS, psWO ) != CE_None )
                return 1;

            GDALClose( hDstDS );
            GDALClose( hSrcDS );

            /* The warped VRT will clean itself the transformer used */
            /* So we have only to destroy the hGenImgProjArg if we */
            /* have wrapped it inside the hApproxArg */
            if (pfnTransformer == GDALApproxTransform)
            {
                if( hGenImgProjArg != NULL )
                    GDALDestroyGenImgProjTransformer( hGenImgProjArg );
            }

            GDALDestroyWarpOptions( psWO );

            CPLFree( pszDstFilename );
            CSLDestroy( papszSrcFiles );
            CSLDestroy( papszWarpOptions );
            CSLDestroy( papszTO );
    
            GDALDumpOpenDatasets( stderr );
            //GDALDestroyDriverManager();
        
            return 0;
        }

/* -------------------------------------------------------------------- */
/*      Initialize and execute the warp.                                */
/* -------------------------------------------------------------------- */
        GDALWarpOperation oWO;

        if( oWO.Initialize( psWO ) == CE_None )
        {
            CPLErr eErr;
            if( bMulti )
                eErr = oWO.ChunkAndWarpMulti( 0, 0, 
                                       GDALGetRasterXSize( hDstDS ),
                                       GDALGetRasterYSize( hDstDS ) );
            else
                eErr = oWO.ChunkAndWarpImage( 0, 0, 
                                       GDALGetRasterXSize( hDstDS ),
                                       GDALGetRasterYSize( hDstDS ) );
            if (eErr != CE_None)
                bHasGotErr = TRUE;
        }

/* -------------------------------------------------------------------- */
/*      Cleanup                                                         */
/* -------------------------------------------------------------------- */
        if( hApproxArg != NULL )
            GDALDestroyApproxTransformer( hApproxArg );
        
        if( hGenImgProjArg != NULL )
            GDALDestroyGenImgProjTransformer( hGenImgProjArg );
        
        GDALDestroyWarpOptions( psWO );

		return 0;
}

int GDALWarpRaster::PreprocessSourceDS(GDALDatasetH & hSrcDS, GDALDataType & eDT, int & nDstBandCount, GDALColorTableH & hCT, char *pszThisTargetSRS, char* srcFile, int iSrc) {
	const char *pszThisSourceSRS = CSLFetchNameValue(papszTO,"SRC_SRS");
    void *hTransformArg;
	double srcGeoTransform[6];
	/* -------------------------------------------------------------------- */
    /*      Check that there's at least one raster band                     */
    /* -------------------------------------------------------------------- */
        if ( GDALGetRasterCount(hSrcDS) == 0 )
        {
            CPLError( CE_Failure, CPLE_AppDefined, "Input file %s has no raster bands.", srcFile );
            return 1;
        }

        if( eDT == GDT_Unknown )
            eDT = GDALGetRasterDataType(GDALGetRasterBand(hSrcDS,1));

/* -------------------------------------------------------------------- */
/*      If we are processing the first file, and it has a color         */
/*      table, then we will copy it to the destination file.            */
/* -------------------------------------------------------------------- */
        if( iSrc <= 0 )
        {
            nDstBandCount = GDALGetRasterCount(hSrcDS);
			GDALGetGeoTransform(hSrcDS,srcGeoTransform);
            hCT = GDALGetRasterColorTable( GDALGetRasterBand(hSrcDS,1) );
            if( hCT != NULL )
            {
                hCT = GDALCloneColorTable( hCT );
                if( !bQuiet )
                    printf( "Copying color table from %s to new file.", srcFile );
            }
        }

/* -------------------------------------------------------------------- */
/*      Get the sourcesrs from the dataset, if not set already.         */
/* -------------------------------------------------------------------- */
        if( pszThisSourceSRS == NULL )
        {
            const char *pszMethod = CSLFetchNameValue( papszTO, "METHOD" );

            if( GDALGetProjectionRef( hSrcDS ) != NULL 
                && strlen(GDALGetProjectionRef( hSrcDS )) > 0
                && (pszMethod == NULL || EQUAL(pszMethod,"GEOTRANSFORM")) )
                pszThisSourceSRS = GDALGetProjectionRef( hSrcDS );
            
            else if( GDALGetGCPProjection( hSrcDS ) != NULL
                     && strlen(GDALGetGCPProjection(hSrcDS)) > 0 
                     && GDALGetGCPCount( hSrcDS ) > 1 
                     && (pszMethod == NULL || EQUALN(pszMethod,"GCP_",4)) )
                pszThisSourceSRS = GDALGetGCPProjection( hSrcDS );
            else if( pszMethod != NULL && EQUAL(pszMethod,"RPC") )
                pszThisSourceSRS = SRS_WKT_WGS84;
            else
                pszThisSourceSRS = "";
        }

        if( pszThisTargetSRS == NULL )
            pszThisTargetSRS = CPLStrdup( pszThisSourceSRS );
        
/* -------------------------------------------------------------------- */
/*      Create a transformation object from the source to               */
/*      destination coordinate system.                                  */
/* -------------------------------------------------------------------- */
        hTransformArg = GDALCreateGenImgProjTransformer2( hSrcDS, NULL, papszTO );
        
        if( hTransformArg == NULL )
        {
            CPLFree( pszThisTargetSRS );
			if ( iSrc >= 0 )
                GDALClose( hSrcDS );
            return 1;
        }

/* -------------------------------------------------------------------- */
/*      Get approximate output definition.                              */
/* -------------------------------------------------------------------- */
        double adfThisGeoTransform[6];
        double adfExtent[4];
        int    nThisPixels, nThisLines;

        if( GDALSuggestedWarpOutput2( hSrcDS, 
                                      GDALGenImgProjTransform, hTransformArg, 
                                      adfThisGeoTransform, 
                                      &nThisPixels, &nThisLines, 
                                      adfExtent, 0 ) != CE_None )
        {
            CPLFree( pszThisTargetSRS );
			if ( iSrc >= 0 )
                GDALClose( hSrcDS );
            return 1;
        }

        if (CPLGetConfigOption( "CHECK_WITH_INVERT_PROJ", NULL ) == NULL)
        {
            double MinX = adfExtent[0];
            double MaxX = adfExtent[2];
            double MaxY = adfExtent[3];
            double MinY = adfExtent[1];
						
            int bSuccess = TRUE;
            
            /* Check that the the edges of the target image are in the validity area */
            /* of the target projection */
#define N_STEPS 20
            int i,j;
            for(i=0;i<=N_STEPS && bSuccess;i++)
            {
                for(j=0;j<=N_STEPS && bSuccess;j++)
                {
                    double dfRatioI = i * 1.0 / N_STEPS;
                    double dfRatioJ = j * 1.0 / N_STEPS;
                    double expected_x = (1 - dfRatioI) * MinX + dfRatioI * MaxX;
                    double expected_y = (1 - dfRatioJ) * MinY + dfRatioJ * MaxY;
                    double x = expected_x;
                    double y = expected_y;
                    double z = 0;
                    /* Target SRS coordinates to source image pixel coordinates */
                    if (!GDALGenImgProjTransform(hTransformArg, TRUE, 1, &x, &y, &z, &bSuccess) || !bSuccess)
                        bSuccess = FALSE;
                    /* Source image pixel coordinates to target SRS coordinates */
                    if (!GDALGenImgProjTransform(hTransformArg, FALSE, 1, &x, &y, &z, &bSuccess) || !bSuccess)
                        bSuccess = FALSE;
                    if (fabs(x - expected_x) > (MaxX - MinX) / nThisPixels ||
                        fabs(y - expected_y) > (MaxY - MinY) / nThisLines)
                        bSuccess = FALSE;
                }
            }
            
            /* If not, retry with CHECK_WITH_INVERT_PROJ=TRUE that forces ogrct.cpp */
            /* to check the consistency of each requested projection result with the */
            /* invert projection */
            if (!bSuccess)
            {
                CPLSetConfigOption( "CHECK_WITH_INVERT_PROJ", "TRUE" );
                CPLDebug("WARP", "Recompute out extent with CHECK_WITH_INVERT_PROJ=TRUE");
                GDALDestroyGenImgProjTransformer(hTransformArg);
                hTransformArg = GDALCreateGenImgProjTransformer2( hSrcDS, NULL, papszTO );
                    
                if( GDALSuggestedWarpOutput2( hSrcDS, 
                                      GDALGenImgProjTransform, hTransformArg, 
                                      adfThisGeoTransform, 
                                      &nThisPixels, &nThisLines, 
                                      adfExtent, 0 ) != CE_None )
				{
					CPLFree( pszThisTargetSRS );
					if ( iSrc >= 0 )
						GDALClose( hSrcDS );
					return 1;
				}
            }
        }

/* -------------------------------------------------------------------- */
/*      Expand the working bounds to include this region, ensure the    */
/*      working resolution is no more than this resolution.             */
/* -------------------------------------------------------------------- */
        if( dfWrkMaxX == 0.0 && dfWrkMinX == 0.0 )
        {
            dfWrkMinX = adfExtent[0];
            dfWrkMaxX = adfExtent[2];
            dfWrkMaxY = adfExtent[3];
            dfWrkMinY = adfExtent[1];
            dfWrkResX = adfThisGeoTransform[1];
            dfWrkResY = ABS(adfThisGeoTransform[5]);
			//dfWrkResX = srcGeoTransform[1];
            //dfWrkResY = ABS(srcGeoTransform[5]);
		}
        else
        {
            dfWrkMinX = MIN(dfWrkMinX,adfExtent[0]);
            dfWrkMaxX = MAX(dfWrkMaxX,adfExtent[2]);
            dfWrkMaxY = MAX(dfWrkMaxY,adfExtent[3]);
            dfWrkMinY = MIN(dfWrkMinY,adfExtent[1]);
            dfWrkResX = MIN(dfWrkResX,adfThisGeoTransform[1]);
            dfWrkResY = MIN(dfWrkResY,ABS(adfThisGeoTransform[5]));
            //dfWrkResX = srcGeoTransform[1];
            //dfWrkResY = ABS(srcGeoTransform[5]);
		}
        
        GDALDestroyGenImgProjTransformer( hTransformArg );

		return 0;
}

GDALDatasetH GDALWarpRaster::GDALWarpCreateOutput( char **papszSrcFiles, const char *pszFilename, 
                      const char *pszFormat, char **papszTO, char ***ppapszCreateOptions, GDALDataType eDT )
{
    GDALDatasetH hDstDS;
    GDALColorTableH hCT = NULL;
    dfWrkMinX=0, dfWrkMaxX=0, dfWrkMinY=0, dfWrkMaxY=0;
    dfWrkResX=0, dfWrkResY=0;
    int nDstBandCount = 0;

	if ( hDstDriver == 0 ) {
		/* -------------------------------------------------------------------- */
		/*      Find the output driver.                                         */
		/* -------------------------------------------------------------------- */
		hDstDriver = GDALGetDriverByName( pszFormat );
		if( hDstDriver == NULL || GDALGetMetadataItem( hDstDriver, GDAL_DCAP_CREATE, NULL ) == NULL )
		{
			CPLError( CE_Failure, CPLE_AppDefined, "Output driver `%s' not recognised or does not support.  direct output file creation.  The following format drivers are configured and support direct output", pszFormat );
			return NULL;
		}
	}

/* -------------------------------------------------------------------- */
/*      For virtual output files, we have to set a special subclass     */
/*      of dataset to create.                                           */
/* -------------------------------------------------------------------- */
    if( bVRT )
        *ppapszCreateOptions = CSLSetNameValue( *ppapszCreateOptions, "SUBCLASS", "VRTWarpedDataset" );

/* -------------------------------------------------------------------- */
/*      Loop over all input files to collect extents.                   */
/* -------------------------------------------------------------------- */
    int     iSrc;
    char    *pszThisTargetSRS = (char*)CSLFetchNameValue( papszTO, "DST_SRS" );
    if( pszThisTargetSRS != NULL ) {
        pszThisTargetSRS = CPLStrdup( pszThisTargetSRS );
	}
	if ( hSrcDS ) {
		PreprocessSourceDS(hSrcDS, eDT, nDstBandCount, hCT, pszThisTargetSRS, "", -1);
	} else {
		for( iSrc = 0; papszSrcFiles[iSrc] != NULL; iSrc++ )
		{
			//const char *pszThisSourceSRS = CSLFetchNameValue(papszTO,"SRC_SRS");

			hSrcDS = GDALOpen( papszSrcFiles[iSrc], GA_ReadOnly );
			if( hSrcDS == NULL )
				return NULL;

			PreprocessSourceDS(hSrcDS, eDT, nDstBandCount, hCT, pszThisTargetSRS, papszSrcFiles[iSrc], iSrc);

			GDALClose( hSrcDS );
		}
	}
/* -------------------------------------------------------------------- */
/*      Did we have any usable sources?                                 */
/* -------------------------------------------------------------------- */
    if( nDstBandCount == 0 )
    {
        CPLError( CE_Failure, CPLE_AppDefined, "No usable source images." );
        CPLFree( pszThisTargetSRS );
        return NULL;
    }

/* -------------------------------------------------------------------- */
/*      Turn the suggested region into a geotransform and suggested     */
/*      number of pixels and lines.                                     */
/* -------------------------------------------------------------------- */
    double adfDstGeoTransform[6];
    int nPixels, nLines;

    adfDstGeoTransform[0] = dfWrkMinX;
    adfDstGeoTransform[1] = dfWrkResX;
    adfDstGeoTransform[2] = 0.0;
    adfDstGeoTransform[3] = dfWrkMaxY;
    adfDstGeoTransform[4] = 0.0;
    adfDstGeoTransform[5] = -1 * dfWrkResY;

    nPixels = (int) ((dfWrkMaxX - dfWrkMinX) / dfWrkResX + 0.5);
    nLines = (int) ((dfWrkMaxY - dfWrkMinY) / dfWrkResY + 0.5);

/* -------------------------------------------------------------------- */
/*      Did the user override some parameters?                          */
/* -------------------------------------------------------------------- */
    if( dfXRes != 0.0 && dfYRes != 0.0 )
    {
        if( dfMinX == 0.0 && dfMinY == 0.0 && dfMaxX == 0.0 && dfMaxY == 0.0 )
        {
            dfMinX = adfDstGeoTransform[0];
            dfMaxX = adfDstGeoTransform[0] + adfDstGeoTransform[1] * nPixels;
            dfMaxY = adfDstGeoTransform[3];
            dfMinY = adfDstGeoTransform[3] + adfDstGeoTransform[5] * nLines;
        }
        
        if ( bTargetAlignedPixels )
        {
            dfMinX = floor(dfMinX / dfXRes) * dfXRes;
            dfMaxX = ceil(dfMaxX / dfXRes) * dfXRes;
            dfMinY = floor(dfMinY / dfYRes) * dfYRes;
            dfMaxY = ceil(dfMaxY / dfYRes) * dfYRes;
        }

        nPixels = (int) ((dfMaxX - dfMinX + (dfXRes/2.0)) / dfXRes);
        nLines = (int) ((dfMaxY - dfMinY + (dfYRes/2.0)) / dfYRes);
        adfDstGeoTransform[0] = dfMinX;
        adfDstGeoTransform[3] = dfMaxY;
        adfDstGeoTransform[1] = dfXRes;
        adfDstGeoTransform[5] = -dfYRes;
    }

    else if( nForcePixels != 0 && nForceLines != 0 )
    {
        if( dfMinX == 0.0 && dfMinY == 0.0 && dfMaxX == 0.0 && dfMaxY == 0.0 )
        {
            dfMinX = dfWrkMinX;
            dfMaxX = dfWrkMaxX;
            dfMaxY = dfWrkMaxY;
            dfMinY = dfWrkMinY;
        }

        dfXRes = (dfMaxX - dfMinX) / nForcePixels;
        dfYRes = (dfMaxY - dfMinY) / nForceLines;

        adfDstGeoTransform[0] = dfMinX;
        adfDstGeoTransform[3] = dfMaxY;
        adfDstGeoTransform[1] = dfXRes;
        adfDstGeoTransform[5] = -dfYRes;

        nPixels = nForcePixels;
        nLines = nForceLines;
    }

    else if( nForcePixels != 0 )
    {
        if( dfMinX == 0.0 && dfMinY == 0.0 && dfMaxX == 0.0 && dfMaxY == 0.0 )
        {
            dfMinX = dfWrkMinX;
            dfMaxX = dfWrkMaxX;
            dfMaxY = dfWrkMaxY;
            dfMinY = dfWrkMinY;
        }

        dfXRes = (dfMaxX - dfMinX) / nForcePixels;
        dfYRes = dfXRes;

        adfDstGeoTransform[0] = dfMinX;
        adfDstGeoTransform[3] = dfMaxY;
        adfDstGeoTransform[1] = dfXRes;
        adfDstGeoTransform[5] = -dfYRes;

        nPixels = nForcePixels;
        nLines = (int) ((dfMaxY - dfMinY + (dfYRes/2.0)) / dfYRes);
    }

    else if( nForceLines != 0 )
    {
        if( dfMinX == 0.0 && dfMinY == 0.0 && dfMaxX == 0.0 && dfMaxY == 0.0 )
        {
            dfMinX = dfWrkMinX;
            dfMaxX = dfWrkMaxX;
            dfMaxY = dfWrkMaxY;
            dfMinY = dfWrkMinY;
        }

        dfYRes = (dfMaxY - dfMinY) / nForceLines;
        dfXRes = dfYRes;

        adfDstGeoTransform[0] = dfMinX;
        adfDstGeoTransform[3] = dfMaxY;
        adfDstGeoTransform[1] = dfXRes;
        adfDstGeoTransform[5] = -dfYRes;

        nPixels = (int) ((dfMaxX - dfMinX + (dfXRes/2.0)) / dfXRes);
        nLines = nForceLines;
    }

    else if( dfMinX != 0.0 || dfMinY != 0.0 || dfMaxX != 0.0 || dfMaxY != 0.0 )
    {
        dfXRes = adfDstGeoTransform[1];
        dfYRes = fabs(adfDstGeoTransform[5]);

        nPixels = (int) ((dfMaxX - dfMinX + (dfXRes/2.0)) / dfXRes);
        nLines = (int) ((dfMaxY - dfMinY + (dfYRes/2.0)) / dfYRes);

        dfXRes = (dfMaxX - dfMinX) / nPixels;
        dfYRes = (dfMaxY - dfMinY) / nLines;

        adfDstGeoTransform[0] = dfMinX;
        adfDstGeoTransform[3] = dfMaxY;
        adfDstGeoTransform[1] = dfXRes;
        adfDstGeoTransform[5] = -dfYRes;
    }

/* -------------------------------------------------------------------- */
/*      Do we want to generate an alpha band in the output file?        */
/* -------------------------------------------------------------------- */
    if( bEnableSrcAlpha )
        nDstBandCount--;

    if( bEnableDstAlpha )
        nDstBandCount++;

/* -------------------------------------------------------------------- */
/*      Create the output file.                                         */
/* -------------------------------------------------------------------- */
    if( !bQuiet )
        printf( "Creating output file that is %dP x %dL.\n", nPixels, nLines );

    hDstDS = GDALCreate( hDstDriver, pszFilename, nPixels, nLines, 
                         nDstBandCount, eDT, *ppapszCreateOptions );
    
    if( hDstDS == NULL )
    {
        CPLFree( pszThisTargetSRS );
        return NULL;
    }

/* -------------------------------------------------------------------- */
/*      Write out the projection definition.                            */
/* -------------------------------------------------------------------- */
    GDALSetProjection( hDstDS, pszThisTargetSRS );
    GDALSetGeoTransform( hDstDS, adfDstGeoTransform );

/* -------------------------------------------------------------------- */
/*      Try to set color interpretation of output file alpha band.      */
/*      TODO: We should likely try to copy the other bands too.         */
/* -------------------------------------------------------------------- */
    if( bEnableDstAlpha )
    {
        GDALSetRasterColorInterpretation( 
            GDALGetRasterBand( hDstDS, nDstBandCount ), 
            GCI_AlphaBand );
    }

/* -------------------------------------------------------------------- */
/*      Copy the color table, if required.                              */
/* -------------------------------------------------------------------- */
    if( hCT != NULL )
    {
        GDALSetRasterColorTable( GDALGetRasterBand(hDstDS,1), hCT );
        GDALDestroyColorTable( hCT );
    }

    CPLFree( pszThisTargetSRS );
    return hDstDS;
}

int GDALWarpRaster::LoadCutline( const char *pszCutlineDSName, const char *pszCLayer, 
             const char *pszCWHERE, const char *pszCSQL, void **phCutlineRet )
{
#ifndef OGR_ENABLED
    CPLError( CE_Failure, CPLE_AppDefined, 
              "Request to load a cutline failed, this build does not support OGR features.\n" );
    return 1;
#else // def OGR_ENABLED
    //OGRRegisterAll();

/* -------------------------------------------------------------------- */
/*      Open source vector dataset.                                     */
/* -------------------------------------------------------------------- */

    hSrcDS = OGROpen( pszCutlineDSName, FALSE, NULL );
    if( hSrcDS == NULL )
        return 1;

/* -------------------------------------------------------------------- */
/*      Get the source layer                                            */
/* -------------------------------------------------------------------- */
    OGRLayerH hLayer = NULL;

    if( pszCSQL != NULL )
        hLayer = OGR_DS_ExecuteSQL( hSrcDS, pszCSQL, NULL, NULL ); 
    else if( pszCLayer != NULL )
        hLayer = OGR_DS_GetLayerByName( hSrcDS, pszCLayer );
    else
        hLayer = OGR_DS_GetLayer( hSrcDS, 0 );

    if( hLayer == NULL )
    {
        CPLError( CE_Failure, CPLE_AppDefined, "Failed to identify source layer from datasource.\n" );
        return 1;
    }

/* -------------------------------------------------------------------- */
/*      Apply WHERE clause if there is one.                             */
/* -------------------------------------------------------------------- */
    if( pszCWHERE != NULL )
        OGR_L_SetAttributeFilter( hLayer, pszCWHERE );

/* -------------------------------------------------------------------- */
/*      Collect the geometries from this layer, and build list of       */
/*      burn values.                                                    */
/* -------------------------------------------------------------------- */
    OGRFeatureH hFeat;
    OGRGeometryH hMultiPolygon = OGR_G_CreateGeometry( wkbMultiPolygon );

    OGR_L_ResetReading( hLayer );
    
    while( (hFeat = OGR_L_GetNextFeature( hLayer )) != NULL )
    {
        OGRGeometryH hGeom = OGR_F_GetGeometryRef(hFeat);

        if( hGeom == NULL )
        {
            CPLError( CE_Failure, CPLE_AppDefined, "ERROR: Cutline feature without a geometry.\n" );
            return 1;
        }
        
        OGRwkbGeometryType eType = wkbFlatten(OGR_G_GetGeometryType( hGeom ));

        if( eType == wkbPolygon ) {
            OGR_G_AddGeometry( hMultiPolygon, hGeom );
		}
        else if( eType == wkbMultiPolygon )
        {
            int iGeom;

            for( iGeom = 0; iGeom < OGR_G_GetGeometryCount( hGeom ); iGeom++ )
            {
                OGR_G_AddGeometry( hMultiPolygon, OGR_G_GetGeometryRef(hGeom,iGeom) );
            }
        }
        else
        {
            CPLError( CE_Failure, CPLE_AppDefined, "ERROR: Cutline not of polygon type.\n" );
            return 1;
        }

        OGR_F_Destroy( hFeat );
    }

    if( OGR_G_GetGeometryCount( hMultiPolygon ) == 0 )
    {
        CPLError( CE_Failure, CPLE_AppDefined, "ERROR: Did not get any cutline features.\n" );
        return 1;
    }

/* -------------------------------------------------------------------- */
/*      Ensure the coordinate system gets set on the geometry.          */
/* -------------------------------------------------------------------- */
    OGR_G_AssignSpatialReference(hMultiPolygon, OGR_L_GetSpatialRef(hLayer) );

    *phCutlineRet = (void *) hMultiPolygon;

/* -------------------------------------------------------------------- */
/*      Cleanup                                                         */
/* -------------------------------------------------------------------- */
    if( pszCSQL != NULL )
        OGR_DS_ReleaseResultSet( hSrcDS, hLayer );

    OGR_DS_Destroy( hSrcDS );
#endif
    return 0;
}

int GDALWarpRaster::TransformCutlineToSource( GDALDatasetH hSrcDS, void *hCutline, char ***ppapszWarpOptions, char **papszTO_In )
{
#ifdef OGR_ENABLED
    OGRGeometryH hMultiPolygon = OGR_G_Clone( (OGRGeometryH) hCutline );
    char **papszTO = CSLDuplicate( papszTO_In );

/* -------------------------------------------------------------------- */
/*      Checkout that SRS are the same.                                 */
/* -------------------------------------------------------------------- */
    OGRSpatialReferenceH  hRasterSRS = NULL;
    const char *pszProjection = NULL;

    if( GDALGetProjectionRef( hSrcDS ) != NULL && strlen(GDALGetProjectionRef( hSrcDS )) > 0 )
        pszProjection = GDALGetProjectionRef( hSrcDS );
    else if( GDALGetGCPProjection( hSrcDS ) != NULL )
        pszProjection = GDALGetGCPProjection( hSrcDS );

    if( pszProjection != NULL )
    {
        hRasterSRS = OSRNewSpatialReference(NULL);
        if( OSRImportFromWkt( hRasterSRS, (char **)&pszProjection ) != CE_None )
        {
            OSRDestroySpatialReference(hRasterSRS);
            hRasterSRS = NULL;
        }
    }

    OGRSpatialReferenceH hCutlineSRS = OGR_G_GetSpatialReference( hMultiPolygon );
    if( hRasterSRS != NULL && hCutlineSRS != NULL )
    {
        /* ok, we will reproject */
    }
    else if( hRasterSRS != NULL && hCutlineSRS == NULL )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                "Warning : the source raster dataset has a SRS, but the cutline features\n"
                "not.  We assume that the cutline coordinates are expressed in the destination SRS.\n"
                "If not, cutline results may be incorrect.\n");
    }
    else if( hRasterSRS == NULL && hCutlineSRS != NULL )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                "Warning : the input vector layer has a SRS, but the source raster dataset does not.\n"
                "Cutline results may be incorrect.\n");
    }

    if( hRasterSRS != NULL )
        OSRDestroySpatialReference(hRasterSRS);

/* -------------------------------------------------------------------- */
/*      Extract the cutline SRS WKT.                                    */
/* -------------------------------------------------------------------- */
    if( hCutlineSRS != NULL )
    {
        char *pszCutlineSRS_WKT = NULL;

        OSRExportToWkt( hCutlineSRS, &pszCutlineSRS_WKT );
        papszTO = CSLSetNameValue( papszTO, "DST_SRS", pszCutlineSRS_WKT );
        CPLFree( pszCutlineSRS_WKT );
    }

/* -------------------------------------------------------------------- */
/*      Transform the geometry to pixel/line coordinates.               */
/* -------------------------------------------------------------------- */
    CutlineTransformer oTransformer;

    /* The cutline transformer will *invert* the hSrcImageTransformer */
    /* so it will convert from the cutline SRS to the source pixel/line */
    /* coordinates */
    oTransformer.hSrcImageTransformer = GDALCreateGenImgProjTransformer2( hSrcDS, NULL, papszTO );

    CSLDestroy( papszTO );

    if( oTransformer.hSrcImageTransformer == NULL )
        return 1;

    OGR_G_Transform( hMultiPolygon, (OGRCoordinateTransformationH) &oTransformer );

    GDALDestroyGenImgProjTransformer( oTransformer.hSrcImageTransformer );

/* -------------------------------------------------------------------- */
/*      Convert aggregate geometry into WKT.                            */
/* -------------------------------------------------------------------- */
    char *pszWKT = NULL;

    OGR_G_ExportToWkt( hMultiPolygon, &pszWKT );
    OGR_G_DestroyGeometry( hMultiPolygon );

    *ppapszWarpOptions = CSLSetNameValue( *ppapszWarpOptions, "CUTLINE", pszWKT );
    CPLFree( pszWKT );
#endif
	return 0;
}

MULTIMAP_API int GDALWarpRaster::processCommand( int argc, char ** argv )
{

/* -------------------------------------------------------------------- */
/*      Register standard GDAL drivers, and process generic GDAL        */
/*      command options.                                                */
/* -------------------------------------------------------------------- */
    //GDALAllRegister();
    argc = GDALGeneralCmdLineProcessor( argc, &argv, 0 );
    if( argc < 1 )
        return ( -argc );

/* -------------------------------------------------------------------- */
/*      Parse arguments.                                                */
/* -------------------------------------------------------------------- */
    for( i = 1; i < argc; i++ )
    {
        if( EQUAL(argv[i], "--utility_version") )
        {
            printf("%s was compiled against GDAL %s and is running against GDAL %s\n",
                   argv[0], GDAL_RELEASE_NAME, GDALVersionInfo("RELEASE_NAME"));
            return 0;
        }
        else if( EQUAL(argv[i],"-co") && i < argc-1 )
        {
            papszCreateOptions = CSLAddString( papszCreateOptions, argv[++i] );
            bCreateOutput = TRUE;
        }   
        else if( EQUAL(argv[i],"-wo") && i < argc-1 )
        {
            papszWarpOptions = CSLAddString( papszWarpOptions, argv[++i] );
        }   
        else if( EQUAL(argv[i],"-multi") )
        {
            bMulti = TRUE;
        }   
        else if( EQUAL(argv[i],"-q") || EQUAL(argv[i],"-quiet"))
        {
            bQuiet = TRUE;
        }   
        else if( EQUAL(argv[i],"-dstalpha") )
        {
            bEnableDstAlpha = TRUE;
        }
        else if( EQUAL(argv[i],"-srcalpha") )
        {
            bEnableSrcAlpha = TRUE;
        }
        else if( EQUAL(argv[i],"-of") && i < argc-1 )
        {
            pszFormat = argv[++i];
            bCreateOutput = TRUE;
            if( EQUAL(pszFormat,"VRT") )
                bVRT = TRUE;
        }
        else if( EQUAL(argv[i],"-t_srs") && i < argc-1 )
        {
            char *pszSRS = SanitizeSRS(argv[++i]);
            papszTO = CSLSetNameValue( papszTO, "DST_SRS", pszSRS );
            CPLFree( pszSRS );
        }
        else if( EQUAL(argv[i],"-s_srs") && i < argc-1 )
        {
            char *pszSRS = SanitizeSRS(argv[++i]);
            papszTO = CSLSetNameValue( papszTO, "SRC_SRS", pszSRS );
            CPLFree( pszSRS );
        }
        else if( EQUAL(argv[i],"-order") && i < argc-1 )
        {
            papszTO = CSLSetNameValue( papszTO, "MAX_GCP_ORDER", argv[++i] );
        }
        else if( EQUAL(argv[i],"-tps") )
        {
            papszTO = CSLSetNameValue( papszTO, "METHOD", "GCP_TPS" );
        }
        else if( EQUAL(argv[i],"-rpc") )
        {
            papszTO = CSLSetNameValue( papszTO, "METHOD", "RPC" );
        }
        else if( EQUAL(argv[i],"-geoloc") )
        {
            papszTO = CSLSetNameValue( papszTO, "METHOD", "GEOLOC_ARRAY" );
        }
        else if( EQUAL(argv[i],"-to") && i < argc-1 )
        {
            papszTO = CSLAddString( papszTO, argv[++i] );
        }
        else if( EQUAL(argv[i],"-et") && i < argc-1 )
        {
            dfErrorThreshold = CPLAtofM(argv[++i]);
        }
        else if( EQUAL(argv[i],"-wm") && i < argc-1 )
        {
            if( CPLAtofM(argv[i+1]) < 10000 )
                dfWarpMemoryLimit = CPLAtofM(argv[i+1]) * 1024 * 1024;
            else
                dfWarpMemoryLimit = CPLAtofM(argv[i+1]);
            i++;
        }
        else if( EQUAL(argv[i],"-srcnodata") && i < argc-1 )
        {
            pszSrcNodata = argv[++i];
        }
        else if( EQUAL(argv[i],"-dstnodata") && i < argc-1 )
        {
            pszDstNodata = argv[++i];
        }
        else if( EQUAL(argv[i],"-tr") && i < argc-2 )
        {
            dfXRes = CPLAtofM(argv[++i]);
            dfYRes = fabs(CPLAtofM(argv[++i]));
            if( dfXRes == 0 || dfYRes == 0 )
            {
                printf( "Wrong value for -tr parameters\n");
                Usage();
            }
            bCreateOutput = TRUE;
        }
        else if( EQUAL(argv[i],"-tap") )
        {
            bTargetAlignedPixels = TRUE;
        }
        else if( EQUAL(argv[i],"-ot") && i < argc-1 )
        {
            int	iType;
            
            for( iType = 1; iType < GDT_TypeCount; iType++ )
            {
                if( GDALGetDataTypeName((GDALDataType)iType) != NULL
                    && EQUAL(GDALGetDataTypeName((GDALDataType)iType),
                             argv[i+1]) )
                {
                    eOutputType = (GDALDataType) iType;
                }
            }

            if( eOutputType == GDT_Unknown )
            {
                printf( "Unknown output pixel type: %s\n", argv[i+1] );
                Usage();
            }
            i++;
            bCreateOutput = TRUE;
        }
        else if( EQUAL(argv[i],"-wt") && i < argc-1 )
        {
            int	iType;
            
            for( iType = 1; iType < GDT_TypeCount; iType++ )
            {
                if( GDALGetDataTypeName((GDALDataType)iType) != NULL
                    && EQUAL(GDALGetDataTypeName((GDALDataType)iType),
                             argv[i+1]) )
                {
                    eWorkingType = (GDALDataType) iType;
                }
            }

            if( eWorkingType == GDT_Unknown )
            {
                printf( "Unknown output pixel type: %s\n", argv[i+1] );
                Usage();
            }
            i++;
        }
        else if( EQUAL(argv[i],"-ts") && i < argc-2 )
        {
            nForcePixels = atoi(argv[++i]);
            nForceLines = atoi(argv[++i]);
            bCreateOutput = TRUE;
        }
        else if( EQUAL(argv[i],"-te") && i < argc-4 )
        {
            dfMinX = CPLAtofM(argv[++i]);
            dfMinY = CPLAtofM(argv[++i]);
            dfMaxX = CPLAtofM(argv[++i]);
            dfMaxY = CPLAtofM(argv[++i]);
            bCreateOutput = TRUE;
        }
        else if( EQUAL(argv[i],"-rn") )
            eResampleAlg = GRA_NearestNeighbour;

        else if( EQUAL(argv[i],"-rb") )
            eResampleAlg = GRA_Bilinear;

        else if( EQUAL(argv[i],"-rc") )
            eResampleAlg = GRA_Cubic;

        else if( EQUAL(argv[i],"-rcs") )
            eResampleAlg = GRA_CubicSpline;

        else if( EQUAL(argv[i],"-r") && i < argc - 1 )
        {
            if ( EQUAL(argv[++i], "near") )
                eResampleAlg = GRA_NearestNeighbour;
            else if ( EQUAL(argv[i], "bilinear") )
                eResampleAlg = GRA_Bilinear;
            else if ( EQUAL(argv[i], "cubic") )
                eResampleAlg = GRA_Cubic;
            else if ( EQUAL(argv[i], "cubicspline") )
                eResampleAlg = GRA_CubicSpline;
            else if ( EQUAL(argv[i], "lanczos") )
                eResampleAlg = GRA_Lanczos;
            else
            {
                printf( "Unknown resampling method: \"%s\".\n", argv[i] );
                Usage();
            }
        }

        else if( EQUAL(argv[i],"-cutline") && i < argc-1 )
        {
            pszCutlineDSName = argv[++i];
        }
        else if( EQUAL(argv[i],"-cwhere") && i < argc-1 )
        {
            pszCWHERE = argv[++i];
        }
        else if( EQUAL(argv[i],"-cl") && i < argc-1 )
        {
            pszCLayer = argv[++i];
        }
        else if( EQUAL(argv[i],"-csql") && i < argc-1 )
        {
            pszCSQL = argv[++i];
        }
        else if( EQUAL(argv[i],"-cblend") && i < argc-1 )
        {
            papszWarpOptions = 
                CSLSetNameValue( papszWarpOptions, 
                                 "CUTLINE_BLEND_DIST", argv[++i] );
        }
        else if( EQUAL(argv[i],"-crop_to_cutline")  )
        {
            bCropToCutline = TRUE;
            bCreateOutput = TRUE;
        }
        else if( EQUAL(argv[i],"-overwrite") )
            bOverwrite = TRUE;

        else if( argv[i][0] == '-' )
            Usage();

        else 
            papszSrcFiles = CSLAddString( papszSrcFiles, argv[i] );
    }
/* -------------------------------------------------------------------- */
/*      Check that incompatible options are not used                    */
/* -------------------------------------------------------------------- */

    if ((nForcePixels != 0 || nForceLines != 0) && 
        (dfXRes != 0 && dfYRes != 0))
    {
        printf( "-tr and -ts options cannot be used at the same time\n");
        Usage();
    }
    
    if (bTargetAlignedPixels && dfXRes == 0 && dfYRes == 0)
    {
        printf( "-tap option cannot be used without using -tr\n");
        Usage();
    }

/* -------------------------------------------------------------------- */
/*      The last filename in the file list is really our destination    */
/*      file.                                                           */
/* -------------------------------------------------------------------- */
    if( CSLCount(papszSrcFiles) > 1 )
    {
        pszDstFilename = papszSrcFiles[CSLCount(papszSrcFiles)-1];
        papszSrcFiles[CSLCount(papszSrcFiles)-1] = NULL;
    }

    if( pszDstFilename == NULL )
        Usage();
        
    if( bVRT && CSLCount(papszSrcFiles) > 1 )
    {
        CPLError( CE_Failure, CPLE_AppDefined, "Warning: gdalwarp -of VRT just takes into account "
                        "the first source dataset.\nIf all source datasets "
                        "are in the same projection, try making a mosaic of\n"
                        "them with gdalbuildvrt, and use the resulting "
                        "VRT file as the input of\ngdalwarp -of VRT.\n");
    }

	return processWarp();

}
MULTIMAP_API void GDALWarpRaster::setSrcDataset(GDALDatasetH srcDatasetH) {
	if ( hSrcDS != 0 ) {
		GDALClose(hSrcDS);
	}
	hSrcDS = srcDatasetH;
}
MULTIMAP_API void GDALWarpRaster::setDstDriver(GDALDriverH dstDriverH) {
	if ( hDstDriver != 0 ) {
		GDALClose(hDstDriver);
	}
	hDstDriver = dstDriverH;
}
MULTIMAP_API void GDALWarpRaster::setCo(char* createOptions) {
	papszCreateOptions = CSLAddString( papszCreateOptions,createOptions);
	bCreateOutput = TRUE;
}
MULTIMAP_API void GDALWarpRaster::setWo(char* warpOptions) {
	papszWarpOptions = CSLAddString( papszWarpOptions,warpOptions);
}
MULTIMAP_API void GDALWarpRaster::setDstAlpha(bool dstAlpha) {
	bEnableDstAlpha = dstAlpha?TRUE:FALSE;
}
MULTIMAP_API void GDALWarpRaster::setSrcAlpha(bool srcAlpha) {
	bEnableSrcAlpha = srcAlpha?TRUE:FALSE;
}
MULTIMAP_API void GDALWarpRaster::setOf(char* outputFormat) {
	pszFormat = new char[strlen(outputFormat)];
	strcpy(pszFormat,outputFormat);
	bCreateOutput = TRUE;
	if( EQUAL(pszFormat,"VRT") )
		bVRT = TRUE;
}
MULTIMAP_API void GDALWarpRaster::setTSrs (char* targetSRS) {
	char *pszSRS = SanitizeSRS(targetSRS);
	papszTO = CSLSetNameValue( papszTO, "DST_SRS", pszSRS );
	CPLFree( pszSRS );
}
MULTIMAP_API void GDALWarpRaster::setTWKT (char* targetWKT) {
	papszTO = CSLSetNameValue( papszTO, "DST_SRS", targetWKT );
}
MULTIMAP_API void GDALWarpRaster::setSSrs (char* sourceSRS) {
	char *pszSRS = SanitizeSRS(sourceSRS);
	papszTO = CSLSetNameValue( papszTO, "SRC_SRS", pszSRS );
	CPLFree( pszSRS );
}
MULTIMAP_API void GDALWarpRaster::setSWKT (char* sourceWKT) {
	papszTO = CSLSetNameValue( papszTO, "SRC_SRS", sourceWKT );
}
MULTIMAP_API void GDALWarpRaster::setOrder(char* order) {
	papszTO = CSLSetNameValue( papszTO, "MAX_GCP_ORDER", order );
}
MULTIMAP_API void GDALWarpRaster::setTps(void) {
	papszTO = CSLSetNameValue( papszTO, "METHOD", "GCP_TPS" );
}
MULTIMAP_API void GDALWarpRaster::setRpc(void) {
	papszTO = CSLSetNameValue( papszTO, "METHOD", "RPC" );
}
MULTIMAP_API void GDALWarpRaster::setGeoloc(void) {
	papszTO = CSLSetNameValue( papszTO, "METHOD", "GEOLOC_ARRAY" );
}
MULTIMAP_API void GDALWarpRaster::setTo(char* to) {
	papszTO = CSLAddString( papszTO, to );
}
MULTIMAP_API void GDALWarpRaster::setEt(double et) {
	dfErrorThreshold = et;
}
MULTIMAP_API void GDALWarpRaster::setWm(double wm) {
	if( wm < 10000 )
		dfWarpMemoryLimit = wm * 1024 * 1024;
	else
		dfWarpMemoryLimit = wm;
}
MULTIMAP_API void GDALWarpRaster::setSrcNodata(char* nodata) {
	pszSrcNodata = nodata;
}
MULTIMAP_API void GDALWarpRaster::setDstNodata(char* nodata) {
	pszDstNodata = nodata;
}
MULTIMAP_API void GDALWarpRaster::setTr(double xRes, double yRes) {
	dfXRes = xRes;
	dfYRes = yRes;
	bCreateOutput = TRUE;
}
MULTIMAP_API void GDALWarpRaster::setTr(GDALDatasetH hDS) {
	double geoTransform[6];
	GDALGetGeoTransform(hDS,geoTransform);
	dfXRes = geoTransform[1];
	dfYRes = ABS(geoTransform[5]);
}
MULTIMAP_API void GDALWarpRaster::setTap(bool tap) {
	bTargetAlignedPixels = tap?TRUE:FALSE;
}
MULTIMAP_API void GDALWarpRaster::setOt(GDALDataType type) {
	eOutputType = type;
	bCreateOutput = TRUE;
}
MULTIMAP_API void GDALWarpRaster::setWt(GDALDataType type) {
	eWorkingType = type;
}
MULTIMAP_API void GDALWarpRaster::setTs(int forcePixels, int forceLines) {
	nForcePixels = forcePixels;
	nForceLines = forceLines;
	bCreateOutput = TRUE;
}
MULTIMAP_API void GDALWarpRaster::setTe(double minX, double minY, double maxX, double maxY) {
	dfMinX = minX;
	dfMinY = minY;
	dfMaxX = maxX;
	dfMaxY = maxY;
	bCreateOutput = TRUE;
}
MULTIMAP_API void GDALWarpRaster::setRn(void) {
	eResampleAlg = GRA_NearestNeighbour;
}
MULTIMAP_API void GDALWarpRaster::setRb(void) {
	eResampleAlg = GRA_Bilinear;
}
MULTIMAP_API void GDALWarpRaster::setRc(void) {
	eResampleAlg = GRA_Cubic;
}
MULTIMAP_API void GDALWarpRaster::setRcs(void) {
	eResampleAlg = GRA_CubicSpline;
}
MULTIMAP_API void GDALWarpRaster::setR(char* method) {
	if ( EQUAL(method, "near") )
		eResampleAlg = GRA_NearestNeighbour;
	else if ( EQUAL(method, "bilinear") )
		eResampleAlg = GRA_Bilinear;
	else if ( EQUAL(method, "cubic") )
		eResampleAlg = GRA_Cubic;
	else if ( EQUAL(method, "cubicspline") )
		eResampleAlg = GRA_CubicSpline;
	else if ( EQUAL(method, "lanczos") )
		eResampleAlg = GRA_Lanczos;
}
MULTIMAP_API void GDALWarpRaster::setR(GDALResampleAlg method) {
	eResampleAlg = method;
}
MULTIMAP_API void GDALWarpRaster::setCutline(char* cutline) {
	pszCutlineDSName = new char[strlen(cutline)];
	strcpy(pszCutlineDSName,cutline);
}
MULTIMAP_API void GDALWarpRaster::setCWhere(char* cwhere) {
	pszCWHERE = new char[strlen(cwhere)];
	strcpy(pszCWHERE,cwhere);
}
MULTIMAP_API void GDALWarpRaster::setCl(char* clayer) {
	pszCLayer = new char[strlen(clayer)];
	strcpy(pszCLayer,clayer);
}
MULTIMAP_API void GDALWarpRaster::setCSql(char* csql) {
	pszCSQL = new char[strlen(csql)];
	strcpy(pszCSQL,csql);
}
MULTIMAP_API void GDALWarpRaster::setCBlend(char* cblend) {
	papszWarpOptions = CSLSetNameValue( papszWarpOptions,"CUTLINE_BLEND_DIST", cblend );
}
MULTIMAP_API void GDALWarpRaster::setCropToLine(void) {
	bCropToCutline = TRUE;
	bCreateOutput = TRUE;
}
MULTIMAP_API void GDALWarpRaster::setFiles(char* files) {
	papszSrcFiles = CSLAddString( papszSrcFiles, files );
	if( CSLCount(papszSrcFiles) > 1 ) {
		pszDstFilename = papszSrcFiles[CSLCount(papszSrcFiles)-1];
		papszSrcFiles[CSLCount(papszSrcFiles)-1] = NULL;
	}
}
MULTIMAP_API void GDALWarpRaster::setDstFile(char* filePath) {
	pszDstFilename = new char[strlen(filePath)];
	strcpy(pszDstFilename,filePath);
}
MULTIMAP_API void GDALWarpRaster::setSrcFile(char* filePath) {
	papszSrcFiles = CSLAddString( papszSrcFiles, filePath );
}