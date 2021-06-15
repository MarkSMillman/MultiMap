#pragma once
#include "MultiMap.h"
#include "gdal.h"
#include "ogr_api.h"

class RPolygon;
class GDALPolygonize2 {
public:
	MULTIMAP_API ~GDALPolygonize2();
	MULTIMAP_API GDALPolygonize2();
	MULTIMAP_API int Run( GDALRasterBandH hSrcBand, GDALRasterBandH hMaskBand, OGRLayerH hOutLayer, int iPixValField, char **papszOptions, GDALProgressFunc pfnProgress, void * pProgressArg, bool usePixelCenters = false );
private:
	static CPLErr EmitPolygonToLayer( OGRLayerH hOutLayer, int iPixValField, RPolygon *poRPoly, double *padfGeoTransform, bool usePixelCenters=false);
};