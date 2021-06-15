#include "MultiMap.h"
#include "MFUtils.h"
#include "GDALImageTools.h"

DISABLE_WARNINGS
#include "gdal.h"
#include "gdal_priv.h"
#include "gdal_alg.h"
#include "ogr_api.h"
#include "ogr_core.h"
#include "ogrsf_frmts.h"
#include "cpl_conv.h"
#include "cpl_string.h"
ENABLE_WARNINGS

static unsigned int blendGrey( unsigned char red, unsigned char green, unsigned char blue, unsigned char grey, float greyFactor = 0.5f ) {
	unsigned int colorref = 0;

	float colorFactor = 1.0f - greyFactor;
	float fGrey = static_cast<float>(grey) * greyFactor;
	float fRed = static_cast<float>(red) * colorFactor + fGrey;
	float fGreen = static_cast<float>(green) * colorFactor + fGrey;
	float fBlue = static_cast<float>(blue) * colorFactor + fGrey;
	red = static_cast<unsigned char>(fRed);
	green = static_cast<unsigned char>(fGreen);
	blue = static_cast<unsigned char>(fBlue);
	colorref = RGBA_MAKE(red,green,blue,0);
	return colorref;
}

static void blendGrey( unsigned char* red, unsigned char* green, unsigned char* blue, unsigned char grey, float greyFactor = 0.5f ) {
	float colorFactor = 1.0f - greyFactor;
	float fGrey = static_cast<float>(grey) * greyFactor;
	float fRed = static_cast<float>(*red) * colorFactor + fGrey;
	float fGreen = static_cast<float>(*green) * colorFactor + fGrey;
	float fBlue = static_cast<float>(*blue) * colorFactor + fGrey;
	*red = static_cast<unsigned char>(fRed);
	*green = static_cast<unsigned char>(fGreen);
	*blue = static_cast<unsigned char>(fBlue);
}
static unsigned char blendGrey( unsigned char base, unsigned char grey, float greyFactor = 0.5f ) {
	float colorFactor = 1.0f - greyFactor;
	float fGrey = static_cast<float>(grey) * greyFactor;
	float fBase = static_cast<float>(base) * (colorFactor) + fGrey;
	return static_cast<unsigned char>(fBase);
}

MULTIMAP_API GDALImageTools::~GDALImageTools() {
}
MULTIMAP_API GDALImageTools::GDALImageTools() {
	origin[0] = 0.0;
	origin[1] = 0.0;
	groundWidth = 0.0;
	groundHeight = 0.0;
	paperWidth = 0.0;
	paperHeight = 0.0;
	scale = 0.0;
	dpi = 0;

	transparency = true;
	backgroundColor = 0x00FFFFFF;
	pixelsX = -1;
	pixelsY = -1;

	vectorPointReductionTolerance = -1.0; // force a setting or determination based on groundWidth/pixelsX
}

