#pragma once
#include "MultiMap.h"
#include "GDALCommon.h"
#include <string>
#include <vector>
#include "MBRect.h"

/**
Change the MULTIMAP_API method arguments from std::string to char* and from std::vector to array pointers.
To enable wrapping in JNDI.  Check out JNDI first
**/



class OGRSFDriver;
class OGRDataSource;
class OGRLayer;
class OGRGeometry;
class OGRSpatialReference;
class GDALDataset;
class GDALRasterBand;
class Logger;
class GDALOCIConnector : public GDALCommon {
public:
	MULTIMAP_API ~GDALOCIConnector();
	MULTIMAP_API GDALOCIConnector();

	MULTIMAP_API GDALOCIConnector(std::string username, std::string password, std::string tnsname);
	MULTIMAP_API GDALOCIConnector(std::string username, std::string password, std::string tnsname, std::string tableName);
	MULTIMAP_API GDALOCIConnector(std::string username, std::string password, std::string tnsname, std::vector<std::string> tableNames);

	MULTIMAP_API OGRDataSource* Connect();
	MULTIMAP_API OGRDataSource* Connect( std::string username, std::string password, std::string tnsname);
	MULTIMAP_API OGRDataSource* Connect( std::string username, std::string password, std::string tnsname, std::string tableName);
	MULTIMAP_API OGRDataSource* Connect( std::string username, std::string password, std::string tnsname, std::vector<std::string> tableNames);
	MULTIMAP_API GDALDataset* ConnectGDAL();
	MULTIMAP_API GDALDataset* GetDataset();
	MULTIMAP_API OGRDataSource* GetDataSource();

	MULTIMAP_API int CreateLayerFork(OGRDataSource* pSrcDS, std::string format, std::string tableName, std::string geomFieldName, std::vector<std::string> fieldNames, int dim, int srid, bool index=true);
	MULTIMAP_API int CreateLayerFork(const char* szInDataset, std::string format, std::string tableName, std::string geomFieldName, std::vector<std::string> fieldNames, int dim, int srid, bool index=true);
	MULTIMAP_API int CreateLayer(std::string newTableName, int dim, int srid, std::string geomField, std::string select, bool index, OGRDataSource* pSrcDS);

	MULTIMAP_API void CreateDLM(std::string outTable, int outClassNum, double resolution, std::string inTableName, MBRect & range, std::string geomColumn, std::string attrColumns,std::string classField,const int nClasses,int* classes, std::string className, bool db=true, bool local=false, bool keepOrig=false);

	MULTIMAP_API void CreateDLM(std::string outTable, double resolution, std::string inTableName, MBRect & range, std::string geomColumn, std::string attrColumns,std::string classField,const int nClasses,int* classes,const char** classNames, bool db=true, bool local=false, bool keepOrig=false);
	MULTIMAP_API void GDALOCIConnector::CreateDLM(std::string outTableName, int outClassNum, double resolution, std::string inTableName, MBRect & range, std::string geomColumn, std::string attrColumns, std::string classField, int classNum, std::string className, bool db=true, bool local=false, bool keepOrig=false);

	MULTIMAP_API void CreateDLM(std::string outTable, double resolution, std::string sqlQuery, MBRect & range, std::string geomColumn, std::string attrColumns,std::string classField, int outClass, std::string className, bool db=true, bool local=false, bool keepOrig=false);

	MULTIMAP_API void CreateDLMThinOnly(std::string outTableName, int outClassNum, double resolution, std::string inTableName, MBRect & range, std::string geomColumn, std::string attrColumns,std::string classField,std::vector<int> inClasses);

	MULTIMAP_API int GetDimension(OGRLayer* pInLayer);
	MULTIMAP_API void Close();
	MULTIMAP_API OGRLayer* GetLayerBySQL(const char* sql);
	MULTIMAP_API std::string GetDatabaseSpecifier(void);
	MULTIMAP_API int DeleteLayer(const char* layerName);
	MULTIMAP_API int Open(const char* tableName, int nSRID=0, int bUpdate=TRUE, int bTestOpen=TRUE);

	//MULTIMAP_API int ImgProjTransform( void *pTransformArg, int bDstToSrc, int nPointCount, double *x, double *y, double *z, int *panSuccess );
	//MULTIMAP_API OGRLayer* Polygonize(GDALRasterBand* pBand, OGRLayer* pLayer = NULL, int gType = 3);
	//MULTIMAP_API GDALDataset* Rasterize(OGRGeometry* pGeometry, double resolution, bool touchAll=false, GDALDataset* pDstDS=NULL);
	//MULTIMAP_API GDALDataset* Rasterize(OGRLayer* pLayer, double resolution, bool touchAll=false, GDALDataset* pDataset=NULL);

	//MULTIMAP_API const char** GetPrimaryKeys(size_t *keysFound);
	//MULTIMAP_API const char* GetPrimaryKey(const char* tableName);

	MULTIMAP_API int Import(std::string tableName, std::string geomColumnName, std::string importFilePath);
	MULTIMAP_API int Export(std::string tableName, std::string importFilePath, MBRect *range = NULL);

