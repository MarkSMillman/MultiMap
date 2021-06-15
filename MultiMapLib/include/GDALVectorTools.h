#pragma once
#include "MultiMap.h"
#include "GDALCommon.h"
#include "MBRect.h"

class OGRLayer;
class OGRDataSource;
class OGRGeometry;
class GDALRasterBand;
class GDALVectorTools : public GDALCommon {
public:
	MULTIMAP_API ~GDALVectorTools();
	MULTIMAP_API GDALVectorTools();

	MULTIMAP_API OGRLayer* Eliminate(OGRDataSource* pDstDS, std::string layerName, OGRLayer* pSrcLayer, double lengthTol, double areaTol, double apRatio);
	MULTIMAP_API OGRLayer* Simplify(OGRDataSource* pDstDS, std::string layerName, OGRLayer* pSrcLayer, double dTolerance, bool preserveTopology, double lengthTol=0.0, double areaTol=0.0, double apRatio=0.0);
	MULTIMAP_API OGRLayer* Polygonize(GDALRasterBand* pBand, OGRLayer* pLayer, bool simplify, int gType = 3);
	MULTIMAP_API OGRLayer* Contour(GDALRasterBand* pBand, OGRLayer* pLayer, double interval, double base, int fixedLevelCount, int idField, int idElev);
	MULTIMAP_API size_t PointCount(OGRGeometry* pGeometry);

	size_t inPointCount;
	size_t outPointCount;
};