MULTIMAP_API bool GDALImageTools::MergeHillshade(GDALDataset** ppMergedDS, std::string mergedName, GDALDataset* pBaseDS, GDALDataset* pHillshadeDS, float  greyRatio) {
	bool status = true;

	GDALDriver* pBaseDriver = pBaseDS->GetDriver();

	int maxNX = pBaseDS->GetRasterXSize();
	int maxNY = pBaseDS->GetRasterYSize();
	int baseBandCount = pBaseDS->GetRasterCount();
	int hsBandCount = pHillshadeDS->GetRasterCount();

	GDALRasterBand*	pHsBand = pHillshadeDS->GetRasterBand(1);
	GDALDataType hDataType = pHsBand->GetRasterDataType();

	std::vector<GDALRasterBand*> baseBands;
	for ( int band=0; band<baseBandCount; band++ ) {
		GDALRasterBand*	pBaseBand = pBaseDS->GetRasterBand(band+1);
		if ( hDataType != pBaseBand->GetRasterDataType() ) {
			SetError(1,"CreateHillshade data types mismatched with base datatype");
			status = false;
		}
		baseBands.push_back(pBaseBand);
	}
	if ( status ) {
		*ppMergedDS = pBaseDriver->Create(mergedName.c_str(),maxNX,maxNY,baseBandCount,hDataType,NULL);
		if ( *ppMergedDS ) {
			int mbandcount = (*ppMergedDS)->GetRasterCount();
			unsigned char** baseScanline = new unsigned char*[baseBandCount];
			for ( int band=0; band<baseBandCount; band++ ) {
				baseScanline[band] = new unsigned char[maxNX];
			}
			unsigned char*hsScanline = new unsigned char[maxNX];

			for ( int row=0; row<maxNY; row++ ) {
				int index = row*maxNX;
				for ( int band=0; band<baseBandCount; band++ ) {
					GDALRasterBand* b=baseBands[band];
					unsigned char* scanline = baseScanline[band];
					b->RasterIO(GF_Read,0,maxNY-row-1,maxNX,1,scanline,maxNX,1,hDataType,0,0);
				}
				pHsBand->RasterIO(GF_Read,0,maxNY-row-1,maxNX,1,hsScanline,maxNX,1,hDataType,0,0);
				for ( size_t col=0; col<maxNX; col++ ) {
					for ( int band=0; band<baseBandCount; band++ ) {
						baseScanline[band][col] = blendGrey(baseScanline[band][col],hsScanline[col],greyRatio);
					}
				}
				for ( int band=0; band<baseBandCount; band++ ) {
					GDALRasterBand* b = (*ppMergedDS)->GetRasterBand(band+1);
					b->RasterIO(GF_Write,0,maxNY-row-1,maxNX,1,baseScanline[band],maxNX,1,hDataType,0,0);
				}
			}
			delete[] hsScanline;
			for ( int band=0; band<baseBandCount; band++ ) {
				delete baseScanline[band];
			}
		} else {
			SetError(2,"CreateHillshade could not create merged dataset");
			status = false;
		}
	}
	return status;
}
static int _ImgProjTransform( void *pTransformArg, int bDstToSrc, int nPointCount, double *padfX, double *padfY, double *padfZ, int *panSuccess ) {

	ImageTransformInfo** ppImageTransformInfo = (ImageTransformInfo**) pTransformArg;
	ImageTransformInfo* imageTransformInfo = *ppImageTransformInfo;
	MBRect* pRange = &imageTransformInfo->range;
	int xSize = imageTransformInfo->xSize;
	int ySize = imageTransformInfo->ySize;

	if ( bDstToSrc ) {
		double groundUnitsPerPixel = 1.0/imageTransformInfo->resolution;
		for( int i = 0; i < nPointCount; i++ ) {
			panSuccess[i] = ( padfX[i] != imageTransformInfo->noData && padfY[i] != imageTransformInfo->noData );
			if ( panSuccess[i] ) {
				padfX[i] = (padfX[i]*groundUnitsPerPixel)+pRange->minX;
				padfY[i] = pRange->maxY-(padfY[i]*groundUnitsPerPixel);
				if ( padfX[i] <  pRange->minX || padfX[i] > pRange->maxX || padfY[i] < pRange->minY || padfY[i] >= pRange->maxY ) {
					panSuccess[i] = FALSE;
				}
			}
		}
	} else {
		double pixelsPerGroundUnit = imageTransformInfo->resolution;
		for( int i = 0; i < nPointCount; i++ ) {
			panSuccess[i] = ( padfX[i] != imageTransformInfo->noData && padfY[i] != imageTransformInfo->noData );
			if ( panSuccess[i] ) {
				padfX[i] = (padfX[i]-pRange->minX)*pixelsPerGroundUnit;
				padfY[i] = (pRange->maxY-padfY[i])*pixelsPerGroundUnit;
				if ( padfX[i] < 0 || padfX[i] >= xSize || padfY[i] < 0 || padfY[i] >= ySize ) {
					panSuccess[i] = FALSE;
				}
			}
		}
	}
	return TRUE;
}
MULTIMAP_API int GDALImageTools::Invert(GDALRasterBand* pDstBand, GDALRasterBand* pSrcBand) {
	ErrorReset();
	int errCode = 0;
	GDALDataset* pDataset = pSrcBand->GetDataset();
	int xSize = pDataset->GetRasterXSize();
	int ySize = pDataset->GetRasterYSize();
	unsigned char* scanLine = new unsigned char[xSize];
	for ( int y=0; y<ySize; y++ ) {
		unsigned int offset = y*xSize;
		errCode = pSrcBand->RasterIO(GF_Read,0,y,xSize,1,scanLine,xSize,1,GDT_Byte,0,0);
		if ( errCode != CE_None ) {
			break;
		}
		for ( int x=0; x<xSize; x++ ) {
			scanLine[x] = 0xFF-scanLine[x];
		}
		errCode = pDstBand->RasterIO(GF_Write,0,y,xSize,1,scanLine,xSize,1,GDT_Byte,0,0);
		if ( errCode != CE_None ) {
			break;
		}
	}
	return errCode;
}

MULTIMAP_API int GDALImageTools::Invert(GDALRasterBand* pBand) {
	ErrorReset();
	int errCode = 0;
	GDALDataset* pDataset = pBand->GetDataset();
	int xSize = pDataset->GetRasterXSize();
	int ySize = pDataset->GetRasterYSize();
	unsigned char* scanLine = new unsigned char[xSize];
	for ( int y=0; y<ySize; y++ ) {
		unsigned int offset = y*xSize;
		errCode = pBand->RasterIO(GF_Read,0,y,xSize,1,scanLine,xSize,1,GDT_Byte,0,0);
		if ( errCode != CE_None ) {
			break;
		}
		for ( int x=0; x<xSize; x++ ) {
			scanLine[x] = 0xFF-scanLine[x];
		}
		errCode = pBand->RasterIO(GF_Write,0,y,xSize,1,scanLine,xSize,1,GDT_Byte,0,0);
		if ( errCode != CE_None ) {
			break;
		}
	}
	return errCode;
}

