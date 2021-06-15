#pragma once
#include "GDALCommon.h"
#include "gdal.h"
#include "gdal_priv.h"

#undef EQUAL
#ifndef EQUAL
#undef STRCASECMP
#undef STRNCASECMP
#  if defined(WIN32) || defined(WIN32CE)
#    define STRCASECMP(a,b)         (_stricmp(a,b))
#    define STRNCASECMP(a,b,n)      (_strnicmp(a,b,n))
#  else
#    define STRCASECMP(a,b)         (_strcasecmp(a,b))
#    define STRNCASECMP(a,b,n)      (_strncasecmp(a,b,n))
#  endif
//#  define EQUALN(a,b,n)           (STRNCASECMP(a,b,n)==0)
#  define EQUAL(a,b)              (STRCASECMP(a,b)==0)
#endif


typedef enum
{
	HILL_SHADE,
	SLOPE,
	ASPECT,
	COLOR_RELIEF,
	TRI,
	TPI,
	ROUGHNESS
} Algorithm;

typedef struct
{
    double dfVal;
    int nR;
    int nG;
    int nB;
    int nA;
} ColorAssociation;

typedef enum
{
    COLOR_SELECTION_INTERPOLATE,
    COLOR_SELECTION_NEAREST_ENTRY,
    COLOR_SELECTION_EXACT_ENTRY
} ColorSelectionMode;

#define GDAL_PERCENT 0
#define GDAL_DEGREES 1

typedef float (*GDALGeneric3x3ProcessingAlg) (float* pafWindow, float fDstNoDataValue, void* pData);

/************************************************************************/
/* ==================================================================== */
/*                       GDALGeneric3x3Dataset                        */
/* ==================================================================== */
/************************************************************************/

class GDALGeneric3x3RasterBand;

class GDALGeneric3x3Dataset : public GDALDataset
{
    friend class GDALGeneric3x3RasterBand;

    GDALGeneric3x3ProcessingAlg pfnAlg;
    void*              pAlgData;
    GDALDatasetH       hSrcDS;
    GDALRasterBandH    hSrcBand;
    float*             apafSourceBuf[3];
    int                bDstHasNoData;
    double             dfDstNoDataValue;
    int                nCurLine;
    int                bComputeAtEdges;

  public:
                        GDALGeneric3x3Dataset(GDALDatasetH hSrcDS,
                                              GDALRasterBandH hSrcBand,
                                              GDALDataType eDstDataType,
                                              int bDstHasNoData,
                                              double dfDstNoDataValue,
                                              GDALGeneric3x3ProcessingAlg pfnAlg,
                                              void* pAlgData,
                                              int bComputeAtEdges);
                       ~GDALGeneric3x3Dataset();

    CPLErr      GetGeoTransform( double * padfGeoTransform );
    const char *GetProjectionRef();
};

/************************************************************************/
/* ==================================================================== */
/*                    GDALGeneric3x3RasterBand                       */
/* ==================================================================== */
/************************************************************************/

class GDALGeneric3x3RasterBand : public GDALRasterBand
{
    friend class GDALGeneric3x3Dataset;
    int bSrcHasNoData;
    float fSrcNoDataValue;
    
    void                    InitWidthNoData(void* pImage);
    
  public:
                 GDALGeneric3x3RasterBand( GDALGeneric3x3Dataset *poDS,
                                           GDALDataType eDstDataType );
    
    virtual CPLErr          IReadBlock( int, int, void * );
    virtual double          GetNoDataValue( int* pbHasNoData );
};


/************************************************************************/
/* ==================================================================== */
/*                       GDALColorReliefDataset                        */
/* ==================================================================== */
/************************************************************************/

class GDALColorReliefRasterBand;

class GDALColorReliefDataset : public GDALDataset
{
    friend class GDALColorReliefRasterBand;

    GDALDatasetH       hSrcDS;
    GDALRasterBandH    hSrcBand;
    int                nColorAssociation;
    ColorAssociation*  pasColorAssociation;
    ColorSelectionMode eColorSelectionMode;
    GByte*             pabyPrecomputed;
    int                nIndexOffset;
    float*             pafSourceBuf;
    int*               panSourceBuf;
    int                nCurBlockXOff;
    int                nCurBlockYOff;

