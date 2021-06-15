#pragma once
#include "MultiMap.h"
#include "gdal.h"
#include "gdal_alg.h"
#include "ogr_api.h"
#include "ogr_geometry.h"
#include "cpl_progress.h"

class GDALGrid {
public:
	MULTIMAP_API ~GDALGrid();
	MULTIMAP_API GDALGrid();
	MULTIMAP_API int Run(std::vector<std::string> args);

	GDALDriverH     hDriver;
	const char      *pszSource;
	const char      *pszDest;
	const char      *pszFormat;
	int             bFormatExplicitelySet;
	char            **papszLayers;
	const char      *pszBurnAttribute;
	double          dfIncreaseBurnValue;
	double          dfMultiplyBurnValue;
	const char      *pszWHERE;
	const char      *pszSQL;
	GDALDataType    eOutputType;
	char            **papszCreateOptions;
	GUInt32         nXSize;
	GUInt32         nYSize;
	double          dfXMin;
	double          dfXMax;
	double          dfYMin;
	double          dfYMax;
	int             bIsXExtentSet;
	int             bIsYExtentSet;
	GDALGridAlgorithm eAlgorithm;
	void            *pOptions;
	char            *pszOutputSRS;
	int             bQuiet;
	GDALProgressFunc pfnProgress;
	OGRGeometry     *poSpatialFilter;
	int             bClipSrc;
	OGRGeometry     *poClipSrc;
	const char      *pszClipSrcDS;
	const char      *pszClipSrcSQL;
	const char      *pszClipSrcLayer;
	const char      *pszClipSrcWhere;

	bool             useLaplace;
private:
	MULTIMAP_API int Run(int argc, char ** argv);
};