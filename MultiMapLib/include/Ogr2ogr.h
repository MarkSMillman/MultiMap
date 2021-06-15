#pragma once
#include "MultiMap.h"

class GDALOCIConnector;
class OGRDataSource;
class Ogr2ogr {
public:
	MULTIMAP_API ~Ogr2ogr();
	MULTIMAP_API Ogr2ogr();

	MULTIMAP_API int Run(std::vector<std::string> args, OGRDataSource* pSrcDS=NULL);
	MULTIMAP_API int Ogr2ogr::Run(std::string newTableName, int dim, int srid, std::string geomField, std::string otherFields, bool index, GDALOCIConnector* connector, OGRDataSource* pSrcDS);
private:
	int Run(int nArgs, char** args, OGRDataSource* pSrcDS=NULL);
	int errCode;
};