  public:
                        GDALColorReliefDataset(GDALDatasetH hSrcDS,
                                            GDALRasterBandH hSrcBand,
                                            const char* pszColorFilename,
                                            ColorSelectionMode eColorSelectionMode,
                                            int bAlpha);
                       ~GDALColorReliefDataset();

    CPLErr      GetGeoTransform( double * padfGeoTransform );
    const char *GetProjectionRef();
};

/************************************************************************/
/* ==================================================================== */
/*                    GDALColorReliefRasterBand                       */
/* ==================================================================== */
/************************************************************************/

class GDALColorReliefRasterBand : public GDALRasterBand
{
    friend class GDALColorReliefDataset;

    
  public:
                 GDALColorReliefRasterBand( GDALColorReliefDataset *, int );
    
    virtual CPLErr          IReadBlock( int, int, void * );
    virtual GDALColorInterp GetColorInterpretation();
};

class ColorTable;
class GDALDem : public GDALCommon {
public:
	MULTIMAP_API GDALDem(Algorithm utilityMode = HILL_SHADE);
	MULTIMAP_API int CreateHillshade(GDALDatasetH *phDstDS, std::string dstDSName, std::string driverName, GDALDatasetH hSrcDataset);
	MULTIMAP_API int CreateHillshade(GDALDatasetH *phDstDS, std::string dstDSName, GDALDriverH hDriver, GDALDatasetH hSrcDataset);
	MULTIMAP_API int CreateColorRelief(GDALDatasetH *phDstDS, std::string dstDSName, std::string driverName, GDALDatasetH hSrcDataset, std::string colorFilename);
	MULTIMAP_API int CreateColorRelief(GDALDatasetH *phDstDS, std::string dstDSName, GDALDriverH hDriver, GDALDatasetH hSrcDataset, std::string colorFilename);

	std::string errorMessage;

	Algorithm eUtilityMode;
	std::string colorFilename;
	//Hillshade
	double z;
	double scale;
	double az;
	double alt;
	int slopeFormat; 
	int bAddAlpha;
	int bZeroForFlat;
	int bAngleAsAzimuth;
	ColorSelectionMode eColorSelectionMode;
	int nBand;
	double  adfGeoTransform[6];

	int bFormatExplicitelySet;
	char **papszCreateOptions;

	GDALDatasetH hSrcDataset;
	GDALDatasetH hDstDataset;
	GDALRasterBandH hSrcBand;
	GDALRasterBandH hDstBand;
	GDALDriverH hDriver;

	int nXSize;
	int nYSize;

	int bComputeAtEdges;
	int bZevenbergenThorne;
	int bCombined;
	int bQuiet;

	bool computeEdges;
	bool combined;
	bool ZevenbergenThorne;
	//Color Relief
	bool alpha;

	GDALProgressFunc pfnProgress;


	static double GDALColorReliefGetAbsoluteValFromPct(GDALRasterBandH hSrcBand, double dfPct);
	static ColorAssociation* GDALColorReliefParseColorFile(GDALRasterBandH hSrcBand, ColorTable *colorTable);
	static ColorAssociation* GDALColorReliefParseColorFile(GDALRasterBandH hSrcBand, const char* pszColorFilename, int* pnColors);
	static CPLErr GDALColorRelief (GDALRasterBandH hSrcBand, GDALRasterBandH hDstBand1, GDALRasterBandH hDstBand2, GDALRasterBandH hDstBand3, GDALRasterBandH hDstBand4, 
		                             const char* pszColorFilename, ColorSelectionMode eColorSelectionMode, GDALProgressFunc pfnProgress, void * pProgressData);
private:
	int Run(GDALDatasetH *phDstDS, std::string dstDSName, std::string driverName, GDALDatasetH hSrcDataset, std::string colorFilename);
	int Run(GDALDatasetH *phDstDS, std::string dstDSName, GDALDriverH hDriver, GDALDatasetH hSrcDataset, std::string colorFilename);
	double dfMin, dfMax;

};