MULTIMAP_API GDALDataset* GDALImageTools::Rasterize(OGRGeometry* pGeometry, double resolution, bool touchAll, GDALDataset* pDataset) {
	ErrorReset ();
	int errCode = 0;
	GDALDataset* pDstDS = pDataset;
	int xSize = 0;
	int ySize = 0;
	OGREnvelope envelope;
	pGeometry->getEnvelope(&envelope);
	if ( !pDstDS ) {
		xSize = static_cast<int>(ceil(envelope.MaxX-envelope.MinX)/resolution);
		ySize = static_cast<int>(ceil(envelope.MaxY-envelope.MinY)/resolution);
		GDALRegister_MEM();
		GDALDriver*	poDriver = GetGDALDriverManager()->GetDriverByName( "MEM");
		char **papszOptions = NULL;
		pDstDS = poDriver->Create( "MEMORY", xSize, ySize, 1, GDT_Byte, papszOptions);
	} else {
		xSize = pDstDS->GetRasterXSize();
		ySize = pDstDS->GetRasterYSize();
	}

	GDALDatasetH hDstDS = (GDALDatasetH) pDstDS;

	int panBandList[1];
	panBandList[0] = 1;
	OGRGeometryH pahGeometries[1];
	pahGeometries[0] = (OGRGeometryH)pGeometry;
	double burnValues[1];
	burnValues[0] = 255.0;

	char **papszOptions = NULL;
	if ( touchAll ) 
		CSLSetNameValue( papszOptions, "ALL_TOUCHED", "TRUE" );
	else
		CSLSetNameValue( papszOptions, "ALL_TOUCHED", "FALSE" );

	ImageTransformInfo imageTransformInfo;
	imageTransformInfo.xSize = xSize;
	imageTransformInfo.ySize = ySize;
	imageTransformInfo.range.minX = envelope.MinX;
	imageTransformInfo.range.minY = envelope.MinY;
	imageTransformInfo.range.maxX = envelope.MaxX;
	imageTransformInfo.range.maxY = envelope.MaxY;
	imageTransformInfo.resolution = resolution;

	void* transformArg = (void*) &imageTransformInfo;

	errCode = GDALRasterizeGeometries(hDstDS,1,panBandList,1,pahGeometries, _ImgProjTransform,&transformArg,burnValues,papszOptions,pfnProgress, pProgressArg);
	CSLDestroy( papszOptions );
	if ( errCode != 0 ) {
		const char* message = NULL;
		int e2 = GetLastError(&message );
		if ( pDstDS != pDataset) {
			GDALClose(pDstDS);
			pDstDS = NULL;
		}
	}
	return pDstDS;
}
MULTIMAP_API GDALDataset* GDALImageTools::Rasterize(OGRLayer* pLayer, double resolution, bool touchAll, GDALDataset* pDataset) {
	ErrorReset ();
	GDALDataset* pDstDS = pDataset;
	int xSize = 0;
	int ySize = 0;
	OGREnvelope envelope;
	pLayer->GetExtent(&envelope);
	if ( !pDstDS ) {
		xSize = static_cast<int>(ceil(envelope.MaxX-envelope.MinX)/resolution);
		ySize = static_cast<int>(ceil(envelope.MaxY-envelope.MinY)/resolution);
		GDALRegister_MEM();
		GDALDriver*	poDriver = GetGDALDriverManager()->GetDriverByName( "MEM");
		char **papszOptions = NULL;
		pDstDS = poDriver->Create( "MEMORY", xSize, ySize, 1, GDT_Byte, papszOptions);
	} else {
		xSize = pDstDS->GetRasterXSize();
		ySize = pDstDS->GetRasterYSize();
	}

	GDALDatasetH hDstDS = (GDALDatasetH) pDstDS;

	int panBandList[1];
	panBandList[0] = 1;
	OGRLayerH pahLayers[1];
	pahLayers[0] = (OGRLayerH) pLayer;
	double burnValues[1];
	burnValues[0] = 0.0;
	GDALRasterBand* pBand = pDstDS->GetRasterBand(1);
	pBand->SetNoDataValue(255.0);
	pBand->Fill(255.0);

	char **papszOptions = NULL;
	if ( touchAll ) 
		CSLSetNameValue( papszOptions, "ALL_TOUCHED", "TRUE" );
	else
		CSLSetNameValue( papszOptions, "ALL_TOUCHED", "FALSE" );

	ImageTransformInfo imageTransformInfo;
	imageTransformInfo.xSize = xSize;
	imageTransformInfo.ySize = ySize;
	imageTransformInfo.range.minX = envelope.MinX;
	imageTransformInfo.range.minY = envelope.MinY;
	imageTransformInfo.range.maxX = envelope.MaxX;
	imageTransformInfo.range.maxY = envelope.MaxY;
	imageTransformInfo.resolution = resolution;

	void* transformArg = (void*) &imageTransformInfo;

	int errCode = GDALRasterizeLayers(hDstDS,1,panBandList,1,pahLayers, _ImgProjTransform,&transformArg,burnValues,papszOptions,pfnProgress, pProgressArg);
	if ( errCode != 0 ) {
		const char* message = NULL;
		int e2 = GetLastError(&message );
		if ( pDstDS != pDataset) {
			GDALClose(pDstDS);
			pDstDS = NULL;
		}
	}
	return pDstDS;
}

