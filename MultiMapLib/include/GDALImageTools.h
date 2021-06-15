#pragma once
#include "MultiMap.h"
#include "GDALCommon.h"
#include "MBRect.h"
#include <string>

class ImageTransformInfo {
public:
	ImageTransformInfo(void) {resolution = 1.0;noData=-32767;xSize=ySize=0;};
	MBRect         range;
	double resolution;
	double     noData;
	int         xSize;
	int         ySize;
};

class GDALDataset;
class OGRLayer;
class OGRGeometry;
class GDALRasterBand;
class GDALOCIConnector;
class GDALImageTools : public GDALCommon {
public:
	MULTIMAP_API ~GDALImageTools();
	MULTIMAP_API GDALImageTools();

	MULTIMAP_API bool MergeHillshade(GDALDataset** ppMergedDS, std::string mergedName, GDALDataset* pBaseDS, GDALDataset* pHillShadeDS, float greyRatio=0.5);

	MULTIMAP_API int SieveFilter(GDALDataset* pDstDS, int threshold, bool allNeighbors = false, int bandNum = 1);
	MULTIMAP_API GDALRasterBand* SieveFilter(GDALRasterBand* pDstBand, GDALRasterBand* pSrcBand, int threshold, bool allNeighbors = false);
	MULTIMAP_API GDALDataset* Rasterize(OGRGeometry* pGeometry, double resolution, bool touchAll=false, GDALDataset* pDstDS=NULL);
	MULTIMAP_API GDALDataset* Rasterize(OGRLayer* pLayer, double resolution, bool touchAll=false, GDALDataset* pDataset=NULL);
	MULTIMAP_API GDALDataset* Rasterize2(OGRLayer* pLayer, double resolution, MBRect* range = NULL, bool touchAll=false, GDALDataset* pDataset=NULL);
	MULTIMAP_API int Invert(GDALRasterBand* pBand);
	MULTIMAP_API int Invert(GDALRasterBand* pDstBand, GDALRasterBand* pSrcBand);

	MULTIMAP_API void setOrigin(double origin[2]);
	MULTIMAP_API void setOrigin(double x, double y);
	MULTIMAP_API double* getOrigin(void);

	MULTIMAP_API void setGroundSize(double range[2]);
	MULTIMAP_API void setGroundSize(double width, double height);
	MULTIMAP_API double getGroundWidth(void);
	MULTIMAP_API double getGroundHeight(void);

	MULTIMAP_API void setPaperSize(double range[2]);
	MULTIMAP_API void setPaperSize(double width, double height);
	MULTIMAP_API double getPaperWidth(void);
	MULTIMAP_API double getPaperHeight(void);

	MULTIMAP_API void setScale(double scale);
	MULTIMAP_API double getScale(void);

	MULTIMAP_API void setDPI(int dpi);
	MULTIMAP_API int getDPI(void);
	MULTIMAP_API void setDPCM(int dpcm);
	MULTIMAP_API int getDPCM(void);

	MULTIMAP_API void setTransparency(bool transparency);
	MULTIMAP_API bool getTransparency(void);

	MULTIMAP_API void setBackgroundColor(unsigned char red, unsigned char green, unsigned char blue);
	MULTIMAP_API void setBackgroundColor(unsigned int color);
	MULTIMAP_API void setBackgroundColor(const char* namedColor, unsigned int onFailedLookup = 0x00FFFFFF);
	MULTIMAP_API unsigned int getBackgroundColor(void);

	MULTIMAP_API void setSourceFolder(std::string sourceFolder);
	MULTIMAP_API std::string getSourceFolder(void);

	MULTIMAP_API void setTargetFolder(std::string sourceFolder);
	MULTIMAP_API std::string getTargetFolder(void);

	MULTIMAP_API void setPixelsX(int pixels);
	MULTIMAP_API int getPixelsX(void);
	MULTIMAP_API void setPixelsY(int pixels);
	MULTIMAP_API int getPixelsY(void);

	MULTIMAP_API void setVectorPointReductionTolerance(double tolerance);
	MULTIMAP_API double getVectorPointReductionTolerance(void);

private: 
	GDALOCIConnector* connector;
	double origin[2];
	double groundWidth;
	double groundHeight;
	double paperWidth;
	double paperHeight;
	double scale;
	int  dpi;
	std::string sourceFolder;
	std::string targetFolder;
	bool transparency;
	unsigned int backgroundColor;

	int pixelsX;
	int pixelsY;

	double vectorPointReductionTolerance;

	std::string sourceProjWKT;
	std::string targetProjWKT;
};