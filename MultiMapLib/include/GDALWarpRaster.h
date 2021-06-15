#pragma once
#include "GDALCommon.h"

DISABLE_WARNINGS
#include "gdalwarper.h"
#include "cpl_string.h"
#include "ogr_spatialref.h"
#include "ogr_api.h"
ENABLE_WARNINGS

#define OGR_ENABLED

class GDALDriver;
class GDALDataset;
class GDALWarpRaster : public GDALCommon {
public:
	MULTIMAP_API GDALWarpRaster::~GDALWarpRaster(void);
	MULTIMAP_API GDALWarpRaster::GDALWarpRaster(void);
    MULTIMAP_API int WarpRaster(GDALDriver* pDstDriver, std::string dstFile, std::string dstWKT, GDALDataset* pSrcDS, GDALResampleAlg eResampleAlg);

    MULTIMAP_API int WarpRaster(void);
	MULTIMAP_API int processCommand( int argc, char ** argv );
	MULTIMAP_API int processWarp(void);
	MULTIMAP_API int processCutline(void);
	MULTIMAP_API void CloseDstDS(void);

	GDALDatasetH        hSrcDS;
	GDALDatasetH	    hDstDS;
	GDALDriverH         hDstDriver;
	char               *pszFormat;
	char              **papszSrcFiles;
	char               *pszDstFilename;
	int                 bCreateOutput;
	void               *hTransformArg;
	void               *hGenImgProjArg;
	void               *hApproxArg;
	char               **papszWarpOptions;
	double             dfErrorThreshold;
	double             dfWarpMemoryLimit;
	GDALTransformerFunc pfnTransformer;
	char                **papszCreateOptions;
	GDALDataType        eOutputType;
	GDALDataType        eWorkingType; 
	GDALResampleAlg     eResampleAlg;
	const char          *pszSrcNodata;
	const char          *pszDstNodata;
	int                 bMulti;
	char                **papszTO;
	char                *pszCutlineDSName;
	char                *pszCLayer;
	char                *pszCWHERE;
	char                *pszCSQL;
	void                *hCutline;
	int                  bHasGotErr;
	int                  bCropToCutline;
	int                  bOverwrite;

	MULTIMAP_API void setDstDriver(GDALDriverH dstDriverH);
	MULTIMAP_API void setSrcDataset(GDALDatasetH srcDatasetH);
	MULTIMAP_API void setCo(char* createOptions);
	MULTIMAP_API void setWo(char* warpOptions);
	MULTIMAP_API void setDstAlpha(bool dstAlpha=true);
	MULTIMAP_API void setSrcAlpha(bool srcAlpha=true);
	MULTIMAP_API void setOf(char* outputFormat);
	MULTIMAP_API void setTSrs (char* targetSRS);
	MULTIMAP_API void setTWKT (char* targetWKT);
	MULTIMAP_API void setSSrs (char* sourceSRS);
	MULTIMAP_API void setSWKT (char* sourceWKT);
	MULTIMAP_API void setOrder(char* order);
	MULTIMAP_API void setTps(void);
	MULTIMAP_API void setRpc(void);
	MULTIMAP_API void setGeoloc(void);
	MULTIMAP_API void setTo(char* to);
	MULTIMAP_API void setEt(double et);
	MULTIMAP_API void setWm(double wm);
	MULTIMAP_API void setSrcNodata(char* nodata);
	MULTIMAP_API void setDstNodata(char* nodata);
	MULTIMAP_API void setTr(double xRes, double yRes);
	MULTIMAP_API void setTr(GDALDatasetH hDS);
	MULTIMAP_API void setTap(bool tap=true);
	MULTIMAP_API void setOt(GDALDataType type);
	MULTIMAP_API void setWt(GDALDataType wt);
	MULTIMAP_API void setTs(int forcePixels, int forceLines);
	MULTIMAP_API void setTe(double minX, double minY, double maxX, double maxY);
	MULTIMAP_API void setRn(void);
	MULTIMAP_API void setRb(void);
	MULTIMAP_API void setRc(void);
	MULTIMAP_API void setRcs(void);
	MULTIMAP_API void setR(char* method);
	MULTIMAP_API void setR(GDALResampleAlg method);
	MULTIMAP_API void setCutline(char* cutline);
	MULTIMAP_API void setCWhere(char* cwhere);
	MULTIMAP_API void setCl(char* clayer);
	MULTIMAP_API void setCSql(char* csql);
	MULTIMAP_API void setCBlend(char* cblend);
	MULTIMAP_API void setCropToLine(void);
	MULTIMAP_API void setFiles(char* files);
	MULTIMAP_API void setDstFile(char* filePath);
	MULTIMAP_API void setSrcFile(char* filePath);

private:
	GDALDatasetH GDALWarpCreateOutput( char **papszSrcFiles, const char *pszFilename, const char *pszFormat, char **papszTO, char ***ppapszCreateOptions, GDALDataType eDT);
    int PreprocessSourceDS(GDALDatasetH & hSourceDS, GDALDataType & eDT, int & nDstBandCount, GDALColorTableH & hCT, char *pszThisTargetSRS, char* srcFile, int iSrc);
	int ProcessSourceDS(GDALDatasetH hSourceDS, int iSrc);
	int LoadCutline( const char *pszCutlineDSName, const char *pszCLayer, const char *pszCWHERE, const char *pszCSQL, void **phCutlineRet );
	int TransformCutlineToSource( GDALDatasetH hSrcDS, void *hCutline, char ***ppapszWarpOptions, char **papszTO );
	void Usage();
	char *SanitizeSRS( const char *pszUserInput );


	double	       dfMinX;
	double         dfMinY;
	double         dfMaxX;
	double         dfMaxY;
	double	       dfXRes;
	double         dfYRes;
	int             bTargetAlignedPixels;
	int             nForcePixels;
	int             nForceLines;
	int             bQuiet;
	int             bEnableDstAlpha;
	int             bEnableSrcAlpha;
	int             bVRT;
    int             bInitDestSetForFirst;
    int i;

    double dfWrkMinX, dfWrkMaxX, dfWrkMinY, dfWrkMaxY;
    double dfWrkResX, dfWrkResY;
};

class CutlineTransformer : public OGRCoordinateTransformation
{
public:

    void         *hSrcImageTransformer;

    virtual OGRSpatialReference *GetSourceCS() { return NULL; }
    virtual OGRSpatialReference *GetTargetCS() { return NULL; }

    virtual int Transform( int nCount, 
                           double *x, double *y, double *z = NULL ) {
        int nResult;

        int *pabSuccess = (int *) CPLCalloc(sizeof(int),nCount);
        nResult = TransformEx( nCount, x, y, z, pabSuccess );
        CPLFree( pabSuccess );

        return nResult;
    }

    virtual int TransformEx( int nCount, 
                             double *x, double *y, double *z = NULL,
                             int *pabSuccess = NULL ) {
        return GDALGenImgProjTransform( hSrcImageTransformer, TRUE, 
                                        nCount, x, y, z, pabSuccess );
    }
};