/**
* SieveFilter removes raster polygons smaller than a provided threshold size (in pixels) and 
* replaces replaces them with the pixel value of the largest neighbour polygon. 
*
* From the GDAL Documentation: Removes small raster polygons.
*
* The function removes raster polygons smaller than a provided threshold size (in pixels) and replaces replaces them with 
* the pixel value of the largest neighbour polygon. Polygon are determined (per GDALRasterPolygonEnumerator) as regions of 
* the raster where the pixels all have the same value, and that are contiguous (connected). Pixels determined to be "nodata" 
* per hMaskBand will not be treated as part of a polygon regardless of their pixel values. Nodata areas will never be changed 
* nor affect polygon sizes.  Polygons smaller than the threshold with no neighbours that are as large as the threshold will 
* not be altered. Polygons surrounded by nodata areas will therefore not be altered.

The algorithm makes three passes over the input file to enumerate the polygons and collect limited information about them. Memory use is proportional to the number of polygons (roughly 24 bytes per polygon), but is not directly related to the size of the raster. So very large raster files can be processed effectively if there aren't too many polygons. But extremely noisy rasters with many one pixel polygons will end up being expensive (in memory) to process.
* @param pDstBand
* @param pSrcBand 
* @param threshhold the size threshold in pixels. Only raster polygons smaller than this size will be removed.
* @param allNeighbors if [false] use 4 ordinal pixels (diagonals are ignored), if true use all 8 neighbor pixels
* @return the pointer to the input pDstBand if successful, otherwise NULL (caller must still dispose of the destination band.)
**/
MULTIMAP_API GDALRasterBand* GDALImageTools::SieveFilter( GDALRasterBand* pDstBand, GDALRasterBand* pSrcBand, int threshhold, bool allNeighbors) {
	ErrorReset();

	GDALRasterBand* pMaskBand = NULL;
	char **papszOptions = NULL;
	int connectedness = 4;
	if ( allNeighbors ) {
		connectedness = 8;
	}
	int errCode = GDALSieveFilter( (GDALRasterBandH) pSrcBand, (GDALRasterBandH) pMaskBand, (GDALRasterBandH) pDstBand, threshhold, connectedness, papszOptions, pfnProgress, pProgressArg );
	if ( errCode != CE_None ) {
		pDstBand = NULL;
	}
	return pDstBand;
}
MULTIMAP_API int GDALImageTools::SieveFilter(GDALDataset* pDS, int threshold, bool allNeighbors, int bandNum ) {
	ErrorReset();
	char **papszOptions = NULL;
	int connectedness = 4;
	if ( allNeighbors ) {
		connectedness = 8;
	}

	GDALRasterBand* pBand = pDS->GetRasterBand(bandNum);
	GDALRasterBand* pMaskBand = pBand->GetMaskBand();
	GDALImageTools imageTool;
	imageTool.Invert(pMaskBand,pBand);
	GDALRasterBandH hMaskBand = (GDALRasterBandH )pMaskBand;

	int errCode = GDALSieveFilter( (GDALRasterBandH) pBand, (GDALRasterBandH) pMaskBand, (GDALRasterBandH) pBand, threshold, connectedness, papszOptions, pfnProgress, pProgressArg );
#ifdef _DEBUG
	if ( errCode == CE_None ) {
		pDS->FlushCache();
	} else {
		const char* message;
		GetLastError(&message);
		printf("%s",message);
	}
#endif
	return errCode;
}
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////   ACCESSORS  //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
MULTIMAP_API void GDALImageTools::setOrigin(double origin[2]) {
	this->origin[0] = origin[0];
	this->origin[1] = origin[1];
}
MULTIMAP_API void GDALImageTools::setOrigin(double x, double y) {
	origin[0] = x;
	origin[1] = y;
}
MULTIMAP_API double* GDALImageTools::getOrigin(void){ 
	return origin;
}

MULTIMAP_API void GDALImageTools::setGroundSize(double range[2]) { 
	groundWidth = range[0];
	groundHeight = range[1];
}
MULTIMAP_API void GDALImageTools::setGroundSize(double width, double height) {
	groundWidth = width;
	groundHeight = height;
}
MULTIMAP_API double GDALImageTools::getGroundWidth(void) { 
	return groundWidth;
}
MULTIMAP_API double GDALImageTools::getGroundHeight(void){ 
	return groundHeight;
}

MULTIMAP_API void GDALImageTools::setPaperSize( double range[2]) { 
	paperWidth = range[0];
	paperHeight = range[1];
}
MULTIMAP_API void GDALImageTools::setPaperSize(double width, double height){ 
	paperWidth = width;
	paperHeight = height;
}

MULTIMAP_API double GDALImageTools::getPaperWidth(void){ 
	return paperWidth;
}
MULTIMAP_API double GDALImageTools::getPaperHeight(void){ 
	return paperHeight;
}

MULTIMAP_API void GDALImageTools::setScale(double scale){ 
	this->scale = scale;
}
MULTIMAP_API double GDALImageTools::getScale(void){ 
	return scale;
}

MULTIMAP_API void GDALImageTools::setDPI(int dpi){ 
	this->dpi = dpi;
}
MULTIMAP_API int GDALImageTools::getDPI(void){ 
	return dpi;
}
MULTIMAP_API void GDALImageTools::setDPCM(int dpcm){ 
	dpi = static_cast<int>(static_cast<float>(dpcm)/2.54);
}
MULTIMAP_API int GDALImageTools::getDPCM(void){ 
	return static_cast<int>(static_cast<float>(dpi)*2.54);
}