	// Connection accessors and mutators
	MULTIMAP_API void setUsername(std::string username);
	MULTIMAP_API void setPassword(std::string password);
	MULTIMAP_API void setTnsname(std::string tnsname);
	MULTIMAP_API void setTableName(std::string tableName);
	MULTIMAP_API void setTableNames(std::vector<std::string> tableNames);
	MULTIMAP_API void addTableName(std::string tableName);
	MULTIMAP_API std::string getUsername(void);
	MULTIMAP_API std::string getTnsname(void);
	MULTIMAP_API std::vector<std::string> getTableNames(void);

	// Layer creation option accessors and mutators
	MULTIMAP_API void setDefaultLayerCreationOptions(void);
	MULTIMAP_API void setOverwrite(bool option=true);
	MULTIMAP_API void setTruncate(bool option=true);
	MULTIMAP_API void setLaunder(bool option=true);
	MULTIMAP_API void setDim(int dim);
	MULTIMAP_API void set2D();
	MULTIMAP_API void set3D();
	MULTIMAP_API void setIndex(bool option=true);
	MULTIMAP_API void setIndexParameters(std::string parameters);
	MULTIMAP_API void setDiminfoX(double min, double max, double res);
	MULTIMAP_API void setDiminfoX(double values[3]);
	MULTIMAP_API void setDiminfoY(double min, double max, double res);
	MULTIMAP_API void setDiminfoY(double values[3]);
	MULTIMAP_API void setDiminfoZ(double min, double max, double res);
	MULTIMAP_API void setDiminfoZ(double values[3]);
	MULTIMAP_API void setSRID(int srid);
	MULTIMAP_API void setMultiLoad(bool option=true);
	MULTIMAP_API void setSqlLoaderFile(std::string sqlLoaderFilePath);
	MULTIMAP_API void setGeomColumnName(std::string geomColumnName);
	MULTIMAP_API void setWorkFolder(std::string folderPath);

	MULTIMAP_API void setLoggerLevel(int level);

	MULTIMAP_API bool getOverwrite();
	MULTIMAP_API bool getTruncate();
	MULTIMAP_API bool getLaunder();
	MULTIMAP_API int getDim();
	MULTIMAP_API bool is2D();
	MULTIMAP_API bool is3D();
	MULTIMAP_API bool getIndex();
	MULTIMAP_API std::string getIndexParameters();
	MULTIMAP_API double* getDiminfoX();
	MULTIMAP_API double* getDiminfoY();
	MULTIMAP_API double* getDiminfoZ();
	MULTIMAP_API int getSRID();
	MULTIMAP_API bool getMultiLoad();
	MULTIMAP_API std::string getSqlLoaderFile();
	MULTIMAP_API std::string getGeomColumnName();
	MULTIMAP_API std::string getWorkFolder();

	MULTIMAP_API void setThinOnly(bool to=true);
	MULTIMAP_API void setProductId(int productid);
	MULTIMAP_API void setSequenceName(std::string sequenceName);
	MULTIMAP_API bool getThinOnly();
	MULTIMAP_API int getProductId();
	MULTIMAP_API std::string getSequenceName();


	MULTIMAP_API static std::string GetSDO_Rectangle(MBRect range, int srid, int p=0);
	MULTIMAP_API static std::string GetSDO_Rectangle(double minX, double minY, double maxX, double maxY, int srid, int p=0);
	MULTIMAP_API static std::string GetSDO_AnyinteractClause(std::string alias, std::string column, MBRect range, int srid, int p=0);
	MULTIMAP_API static std::string GetSDO_AnyinteractClause(std::string alias, std::string column, double minX, double minY, double maxX, double maxY, int srid, int p=0);

	MULTIMAP_API static bool WritePRJ(std::string filePath, int epsg);
	MULTIMAP_API static bool WritePRJ(std::string filePath, OGRLayer* pLayer);
	MULTIMAP_API static bool WritePRJ(std::string filePath, OGRSpatialReference* pSRS);
	MULTIMAP_API static bool WritePRJ(std::string filePath, const char* prj);

//MOVE TO PRIVATE
	bool erode;
	bool fill;
private:
	void RegisterOCIDriver(void);
	std::string BuildConnectionString(void);
	void GDALOCIConnector::ReportLastError(void);

	// Connection parameters
	std::string username;
	std::string password;
	std::string tnsname;

	std::vector<std::string> tableNames;

	std::string workFolder;

	double lastRunTime;

	// Layer creation options
	bool overwrite;
	bool truncate;
	bool launder;
	bool precision;
	int dim;
	bool index;
	std::string indexParameters;
	double diminfoX[3];
	double diminfoY[3];
	double diminfoZ[3];
	int srid;
	bool multiLoad;
	std::string sqlLoaderFile;
	std::string geomColumnName;

	int productId;
	std::string sequenceName;
	bool thinOnly;

	OGRDataSource* pDataSource;
	OGRSFDriver*   pDriver;
	Logger*        logger;
};