MULTIMAP_API void GDALImageTools::setTransparency(bool transparency){
	this->transparency = transparency;
	if ( transparency ) {
		setBackgroundColor(0x00FFFFFF);
	}
}
MULTIMAP_API bool GDALImageTools::getTransparency(void){ 
	return transparency;
}
MULTIMAP_API void GDALImageTools::setBackgroundColor(unsigned char red, unsigned char green, unsigned char blue) {
	backgroundColor = RGBA_MAKE(red,green,blue,0);
}
MULTIMAP_API void GDALImageTools::setBackgroundColor(unsigned int color) {
	backgroundColor = color;
}
MULTIMAP_API void GDALImageTools::setBackgroundColor(const char* namedColor, unsigned int onFailedLookup) {
	backgroundColor = onFailedLookup;
	MFUtils mfUtils;
	int nColors = sizeof(ucNamedColors) / sizeof(ucNamedColors[0]);
	const char* namedColorLC = mfUtils.ToLower(namedColor).c_str();
	for ( int i=0; i<nColors; i++ )  {
		if ( strcmp(ucNamedColors[i].name,namedColor) == 0 ) {
			backgroundColor = RGBA_MAKE(ucNamedColors[i].r,ucNamedColors[i].g,ucNamedColors[i].b,0);
		}
	}
}
MULTIMAP_API unsigned int GDALImageTools::getBackgroundColor(void) {
	return backgroundColor;
}

MULTIMAP_API void GDALImageTools::setSourceFolder(std::string sourceFolder){ 
	this->sourceFolder = sourceFolder;
}
MULTIMAP_API std::string GDALImageTools::getSourceFolder(void){ 
	return sourceFolder;
}

MULTIMAP_API void GDALImageTools::setTargetFolder(std::string targetFolder){ 
	this->targetFolder = targetFolder;
}
MULTIMAP_API std::string GDALImageTools::getTargetFolder(void){ 
	return targetFolder;
}
MULTIMAP_API void GDALImageTools::setPixelsX(int pixels) {
	pixelsX = pixels;
}
MULTIMAP_API void GDALImageTools::setPixelsY(int pixels) {
	pixelsY = pixels;
}
MULTIMAP_API int GDALImageTools::getPixelsX(void) {
	if ( pixelsX <= 0 ) {
		double dPixelsX = paperWidth * getDPCM() + 0.5;
		pixelsX = static_cast<int>(dPixelsX);
	}
	return pixelsX;
}
MULTIMAP_API int GDALImageTools::getPixelsY(void) {
	if ( pixelsY <= 0 ) {
		double dPixelsY = paperHeight * getDPCM() + 0.5;
		pixelsY = static_cast<int>(dPixelsY);
	}
	return pixelsY;
}

MULTIMAP_API void GDALImageTools::setVectorPointReductionTolerance(double tolerance) {
	vectorPointReductionTolerance = tolerance;
}
MULTIMAP_API double GDALImageTools::getVectorPointReductionTolerance(void) {
	if ( vectorPointReductionTolerance < 0.0 ) {
		vectorPointReductionTolerance = getGroundWidth() / static_cast<double>(getPixelsX());
	}
	return vectorPointReductionTolerance;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/************************************************************************/
/*                          InvertGeometries()                          */
/************************************************************************/

static void InvertGeometries( GDALDatasetH hDstDS, 
							 std::vector<OGRGeometryH> &ahGeometries )

{
	OGRGeometryH hCollection = 
		OGR_G_CreateGeometry( wkbGeometryCollection );

	/* -------------------------------------------------------------------- */
	/*      Create a ring that is a bit outside the raster dataset.         */
	/* -------------------------------------------------------------------- */
	OGRGeometryH hUniversePoly, hUniverseRing;
	double adfGeoTransform[6];
	int brx = GDALGetRasterXSize( hDstDS ) + 2;
	int bry = GDALGetRasterYSize( hDstDS ) + 2;

	GDALGetGeoTransform( hDstDS, adfGeoTransform );

	hUniverseRing = OGR_G_CreateGeometry( wkbLinearRing );

	OGR_G_AddPoint_2D( 
		hUniverseRing, 
		adfGeoTransform[0] + -2*adfGeoTransform[1] + -2*adfGeoTransform[2],
		adfGeoTransform[3] + -2*adfGeoTransform[4] + -2*adfGeoTransform[5] );

	OGR_G_AddPoint_2D( 
		hUniverseRing, 
		adfGeoTransform[0] + brx*adfGeoTransform[1] + -2*adfGeoTransform[2],
		adfGeoTransform[3] + brx*adfGeoTransform[4] + -2*adfGeoTransform[5] );

	OGR_G_AddPoint_2D( 
		hUniverseRing, 
		adfGeoTransform[0] + brx*adfGeoTransform[1] + bry*adfGeoTransform[2],
		adfGeoTransform[3] + brx*adfGeoTransform[4] + bry*adfGeoTransform[5] );

	OGR_G_AddPoint_2D( 
		hUniverseRing, 
		adfGeoTransform[0] + -2*adfGeoTransform[1] + bry*adfGeoTransform[2],
		adfGeoTransform[3] + -2*adfGeoTransform[4] + bry*adfGeoTransform[5] );

	OGR_G_AddPoint_2D( 
		hUniverseRing, 
		adfGeoTransform[0] + -2*adfGeoTransform[1] + -2*adfGeoTransform[2],
		adfGeoTransform[3] + -2*adfGeoTransform[4] + -2*adfGeoTransform[5] );

	hUniversePoly = OGR_G_CreateGeometry( wkbPolygon );
	OGR_G_AddGeometryDirectly( hUniversePoly, hUniverseRing );

	OGR_G_AddGeometryDirectly( hCollection, hUniversePoly );

	/* -------------------------------------------------------------------- */
	/*      Add the rest of the geometries into our collection.             */
	/* -------------------------------------------------------------------- */
	unsigned int iGeom;

	for( iGeom = 0; iGeom < ahGeometries.size(); iGeom++ )
		OGR_G_AddGeometryDirectly( hCollection, ahGeometries[iGeom] );

	ahGeometries.resize(1);
	ahGeometries[0] = hCollection;
}

/************************************************************************/
/*                            ProcessLayer()                            */
/*                                                                      */
/*      Process all the features in a layer selection, collecting       */
/*      geometries and burn values.                                     */
/************************************************************************/

static void ProcessLayer( 
	OGRLayerH hSrcLayer, int bSRSIsSet, 
	GDALDatasetH hDstDS, std::vector<int> anBandList,
	std::vector<double> &adfBurnValues, int b3D, int bInverse,
	const char *pszBurnAttribute, char **papszRasterizeOptions,
	GDALProgressFunc pfnProgress, void* pProgressData )

{
	/* -------------------------------------------------------------------- */
	/*      Checkout that SRS are the same.                                 */
	/*      If -a_srs is specified, skip the test                           */
	/* -------------------------------------------------------------------- */
	if (false && !bSRSIsSet)
	{
		OGRSpatialReferenceH  hDstSRS = NULL;
		if( GDALGetProjectionRef( hDstDS ) != NULL )
		{
			char *pszProjection;

			pszProjection = (char *) GDALGetProjectionRef( hDstDS );

			hDstSRS = OSRNewSpatialReference(NULL);
			if( OSRImportFromWkt( hDstSRS, &pszProjection ) != CE_None )
			{
				OSRDestroySpatialReference(hDstSRS);
				hDstSRS = NULL;
			}
		}

		OGRSpatialReferenceH hSrcSRS = OGR_L_GetSpatialRef(hSrcLayer);
		if( hDstSRS != NULL && hSrcSRS != NULL )
		{
			if( OSRIsSame(hSrcSRS, hDstSRS) == FALSE )
			{
				fprintf(stderr,
					"Warning : the output raster dataset and the input vector layer do not have the same SRS.\n"
					"Results might be incorrect (no on-the-fly reprojection of input data).\n");
			}
		}
		else if( hDstSRS != NULL && hSrcSRS == NULL )
		{
			fprintf(stderr,
				"Warning : the output raster dataset has a SRS, but the input vector layer SRS is unknown.\n"
				"Ensure input vector has the same SRS, otherwise results might be incorrect.\n");
		}
		else if( hDstSRS == NULL && hSrcSRS != NULL )
		{
			fprintf(stderr,
				"Warning : the input vector layer has a SRS, but the output raster dataset SRS is unknown.\n"
				"Ensure output raster dataset has the same SRS, otherwise results might be incorrect.\n");
		}

		if( hDstSRS != NULL )
		{
			OSRDestroySpatialReference(hDstSRS);
		}
	}

	/* -------------------------------------------------------------------- */
	/*      Get field index, and check.                                     */
	/* -------------------------------------------------------------------- */
	int iBurnField = -1;

	if( pszBurnAttribute )
	{
		iBurnField = OGR_FD_GetFieldIndex( OGR_L_GetLayerDefn( hSrcLayer ),
			pszBurnAttribute );
		if( iBurnField == -1 )
		{
			printf( "Failed to find field %s on layer %s, skipping.\n",
				pszBurnAttribute, 
				OGR_FD_GetName( OGR_L_GetLayerDefn( hSrcLayer ) ) );
			return;
		}
	}

	/* -------------------------------------------------------------------- */
	/*      Collect the geometries from this layer, and build list of       */
	/*      burn values.                                                    */
	/* -------------------------------------------------------------------- */
	OGRFeatureH hFeat;
	std::vector<OGRGeometryH> ahGeometries;
	std::vector<double> adfFullBurnValues;

	OGR_L_ResetReading( hSrcLayer );

	while( (hFeat = OGR_L_GetNextFeature( hSrcLayer )) != NULL )
	{
		OGRGeometryH hGeom;

		if( OGR_F_GetGeometryRef( hFeat ) == NULL )
		{
			OGR_F_Destroy( hFeat );
			continue;
		}

		hGeom = OGR_G_Clone( OGR_F_GetGeometryRef( hFeat ) );
		ahGeometries.push_back( hGeom );

		for( unsigned int iBand = 0; iBand < anBandList.size(); iBand++ )
		{
			if( adfBurnValues.size() > 0 )
				adfFullBurnValues.push_back( 
				adfBurnValues[MIN(iBand,adfBurnValues.size()-1)] );
			else if( pszBurnAttribute )
			{
				adfFullBurnValues.push_back( OGR_F_GetFieldAsDouble( hFeat, iBurnField ) );
			}
			/* I have made the 3D option exclusive to other options since it
			can be used to modify the value from "-burn value" or
			"-a attribute_name" */
			if( b3D )
			{
				// TODO: get geometry "z" value
				/* Points and Lines will have their "z" values collected at the
				point and line levels respectively. However filled polygons
				(GDALdllImageFilledPolygon) can use some help by getting
				their "z" values here. */
				adfFullBurnValues.push_back( 0.0 );
			}
		}

		OGR_F_Destroy( hFeat );
	}

	/* -------------------------------------------------------------------- */
	/*      If we are in inverse mode, we add one extra ring around the     */
	/*      whole dataset to invert the concept of insideness and then      */
	/*      merge everything into one geometry collection.                  */
	/* -------------------------------------------------------------------- */
	if( bInverse )
	{
		if( ahGeometries.size() == 0 )
		{
			for( unsigned int iBand = 0; iBand < anBandList.size(); iBand++ )
			{
				if( adfBurnValues.size() > 0 )
					adfFullBurnValues.push_back(
					adfBurnValues[MIN(iBand,adfBurnValues.size()-1)] );
				else /* FIXME? Not sure what to do exactly in the else case, but we must insert a value */
					adfFullBurnValues.push_back( 0.0 );
			}
		}

		InvertGeometries( hDstDS, ahGeometries );
	}

	/* -------------------------------------------------------------------- */
	/*      Perform the burn.                                               */
	/* -------------------------------------------------------------------- */
	GDALRasterizeGeometries( hDstDS, (int)anBandList.size(), &(anBandList[0]), 
		(int)ahGeometries.size(), &(ahGeometries[0]), 
		NULL, NULL, &(adfFullBurnValues[0]), 
		papszRasterizeOptions,
		pfnProgress, pProgressData );

	/* -------------------------------------------------------------------- */
	/*      Cleanup geometries.                                             */
	/* -------------------------------------------------------------------- */
	while ( ahGeometries.size() > 0 ) {
		OGRGeometryH hGeom = ahGeometries.back();
		if ( hGeom ) {
			OGR_G_DestroyGeometry(hGeom);
		}
		ahGeometries.pop_back();
	}

	//for( size_t iGeom = ahGeometries.size()-1; iGeom >= 0; iGeom-- ) {
	//    OGR_G_DestroyGeometry( ahGeometries[iGeom] );
	// }
}

/************************************************************************/
/*                  CreateOutputDataset()                               */
/************************************************************************/

static
	GDALDatasetH CreateOutputDataset(std::vector<OGRLayerH> ahLayers,
	OGRSpatialReferenceH hSRS,
	int bGotBounds, OGREnvelope sEnvelop,
	GDALDriverH hDriver, const char* pszDstFilename,
	int nXSize, int nYSize, double dfXRes, double dfYRes,
	int bTargetAlignedPixels,
	int nBandCount, GDALDataType eOutputType,
	char** papszCreateOptions, std::vector<double> adfInitVals,
	int bNoDataSet, double dfNoData)
{
	int bFirstLayer = TRUE;
	char* pszWKT = NULL;
	GDALDatasetH hDstDS = NULL;
	unsigned int i;

	for( i = 0; i < ahLayers.size(); i++ )
	{
		OGRLayerH hLayer = ahLayers[i];

		if (!bGotBounds)
		{
			OGREnvelope sLayerEnvelop;

			if (OGR_L_GetExtent(hLayer, &sLayerEnvelop, TRUE) != OGRERR_NONE)
			{
				fprintf(stderr, "Cannot get layer extent\n");
				exit(2);
			}

			/* When rasterizing point layers and that the bounds have */
			/* not been explicitely set, voluntary increase the extent by */
			/* a half-pixel size to avoid missing points on the border */
			if (wkbFlatten(OGR_L_GetGeomType(hLayer)) == wkbPoint &&
				!bTargetAlignedPixels && dfXRes != 0 && dfYRes != 0)
			{
				sLayerEnvelop.MinX -= dfXRes / 2;
				sLayerEnvelop.MaxX += dfXRes / 2;
				sLayerEnvelop.MinY -= dfYRes / 2;
				sLayerEnvelop.MaxY += dfYRes / 2;
			}

			if (bFirstLayer)
			{
				sEnvelop.MinX = sLayerEnvelop.MinX;
				sEnvelop.MinY = sLayerEnvelop.MinY;
				sEnvelop.MaxX = sLayerEnvelop.MaxX;
				sEnvelop.MaxY = sLayerEnvelop.MaxY;

				if (hSRS == NULL)
					hSRS = OGR_L_GetSpatialRef(hLayer);

				bFirstLayer = FALSE;
			}
			else
			{
				sEnvelop.MinX = MIN(sEnvelop.MinX, sLayerEnvelop.MinX);
				sEnvelop.MinY = MIN(sEnvelop.MinY, sLayerEnvelop.MinY);
				sEnvelop.MaxX = MAX(sEnvelop.MaxX, sLayerEnvelop.MaxX);
				sEnvelop.MaxY = MAX(sEnvelop.MaxY, sLayerEnvelop.MaxY);
			}
		}
		else
		{
			if (bFirstLayer)
			{
				if (hSRS == NULL)
					hSRS = OGR_L_GetSpatialRef(hLayer);

				bFirstLayer = FALSE;
			}
		}
	}

	if (dfXRes == 0 && dfYRes == 0)
	{
		dfXRes = (sEnvelop.MaxX - sEnvelop.MinX) / nXSize;
		dfYRes = (sEnvelop.MaxY - sEnvelop.MinY) / nYSize;
	}
	else if (bTargetAlignedPixels && dfXRes != 0 && dfYRes != 0)
	{
		sEnvelop.MinX = floor(sEnvelop.MinX / dfXRes) * dfXRes;
		sEnvelop.MaxX = ceil(sEnvelop.MaxX / dfXRes) * dfXRes;
		sEnvelop.MinY = floor(sEnvelop.MinY / dfYRes) * dfYRes;
		sEnvelop.MaxY = ceil(sEnvelop.MaxY / dfYRes) * dfYRes;
	}

	double adfProjection[6];
	adfProjection[0] = sEnvelop.MinX;
	adfProjection[1] = dfXRes;
	adfProjection[2] = 0;
	adfProjection[3] = sEnvelop.MaxY;
	adfProjection[4] = 0;
	adfProjection[5] = -dfYRes;

	if (nXSize == 0 && nYSize == 0)
	{
		nXSize = (int)(0.5 + (sEnvelop.MaxX - sEnvelop.MinX) / dfXRes);
		nYSize = (int)(0.5 + (sEnvelop.MaxY - sEnvelop.MinY) / dfYRes);
	}

	hDstDS = GDALCreate(hDriver, pszDstFilename, nXSize, nYSize,
		nBandCount, eOutputType, papszCreateOptions);
	if (hDstDS == NULL)
	{
		fprintf(stderr, "Cannot create %s\n", pszDstFilename);
		exit(2);
	}

	GDALSetGeoTransform(hDstDS, adfProjection);

	if (hSRS)
		OSRExportToWkt(hSRS, &pszWKT);
	if (pszWKT)
		GDALSetProjection(hDstDS, pszWKT);
	CPLFree(pszWKT);

	int iBand;
	/*if( nBandCount == 3 || nBandCount == 4 )
	{
	for(iBand = 0; iBand < nBandCount; iBand++)
	{
	GDALRasterBandH hBand = GDALGetRasterBand(hDstDS, iBand + 1);
	GDALSetRasterColorInterpretation(hBand, (GDALColorInterp)(GCI_RedBand + iBand));
	}
	}*/

	if (bNoDataSet)
	{
		for(iBand = 0; iBand < nBandCount; iBand++)
		{
			GDALRasterBandH hBand = GDALGetRasterBand(hDstDS, iBand + 1);
			GDALSetRasterNoDataValue(hBand, dfNoData);
		}
	}

	if (adfInitVals.size() != 0)
	{
		for(iBand = 0; iBand < MIN(nBandCount,(int)adfInitVals.size()); iBand++)
		{
			GDALRasterBandH hBand = GDALGetRasterBand(hDstDS, iBand + 1);
			GDALFillRaster(hBand, adfInitVals[iBand], 0);
		}
	}

	return hDstDS;
}


MULTIMAP_API GDALDataset* GDALImageTools::Rasterize2(OGRLayer* pLayer, double resolution, MBRect* range, bool touchAll, GDALDataset* pDataset) {
	ErrorReset ();
	GDALDataset* pDstDS = pDataset;
	GDALDatasetH hDstDS = (GDALDatasetH) pDstDS;
	OGRLayerH hLayer = (OGRLayerH) pLayer;


	int xSize = 0;
	int ySize = 0;
	OGREnvelope envelope;
	pLayer->GetExtent(&envelope);
	if ( !pDstDS ) {

		/*
		hDstDS = CreateOutputDataset(ahLayers, hSRS,
		bGotBounds, sEnvelop,
		hDriver, pszDstFilename,
		nXSize, nYSize, dfXRes, dfYRes,
		bTargetAlignedPixels,
		anBandList.size(), eOutputType,
		papszCreateOptions, adfInitVals,
		bNoDataSet, dfNoData);
		*/

		xSize = static_cast<int>(ceil(envelope.MaxX-envelope.MinX)/resolution);
		ySize = static_cast<int>(ceil(envelope.MaxY-envelope.MinY)/resolution);
		GDALRegister_MEM();
		GDALDriver*	poDriver = GetGDALDriverManager()->GetDriverByName( "MEM");
		char **papszOptions = NULL;
		pDstDS = poDriver->Create( "MEMORY", xSize, ySize, 1, GDT_Byte, papszOptions);
	} else {
		xSize = pDstDS->GetRasterXSize();
		ySize = pDstDS->GetRasterYSize();
	}

	std::vector<int> anBandList;
	anBandList.push_back(1);
	OGRLayerH pahLayers[1];
	pahLayers[0] = (OGRLayerH) pLayer;
	std::vector<double> adfBurnValues;
	adfBurnValues.push_back(0.0);
	int b3D = FALSE;
	int bInverse = FALSE;
	const char* pszBurnAttribute = NULL;
	GDALRasterBand* pBand = pDstDS->GetRasterBand(1);
	pBand->SetNoDataValue(255.0);
	pBand->Fill(255.0);

	char **papszOptions = NULL;
	if ( touchAll ) 
		CSLSetNameValue( papszOptions, "ALL_TOUCHED", "TRUE" );
	else
		CSLSetNameValue( papszOptions, "ALL_TOUCHED", "FALSE" );

	ImageTransformInfo imageTransformInfo;
	imageTransformInfo.xSize = xSize;
	imageTransformInfo.ySize = ySize;
	imageTransformInfo.range.minX = envelope.MinX;
	imageTransformInfo.range.minY = envelope.MinY;
	imageTransformInfo.range.maxX = envelope.MaxX;
	imageTransformInfo.range.maxY = envelope.MaxY;
	imageTransformInfo.resolution = resolution;

	void* transformArg = (void*) &imageTransformInfo;


	ProcessLayer( hLayer, FALSE, hDstDS, anBandList, 
		adfBurnValues, b3D, bInverse, pszBurnAttribute,
		NULL,NULL,NULL);
	//papszRasterizeOptions, GDALScaledProgress, pScaledProgress );

	return pDstDS;
}