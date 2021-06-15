#pragma warning ( disable : 4251 )
#include "MultiMap.h"
#include "MFUtils.h"
#include "Logger.h"
#include "GDALOCIConnector.h"
#include "ThreadTools.h"
#include "ByteBuffer.h"

#include "GDALImageTools.h"

DISABLE_WARNINGS
#include "GDALVectorTools.h"
#include "gdal_priv.h"
#include "gdal_alg.h"
#include "ogr_api.h"
#include "ogr_core.h"
#include "ogrsf_frmts.h"
#include "ogr_oci.h"
#include "Ogr2ogr.h"
ENABLE_WARNINGS

/**
@see http://www.gdal.org/drv_oci.html

Oracle Spatial

This driver supports reading and writing data in Oracle Spatial (8.1.7 or later) Object-Relational format. 
The Oracle Spatial driver is not normally built into OGR, but may be built in on platforms where the Oracle client libraries are available.
When opening a database, it's name should be specified in the form "OCI:userid/password@database_instance:table,table". 
The list of tables is optional. The database_instance portion may be omitted when accessing the default local database instance.

If the list of tables is not provided, then all tables appearing in ALL_SDO_GEOM_METADATA will be treated by OGR as layers with the 
table names as the layer names. Non-spatial tables or spatial tables not listed in the ALL_SDO_GEOM_METADATA table are not accessable 
unless explicitly listed in the datasource name. Even in databases where all desired layers are in the ALL_SDO_GEOM_METADATA table, 
it may be desirable to list only the tables to be used as this can substantially reduce initialization time in databases with many tables.

If the table has an integer column called OGR_FID it will be used as the feature id by OGR (and it will not appear as a regular attribute). 
When loading data into Oracle Spatial OGR will always create the OGR_FID field.

SQL Issues

By default, the Oracle driver passes SQL statements directly to Oracle rather than evaluating them internally when using the ExecuteSQL() 
call on the OGRDataSource, or the -sql command option to ogr2ogr. Attribute query expressions are also passed through to Oracle.

As well two special commands are supported via the ExecuteSQL() interface. 
These are "DELLAYER:<table_name>" to delete a layer, and 
"VALLAYER:<table_name>" to apply the SDO_GEOM.VALIDATE_GEOMETRY() check to a layer. 
Internally these pseudo-commands are translated into more complex SQL commands for Oracle.

It's also possible to request the driver to handle SQL commands with OGR SQL engine, by passing "OGRSQL" string to the ExecuteSQL() method,
as name of the SQL dialect.

Caveats

The type recognition logic is currently somewhat impoverished. No effort is made to preserve real width information for integer and real fields.
Various types such as objects, and BLOBs in Oracle will be completely ignored by OGR.
Currently the OGR transaction semantics are not properly mapped onto transaction semantics in Oracle.
If an attribute called OGR_FID exists in the schema for tables being read, it will be used as the FID. Random (FID based) 
reads on tables without an identified (and indexed) FID field can be very slow. To force use of a particular field name the OCI_FID 
configuration variable (ie. environment variable) can be set to the target field name.
Curved geometry types are converted to linestrings or linear rings in six degree segments when reading. The driver has no support 
for writing curved geometries.
There is no support for point cloud (SDO_PC), TIN (SDO_TIN) and annotation text data types in Oracle Spatial.

Creation Issues

The Oracle Spatial driver does not support creation of new datasets (database instances), 
but it does allow creation of new layers within an existing database.
Upon closing the OGRDataSource newly created layers will have a spatial index automatically built. 
At this point the USER_SDO_GEOM_METADATA table will also be updated with bounds for the table based on the features that have 
actually been written. One concequence of this is that once a layer has been loaded it is generally not possible to load 
additional features outside the original extents without manually modifying the DIMINFO information in USER_SDO_GEOM_METADATA and 
rebuilding the spatial index.

Example

Simple translation of a shapefile into Oracle. The table 'ABC' will be created with the features from abc.shp and attributes from abc.dbf.
% ogr2ogr -f OCI OCI:warmerda/password@gdal800.dreadfest.com abc.shp
This second example loads a political boundaries layer from VPF (via the OGDI driver), and renames the layer from the cryptic OGDI layer name to something more sensible. If an existing table of the desired name exists it is overwritten.

% ogr2ogr  -f OCI OCI:warmerda/password gltp:/vrf/usr4/mpp1/v0eur/vmaplv0/eurnasia -lco OVERWRITE=yes -nln polbndl_bnd 'polbndl@bnd(*)_line'

This example shows using ogrinfo to evaluate an SQL query statement within Oracle. More sophisticated Oracle Spatial specific queries may 
also be used via the -sql commandline switch to ogrinfo.

ogrinfo -ro OCI:warmerda/password -sql "SELECT pop_1994 from canada where province_name = 'Alberta'"

@see http://www.gdal.org/drv_oci.html
**/

MULTIMAP_API GDALOCIConnector::~GDALOCIConnector(){
}
MULTIMAP_API GDALOCIConnector::GDALOCIConnector(){
	RegisterOCIDriver();
	setDefaultLayerCreationOptions();
	pDriver = NULL;
	pDataSource = NULL;
}
MULTIMAP_API GDALOCIConnector::GDALOCIConnector(std::string username, std::string password, std::string tnsname){
	RegisterOCIDriver();
	setUsername(username);
	setPassword(password);
	setTnsname(tnsname);
	setDefaultLayerCreationOptions();
}
MULTIMAP_API GDALOCIConnector::GDALOCIConnector(std::string username, std::string password, std::string tnsname, std::string tableName) {
	RegisterOCIDriver();
	setUsername(username);
	setPassword(password);
	setTnsname(tnsname);
	setTableName(tableName);
	setDefaultLayerCreationOptions();
}
MULTIMAP_API GDALOCIConnector::GDALOCIConnector(std::string username, std::string password, std::string tnsname, std::vector<std::string> tableNames) {
	RegisterOCIDriver();
	setUsername(username);
	setPassword(password);
	setTnsname(tnsname);
	setTableNames(tableNames);
	setDefaultLayerCreationOptions();
}
MULTIMAP_API int GDALOCIConnector::DeleteLayer(const char* layerName) {
	int err = 1;
	int layerCount = pDataSource->GetLayerCount();
	for ( int i=0; i<layerCount; i++ ) {
		OGRLayer* pLayer = pDataSource->GetLayer(i);
		const char* name = pLayer->GetName();
		if ( strcmp(name,layerName) == 0 ) {
			pDataSource->DeleteLayer(i);
			err = 0;
			break;
		}
	}
	if ( err != 0 ) {
		char buffer[64];
		sprintf(buffer,"DROP TABLE %s",layerName);
		OGRLayer* x = pDataSource->ExecuteSQL((const char*)buffer,NULL,NULL);
		err = x==NULL?0:0;
	}
	return err;
}
MULTIMAP_API int GDALOCIConnector::Open(const char* tableName, int nSRID, int bUpdate, int bTestOpen) {
	OGROCIDataSource* pOciDataSource = (OGROCIDataSource*)pDataSource;
	return pOciDataSource->Open(tableName,bUpdate,bTestOpen);
}
MULTIMAP_API int GDALOCIConnector::GetDimension(OGRLayer* pInLayer) {
	int dimensions = 2;
	if ( pInLayer ) {
		int featureCount = pInLayer->GetFeatureCount();
		if ( featureCount > 0 ) {
			OGRFeature* pFeature = pInLayer->GetFeature(0);
			if ( pFeature ) {
				OGRGeometry* pGeomRef = pFeature->GetGeometryRef();
				if ( pGeomRef ) {
					dimensions= pGeomRef->getDimension();
				}
			}
		}
	}
	return dimensions;
}
MULTIMAP_API int GDALOCIConnector::CreateLayerFork(OGRDataSource* pSrcDS, std::string format, std::string tableName, std::string geomFieldName, std::vector<std::string> fieldNames, int dim, int srid, bool index) {
	int errCode = 0;
	if ( pSrcDS ) {
		if ( dim != 2 && dim != 3 ) {
			OGRLayer* pInLayer = pSrcDS->GetLayer(0);
			dim = GetDimension(pInLayer);
		}

		errCode = CreateLayerFork(pSrcDS->GetName(),format,tableName,geomFieldName, fieldNames,dim,srid,index);
	}
	return errCode;
}
MULTIMAP_API int GDALOCIConnector::CreateLayerFork(const char* szInDataset, std::string format, std::string tableName, std::string geomFieldName, std::vector<std::string> fieldNames, int dim, int srid, bool index) {
	int errCode = 0;

	const char* dataSourceName = pDataSource->GetName();
	DeleteLayer(tableName.c_str());

	std::string command;
	command.append("ogr2ogr");
	command.append(" -skipfailures");
	command.append(" -preserve_fid");
	command.append(" -f ");
	command.append(format);
	if ( dim == 2 || dim == 3 ) {
		command.append(" -dim ");
		char buf[8];
		sprintf(buf,"%d",dim);
		command.append(buf);
	}
	if ( fieldNames.size() > 0 ) {
		command.append(" -select ");
		for ( size_t i=0; i<fieldNames.size(); i++ ) {
			command.append(fieldNames[i]);
			if ( i!=fieldNames.size()-1 ) {
				command.append(",");
			}
		}
	}
	command.append(" -nln ");
	command.append(tableName);
	if ( index ) {
		command.append(" -lco INDEX=YES");
	}
	if ( geomFieldName.length() > 0 ) {
		command.append(" -geomfield ");
		command.append(geomFieldName);
		command.append(" -lco GEOMETRY_NAME=");
		command.append(geomFieldName);
	}
	if ( srid > 0 ) {
		command.append(" -lco SRID=");
		char buf[12];
		sprintf(buf,"%d",srid);
		command.append(buf);
	}
	{
		char buf[32];
		command.append(" -lco DIMINFO_X=");
		sprintf(buf,"%.0f,%.0f,%f",diminfoX[0],diminfoX[1],diminfoX[2]);
		command.append(buf);
		command.append(" -lco DIMINFO_Y=");
		sprintf(buf,"%.0f,%.0f,%f",diminfoY[0],diminfoY[1],diminfoY[2]);
		command.append(buf);
		if ( dim > 2 ) {
			command.append(" -lco DIMINFO_Z=");
			sprintf(buf,"%.0f,%.0f,%f",diminfoZ[0],diminfoZ[1],diminfoZ[2]);
			command.append(buf);
		}
	}
	command.append(" ");
	command.append(dataSourceName);
	command.append(" ");
	command.append(szInDataset);

	ThreadTools launcher;
#ifdef _DEBUG
	printf("%s",command.c_str());
#endif
	errCode = launcher.RunAndWait(command);

	return errCode;
}

MULTIMAP_API int GDALOCIConnector::CreateLayer(std::string newTableName, int dim, int srid, std::string geomField, std::string select, bool index, OGRDataSource* pSrcDS) {
	int errcode = 0;

	char szDim[2] = {0,0};
	if ( dim == 2 )
		strcpy(szDim,"2");
	else if ( dim == 3 )
		strcpy(szDim,"3");

	char szSrid[16];
	sprintf(szSrid,"SRID=%d",srid);

	char szGeomField[64];
	sprintf(szGeomField,"GEOMETRY_NAME=%s",geomField.c_str());

	double* diminfoX = getDiminfoX();
	double* diminfoY = getDiminfoY();
	std::string dbSource = GetDatabaseSpecifier();
	char szDiminfoX[128];
	char szDiminfoY[128];
	sprintf(szDiminfoX,"DIMINFO_X=%.0f,%.0f,%.5f",diminfoX[0],diminfoX[1],diminfoX[2]);
	sprintf(szDiminfoY,"DIMINFO_Y=%.0f,%.0f,%.5f",diminfoY[0],diminfoY[1],diminfoY[2]);

	std::vector<std::string> args;
	args.push_back("ogr2ogr");
	args.push_back("-skipfailures");
	args.push_back("-preserve_fid");
	args.push_back("-f");
	args.push_back("OCI");
	if ( szDim[0] ) {
		args.push_back("-dim");
		args.push_back(szDim);
	}
	args.push_back("-select");
	args.push_back((char*)select.c_str());
	args.push_back("-geomfield");
	args.push_back((char*)geomField.c_str());
	args.push_back("-lco");
	if ( index ) {
		args.push_back("INDEX=YES");
	} else {
		args.push_back("INDEX=NO");
	}
	args.push_back("-lco");
	args.push_back(szSrid);
	args.push_back("-lco");
	args.push_back(szGeomField);
	args.push_back("-lco");
	args.push_back(szDiminfoX);
	args.push_back("-lco");
	args.push_back(szDiminfoY);
	args.push_back("-nln"); // new table name
	args.push_back((char*)newTableName.c_str()); // new table name
	args.push_back(dbSource.c_str());
	//args.push_back("OCI:mm_dlm/mm_dlm@domain13:DLM_0_WAY"); // dst_datasource

	Ogr2ogr ogr2ogr;
	int errCode = ogr2ogr.Run(args,pSrcDS);
	return errCode;
}
MULTIMAP_API std::string GDALOCIConnector::GetDatabaseSpecifier(void) {
	return BuildConnectionString();
}
PRIVATE void GDALOCIConnector::RegisterOCIDriver(void) {
	defaultErrorHandler = CPLSetErrorHandler(CPLQuietErrorHandler);
	CPLErrorReset ();
	OGRRegisterAll();
}
PRIVATE std::string GDALOCIConnector::BuildConnectionString(){
	std::string connector = "OCI:"+username+"/"+password+"@"+tnsname;
	if ( tableNames.size() > 0 ) {
		connector.append(":");

		for ( size_t t=0; t<tableNames.size(); t++ ) {
			if ( t>0 ) {
				connector.append(",");
			}
			connector.append(tableNames[t]);
		}
	}
	return connector;
}
MULTIMAP_API GDALDataset* GDALOCIConnector::GetDataset() {
	if ( !pDataSource ) {
		pDataSource = Connect();
	}
	return (GDALDataset*)pDataSource;
}
MULTIMAP_API OGRDataSource* GDALOCIConnector::GetDataSource() {
	if ( !pDataSource ) {
		pDataSource = Connect();
	}
	return pDataSource;
}
MULTIMAP_API OGRDataSource* GDALOCIConnector::Connect(){
	Close();
	CPLErrorReset ();
	std::string connector = BuildConnectionString();
	pDataSource = (OGRDataSource*)OGROpenShared(connector.c_str(),0,(OGRSFDriverH*)&pDriver);

	return pDataSource;
}
MULTIMAP_API GDALDataset* GDALOCIConnector::ConnectGDAL(){
	GDALDataset* pDS = NULL;
	Close();
	CPLErrorReset ();
	std::string connector = BuildConnectionString();
	GDALDatasetH hDS = GDALOpen(connector.c_str(),GA_Update);
	if ( hDS ) {
		pDS = (GDALDataset*)hDS;
		GDALDriver* gpDriver = pDS->GetDriver();
		pDriver = (OGRSFDriver*) gpDriver;
	}

	return pDS;
}
MULTIMAP_API OGRDataSource* GDALOCIConnector::Connect( std::string username, std::string password, std::string tnsname){
	setUsername(username);
	setPassword(password);
	setTnsname(tnsname);
	return Connect();
}
MULTIMAP_API OGRDataSource* GDALOCIConnector::Connect( std::string username, std::string password, std::string tnsname, std::string tableName) {
	setUsername(username);
	setPassword(password);
	setTnsname(tnsname);
	setTableName(tableName);
	return Connect();
}
MULTIMAP_API OGRDataSource* GDALOCIConnector::Connect( std::string username, std::string password, std::string tnsname, std::vector<std::string> tableNames) {
	setUsername(username);
	setPassword(password);
	setTnsname(tnsname);
	setTableNames(tableNames);
	return Connect();
}
MULTIMAP_API void GDALOCIConnector::Close(){
	CPLErrorReset ();
	if ( pDataSource ) {
		OGRDataSource::DestroyDataSource(pDataSource);
		pDataSource = NULL;
	}
}

// These are incomplete and of dubious value.  Used them to test query methods.
//MULTIMAP_API const char** GDALOCIConnector::GetPrimaryKeys(size_t *keysFound) {
//	char** keys = NULL;
//	if ( pDataSource ) {
//
//	}
//	return (const char**) keys;
//}
//MULTIMAP_API const char* GDALOCIConnector::GetPrimaryKey(const char* tableName) {
//	char* key = NULL;
//	if ( pDataSource ) {
//		std::string sql;
//		sql.assign("select c.COLUMN_NAME from USER_IND_COLUMNS c, USER_INDEXES i where i.TABLE_NAME='");
//		sql.append(tableName);
//		sql.append("' and i.INDEX_TYPE = 'NORMAL' and i.UNIQUENESS = 'UNIQUE' and c.INDEX_NAME = i.INDEX_NAME");
//		OGRLayer* pLayer = pDataSource->ExecuteSQL(sql.c_str(), NULL, NULL);
//		if ( pLayer ) {
//			int featureCount = pLayer->GetFeatureCount();
//			if ( featureCount > 0 ) {
//				OGRFeature* pFeature = pLayer->GetFeature(0);
//				key = (char*)pFeature->GetFieldAsString(0);
//			}
//		}
//	}
//	return (const char*) key;
//}
/////////////////////////////////////////////////////////////////////////////////////

MULTIMAP_API void GDALOCIConnector::setUsername(std::string username){
	this->username = username;
}
MULTIMAP_API void GDALOCIConnector::setPassword(std::string password){
	this->password = password;
}
MULTIMAP_API void GDALOCIConnector::setTnsname(std::string tnsname){
	this->tnsname = tnsname;
}
MULTIMAP_API void GDALOCIConnector::setTableName(std::string tableName) {
	tableNames.clear();
	tableNames.resize(0);
	tableNames.push_back(tableName);
}
MULTIMAP_API void GDALOCIConnector::setTableNames(std::vector<std::string> tableNames) {
	this->tableNames.clear();
	this->tableNames.resize(0);
	this->tableNames = tableNames;
}
MULTIMAP_API void GDALOCIConnector::addTableName(std::string tableName) {
	tableNames.push_back(tableName);
}
MULTIMAP_API std::string GDALOCIConnector::getUsername(void){
	return username;
}
MULTIMAP_API std::string GDALOCIConnector::getTnsname(void){
	return tnsname;
}
MULTIMAP_API std::vector<std::string> GDALOCIConnector::getTableNames(void) {
	return tableNames;
}

MULTIMAP_API void GDALOCIConnector::setDefaultLayerCreationOptions(void){
	MFUtils mfUtils;
	logger = Logger::Instance();

	overwrite = false;
	truncate = false;
	launder = true;
	precision = true;
	dim = 2;
	index = true;
	indexParameters = "";
	memset(diminfoX,0,sizeof(diminfoX));
	memset(diminfoY,0,sizeof(diminfoY));
	memset(diminfoZ,0,sizeof(diminfoZ));
	srid = 4326;
	multiLoad = true;
	sqlLoaderFile = "";
	geomColumnName = "GEOM";
	workFolder = "C:\\MultiMap\\WorkFolder";
	mfUtils.FolderExists(workFolder,true);

	setUsername("mm_dlm");
	setPassword("mm_dlm");
	setSRID(2157);
	std::string hostName = mfUtils.HostName();
	if ( mfUtils.BeginsWith(hostName,"Aquis",true) ) {
		setTnsname("mcmahon");
	} else {
		setTnsname("domain13");
	}
	setTableName("dlm_0_way");
	setDiminfoX(400000.0,  800000.0, .0005);
	setDiminfoY(500000.0, 1000000.0, .0005);
	setDim(2);

	productId = 0;
	fill = true;
	erode = false;
	verbose=true;
}

/**
@params option set to true to force an existing layer of the desired name to be destroyed before creating the requested layer.
**/
MULTIMAP_API void GDALOCIConnector::setOverwrite(bool option){
	overwrite = option;
}
/**
@params option set to true to force the existing table to be reused, but to first truncate all records in the table, preserving indexes or dependencies.
**/
MULTIMAP_API void GDALOCIConnector::setTruncate(bool option){
	truncate = option;
}
/**
@params option set to true to force new fields created on this layer to have their field names "laundered" into a form more compatible with Oracle. This converts to upper case and converts some special characters like "-" and "#" to "_". The default value is true.
**/
MULTIMAP_API void GDALOCIConnector::setLaunder(bool option){
	launder = option;
}
/**
@params dim set to 2 or 3 to set the dimension of the new layer, default is 2
**/
MULTIMAP_API void GDALOCIConnector::setDim(int dim) {
	if ( dim == 2 || dim == 3 ) {
		this->dim = dim;
	} else {
		CPLError(CE_Failure,1001,"GDALOCIConnector::setDim(%d) is invalid dimension.  Must be 2 or 3", dim);
	}
}
/**
same as setDim(2)
@see #setDim(int)
**/
MULTIMAP_API void GDALOCIConnector::set2D(){
	dim = 2;
}
/**
same as setDim(3)
@see #setDim(int)
**/
MULTIMAP_API void GDALOCIConnector::set3D(){
	dim = 3;
}

/**
@params option set to false to disable creation of a spatial index when a layer load is complete. By default an index is created if any of the layer features have valid geometries.
**/
MULTIMAP_API void GDALOCIConnector::setIndex(bool option){
	index = option;
}
/**
@params parameters set to pass creation parameters when the spatial index is created. For instance setting INDEX_PARAMETERS to SDO_LEVEL=5 would cause a 5 level tile index to be used. By default no parameters are passed causing a default R-Tree spatial index to be created.
**/
MULTIMAP_API void GDALOCIConnector::setIndexParameters(std::string parameters){
	indexParameters = parameters;
}
/**
By default extents are collected from the actual data written.
@param min set SDO_DIM_ELEMENT minimum for X
@param max set SDO_DIM_ELEMENT maximum for X
@param res set SDO_DIM_ELEMENT resolution for X
**/
MULTIMAP_API void GDALOCIConnector::setDiminfoX(double min, double max, double res){
	diminfoX[0] = min;
	diminfoX[1] = max;
	diminfoX[2] = res;
}
/**
By default extents are collected from the actual data written.
@param values set SDO_DIM_ELEMENT minimum, maximum, and resolution for X
**/
MULTIMAP_API void GDALOCIConnector::setDiminfoX(double values[3]){
	diminfoX[0] = values[0];
	diminfoX[1] = values[1];
	diminfoX[2] = values[2];
}
/**
By default extents are collected from the actual data written.
@param min set SDO_DIM_ELEMENT minimum for Y
@param max set SDO_DIM_ELEMENT maximum for Y
@param res set SDO_DIM_ELEMENT resolution for Y
**/
MULTIMAP_API void GDALOCIConnector::setDiminfoY(double min, double max, double res){
	diminfoY[0] = min;
	diminfoY[1] = max;
	diminfoY[2] = res;
}
/**
By default extents are collected from the actual data written.
@param values set SDO_DIM_ELEMENT minimum, maximum, and resolution for Y
**/
MULTIMAP_API void GDALOCIConnector::setDiminfoY(double values[3]){
	diminfoY[0] = values[0];
	diminfoY[1] = values[1];
	diminfoY[2] = values[2];
}
/**
By default extents are collected from the actual data written.
@param min set SDO_DIM_ELEMENT minimum for Z
@param max set SDO_DIM_ELEMENT maximum for Z
@param res set SDO_DIM_ELEMENT resolution for Z
**/
MULTIMAP_API void GDALOCIConnector::setDiminfoZ(double min, double max, double res){
	diminfoZ[0] = min;
	diminfoZ[1] = max;
	diminfoZ[2] = res;
}
/**
By default extents are collected from the actual data written.
@param values set SDO_DIM_ELEMENT minimum, maximum, and resolution for Z
**/
MULTIMAP_API void GDALOCIConnector::setDiminfoZ(double values[3]){
	diminfoZ[0] = values[0];
	diminfoZ[1] = values[1];
	diminfoZ[2] = values[2];
}
/**
@param srid set SRID value
**/
MULTIMAP_API void GDALOCIConnector::setSRID(int srid){
	this->srid = srid;
}
/**
SRID: By default this driver will attempt to find an existing row in the MDSYS.CS_SRS table with a well known text coordinate system exactly matching the one for this dataset. If one is not found, a new row will be added to this table. The SRID creation option allows the user to force use of an existing Oracle SRID item even it if does not exactly match the WKT the driver expects.
MULTI_LOAD: If enabled new features will be created in groups of 100 per SQL INSERT command, instead of each feature being a separate INSERT command. Having this enabled is the fastest way to load data quickly. Multi-load mode is enabled by default, and may be forced off for existing layers or for new layers by setting to NO.
LOADER_FILE: If this option is set, all feature information will be written to a file suitable for use with SQL*Loader instead of inserted directly in the database. The layer itself is still created in the database immediately. The SQL*Loader support is experimental, and generally MULTI_LOAD enabled mode should be used instead when trying for optimal load performance.
GEOMETRY_NAME: By default OGR creates new tables with the geometry column named ORA_GEOMETRY. If you wish to use a different name, it can be supplied with the GEOMETRY_NAME layer creation option.

**/
/**
@params option if [true] new features will be created in groups of 100 per SQL INSERT command, instead of each feature being a separate INSERT command. Having this enabled is the fastest way to load data quickly. Multi-load mode is enabled by default, and may be forced off for existing layers or for new layers by setting to false
**/
MULTIMAP_API void GDALOCIConnector::setMultiLoad(bool option){
	multiLoad = option;
}
/**
If this option is set, all feature information will be written to the file suitable for use with SQL*Loader instead of inserted directly in the database. The layer itself is still created in the database immediately. The SQL*Loader support is experimental, and generally MULTI_LOAD enabled mode should be used instead when trying for optimal load performance.
@param sqlLoaderFilePath 
**/
MULTIMAP_API void GDALOCIConnector::setSqlLoaderFile(std::string sqlLoaderFilePath){
	sqlLoaderFile = sqlLoaderFilePath;
}
/**
By default OGR creates new tables with the geometry column named GEOM. If you wish to use a different name, it can be supplied with the GEOMETRY_NAME layer creation option.
@param geomColumnName
**/
MULTIMAP_API void GDALOCIConnector::setGeomColumnName(std::string geomColumnName){
	this->geomColumnName = geomColumnName;
}
MULTIMAP_API void GDALOCIConnector::setWorkFolder(std::string workFolder){
	this->workFolder = workFolder;
	MFUtils mfUtils;
	mfUtils.FolderExists(workFolder,true);
}
MULTIMAP_API bool GDALOCIConnector::getOverwrite(){
	return overwrite;
}
MULTIMAP_API bool GDALOCIConnector::getTruncate(){
	return truncate;
}
MULTIMAP_API bool GDALOCIConnector::getLaunder(){
	return launder;
}
MULTIMAP_API int GDALOCIConnector::getDim(){
	return dim;
}
MULTIMAP_API bool GDALOCIConnector::is2D(){
	return dim==2?true:false;
}
MULTIMAP_API bool GDALOCIConnector::is3D(){
	return dim==3?true:false;
}
MULTIMAP_API bool GDALOCIConnector::getIndex(){
	return index;
}
MULTIMAP_API std::string GDALOCIConnector::getIndexParameters(){
	return indexParameters;
}
MULTIMAP_API double* GDALOCIConnector::getDiminfoX(){
	return diminfoX;
}
MULTIMAP_API double* GDALOCIConnector::getDiminfoY(){
	return diminfoY;
}
MULTIMAP_API double* GDALOCIConnector::getDiminfoZ(){
	return diminfoZ;
}
MULTIMAP_API int GDALOCIConnector::getSRID(){
	return srid;
}
MULTIMAP_API bool GDALOCIConnector::getMultiLoad(){
	return multiLoad;
}
MULTIMAP_API std::string GDALOCIConnector::getSqlLoaderFile(){
	return sqlLoaderFile;
}
MULTIMAP_API std::string GDALOCIConnector::getGeomColumnName(){
	return geomColumnName;
}
MULTIMAP_API std::string GDALOCIConnector::getWorkFolder(){
	return workFolder;
}
MULTIMAP_API void GDALOCIConnector::setLoggerLevel(int level) {
	logger->SetLevel(level);
}
MULTIMAP_API void GDALOCIConnector::setThinOnly(bool to) {
	thinOnly = to;
}
MULTIMAP_API void GDALOCIConnector::setProductId(int productid) {
	productId = productid;
}
MULTIMAP_API void GDALOCIConnector::setSequenceName(std::string sequenceName) {
	this->sequenceName = sequenceName;
}
MULTIMAP_API bool GDALOCIConnector::getThinOnly() {
	return thinOnly;
}
MULTIMAP_API int GDALOCIConnector::getProductId() {
	return productId;
}
MULTIMAP_API std::string GDALOCIConnector::getSequenceName() {
	return sequenceName;
}


STATIC MULTIMAP_API std::string GDALOCIConnector::GetSDO_Rectangle(MBRect range, int srid, int p) {
	return GDALOCIConnector::GetSDO_Rectangle(range.minX,range.minY,range.maxX,range.maxY,srid,p);
}
/**
* Formats a simple 2D SDO_GEOMETRY for a range.
* SDO_GEOMETRY(2003, 2157, NULL, SDO_ELEM_INFO_ARRAY(1, 1003, 3), SDO_ORDINATE_ARRAY(500000, 700000, 560000, 740000))
**/
STATIC MULTIMAP_API std::string GDALOCIConnector::GetSDO_Rectangle(double minX, double minY, double maxX, double maxY, int srid, int p) {
	char buffer[256];
	sprintf(buffer,"SDO_GEOMETRY(2003,%d,NULL,SDO_ELEM_INFO_ARRAY(1,1003,3),SDO_ORDINATE_ARRAY(%.*f,%.*f,%.*f,%.*f))",srid,p,minX,p,minY,p,maxX,p,maxY);
	return std::string(buffer);
}
STATIC MULTIMAP_API std::string GDALOCIConnector::GetSDO_AnyinteractClause(std::string alias, std::string column, MBRect range, int srid, int p) {
	return GDALOCIConnector::GetSDO_AnyinteractClause(alias,column,range.minX,range.minY,range.maxX,range.maxY,srid,p);
}
STATIC MULTIMAP_API std::string GDALOCIConnector::GetSDO_AnyinteractClause(std::string alias, std::string column, double minX, double minY, double maxX, double maxY, int srid, int p) {
	std::string clause;
	clause.append("SDO_ANYINTERACT(");
	clause.append(alias);
	clause.append(".");
	clause.append(column);
	clause.append(",");
	clause.append( GDALOCIConnector::GetSDO_Rectangle(minX,minY,maxX,maxY,srid,p));
	clause.append(")='TRUE'");
	return clause;
}
STATIC MULTIMAP_API bool GDALOCIConnector::WritePRJ(std::string filePath, int epsg) {
	bool status = false;
	OGRSpatialReference SRS;
	int err = SRS.importFromEPSG(epsg);
	if ( err == OGRERR_NONE ) {
		status = GDALOCIConnector::WritePRJ(filePath,&SRS);
	} else {
		const char *message = CPLGetLastErrorMsg();
		printf("%s",message);
	}
	return status;
}
STATIC MULTIMAP_API bool GDALOCIConnector::WritePRJ(std::string filePath, OGRLayer* pLayer) {
	bool status = false;
	if ( pLayer ) {
		OGRSpatialReference* pSRS = pLayer->GetSpatialRef();
		if ( pSRS ) {
			status = GDALOCIConnector::WritePRJ(filePath,pSRS);
		}
	}
	return status;
}
STATIC MULTIMAP_API bool GDALOCIConnector::WritePRJ(std::string filePath, OGRSpatialReference* pSRS) {
	bool status = false;
	if ( pSRS ) {
		char* buffer = NULL;
		int err = pSRS->morphToESRI();
		if ( err == OGRERR_NONE ) {
			err = pSRS->exportToWkt(&buffer);
			if ( err == OGRERR_NONE ) {
				status = GDALOCIConnector::WritePRJ(filePath,(const char*)buffer);
				OGRFree(buffer);
			}
		}
	}
	return status;
}
STATIC MULTIMAP_API bool GDALOCIConnector::WritePRJ(std::string filePath, const char* prj) {
	bool status = false;
	if ( prj ) {
		MFUtils mfUtils;
		std::string prjFilePath;
		mfUtils.Retype(prjFilePath,".prj",filePath);
		FILE* file = fopen(prjFilePath.c_str(),"wt");
		if ( file ) {
			size_t ne = fwrite(prj,strlen(prj),1,file);
			fclose(file);
			status = ne>0?true:false;
		}
	}
	return status;
}

MULTIMAP_API void GDALOCIConnector::CreateDLM(std::string outTableName, double resolution, std::string inTableName, MBRect & range, std::string geomColumn, std::string attrColumns,std::string classField,const int nClasses,int* classes,const char** classNames, bool db, bool local, bool keepOrig) {

	for ( int c=0; c<nClasses; c++ ) {
		CreateDLM(outTableName,classes[c],resolution,inTableName,range,geomColumn,attrColumns,classField,classes[c],std::string(classNames[c]),db,local,keepOrig);
	}
}
MULTIMAP_API void GDALOCIConnector::CreateDLM(std::string outTableName, int outClassNum, double resolution, std::string inTableName, MBRect & range, std::string geomColumn, std::string attrColumns,std::string classField,const int nClasses,int* classes, std::string className, bool db, bool local, bool keepOrig) {
	MFUtils mfUtils;
	std::string alias = "g";

	char buffer[20480];
	std::string sqlString;
	sqlString.clear();
	sqlString.append("select ");
	sqlString.append(attrColumns);
	sqlString.append(",");
	sqlString.append(geomColumn);
	sqlString.append(" from ");
	sqlString.append(inTableName);
	sqlString.append(" ");
	sqlString.append(alias);
	sqlString.append(" ");
	sqlString.append("where ");
	if ( classField.length() > 0 && classes[0] > 0 ) {
		sqlString.append("(");
		for ( int f=0; f<nClasses; f++ ) {
			sprintf(buffer,"%s=%d ",classField.c_str(),classes[f]);
			sqlString.append(buffer);
			if ( f != nClasses-1 ) {
				sqlString.append("OR ");
			}
		}
		sqlString.append(") and ");
	}

	sqlString.append(GDALOCIConnector::GetSDO_AnyinteractClause(alias,geomColumn,range,2157));

	CreateDLM( outTableName, resolution, sqlString, range, geomColumn, attrColumns,classField, outClassNum,className, db, local, keepOrig); 
}
MULTIMAP_API void GDALOCIConnector::CreateDLM(std::string outTableName, int outClassNum, double resolution, std::string inTableName, MBRect & range, std::string geomColumn, std::string attrColumns, std::string classField, int classNum, std::string className, bool db, bool local, bool keepOrig) {

	MFUtils mfUtils;
	std::string alias = "g";

	char buffer[20480];
	std::string sqlString;
	sqlString.clear();
	sqlString.append("select ");
	sqlString.append(attrColumns);
	sqlString.append(",");
	sqlString.append(geomColumn);
	sqlString.append(" from ");
	sqlString.append(inTableName);
	sqlString.append(" ");
	sqlString.append(alias);
	sqlString.append(" ");
	sqlString.append("where ");
	if ( classField.length() > 0 ) {
		sqlString.append(classField);
		if ( classNum >= 0 ) {
			sprintf(buffer,"=%d and ",classNum);
			sqlString.append(buffer);
		} else {
			sqlString.append(" is null and ");
		}
	}

	sqlString.append(GDALOCIConnector::GetSDO_AnyinteractClause(alias,geomColumn,range,srid));

	CreateDLM( outTableName, resolution, sqlString, range, geomColumn, attrColumns,classField, classNum,className, db, local, keepOrig); 
}

MULTIMAP_API void GDALOCIConnector::CreateDLMThinOnly(std::string outTableName, int outClassNum, double resolution, std::string inTableName, MBRect & range, std::string geomColumn, std::string attrColumns,std::string classField,std::vector<int> inClasses) {

	time_t startTime = time(NULL);
	OGRDataSource* pOciDS = GetDataSource();

	std::string alias = "g";

	char buffer[1024];
	double precision = resolution/10.0;

	std::string sql;
	sql.append("insert into ");
	sql.append(outTableName);
	sql.append(" (id,product,");
	sql.append(classField);
	sql.append(",");
	sql.append(geomColumn);
	sql.append(") select ");
	sql.append(sequenceName);
	sql.append(".nextval,");
	sprintf(buffer,"%d,%d",productId,outClassNum);
	sql.append(std::string(buffer));
	sql.append(",SDO_UTIL.SIMPLIFY(");
	sql.append(geomColumn);
	sprintf(buffer,",%.2f,%.4f) from ",resolution,precision);
	sql.append(std::string(buffer));
	sql.append(inTableName);
	sql.append(" ");
	sql.append(alias);
	sql.append(" ");
	sql.append("where ");
	if ( classField.length() > 0 ) {
		sql.append("(");
		for ( int f=0; f<inClasses.size(); f++ ) {
			sprintf(buffer,"%s=%d ",classField.c_str(),inClasses[f]);
			sql.append(buffer);
			if ( f != inClasses.size()-1 ) {
				sql.append("OR ");
			}
		}
		sql.append(") and ");
	}
	std::string anyInteract = GDALOCIConnector::GetSDO_AnyinteractClause(alias,geomColumn,range,srid);
	sql.append(anyInteract);

	logger->Log(INFO,"%s",sql.c_str());
	pOciDS->ExecuteSQL(sql.c_str(),NULL,NULL);
	const char* message=NULL;
	int err = GetLastError(&message);
	if ( err != 0 ) {
		logger->Log(ERR,"%s",message);
	}
	lastRunTime = static_cast<double>(time(NULL) - startTime);
	if ( verbose ) {
		logger->Log(INFO,"DLM render time for %s %.0f seconds",outTableName.c_str(),lastRunTime);
	}
}
MULTIMAP_API void GDALOCIConnector::CreateDLM(std::string outTableName, double resolution, std::string sqlQuery, MBRect & range, std::string geomColumn, std::string attrColumns,std::string classField, int outClass, std::string className, bool db, bool local, bool keepOrig) {

	time_t startTime = time(NULL);
	OGRDataSource* pOciDS = GetDataSource();

	MFUtils mfUtils;
	std::string alias = "g";

	bool VRV = true;
	bool simplifyInPolygonize = false;

	bool shpOriginal = false;
	bool keepRaster = false;
	bool saveToDatabase = false;
	if ( db ) {
		saveToDatabase = true;
	}
	if ( local  ) {
		keepRaster = true;
	}
	if ( keepOrig ) {
		shpOriginal = true;
	}
	OGRRegisterAll();
	GDALRegister_GTiff();
	GDALRegister_MEM();

	GDALImageTools imageTools;
	GDALVectorTools vectorTools;

	//for ( int c=0; c<nClasses; c++ ) {
	char buffer[4096];

	OGRSFDriver *memoryDriver = (OGRSFDriver *)OGRGetDriverByName("Memory");
	OGRSFDriver *pVectorDriver = NULL;
	if ( local ) {
		pVectorDriver = (OGRSFDriver *)OGRGetDriverByName("ESRI Shapefile");
	} else {
		pVectorDriver = memoryDriver;
	}

	GDALDriver* pRasterDriver = NULL;
	if ( keepRaster ) {
		pRasterDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
	} else {
		pRasterDriver = GetGDALDriverManager()->GetDriverByName("MEM");
	}

	logger->Log(INFO,"Executing SQL ");
	//logger->Log(INFO,sqlQuery.c_str());
	OGRLayer* pInputLayer =  pOciDS->ExecuteSQL(sqlQuery.c_str(),NULL,NULL);
	if ( pInputLayer ) {
		int featureCount = pInputLayer->GetFeatureCount();
		if ( featureCount > 0 ) {

			sprintf(buffer,"%s_%s_%d_%d_%dx%d",outTableName.c_str(),className.c_str(),524,724,12,6);
			std::string fileStem = std::string(buffer);	

			OGRFeature* pFeature = pInputLayer->GetFeature(0);
			OGRGeometry* pGeometry = pFeature->GetGeometryRef();
			OGRwkbGeometryType gType =  wkbFlatten(pGeometry->getGeometryType());

			if (shpOriginal){
				char **papszShpOptions = NULL;
				std::string wayLayerShp;
				if ( VRV ) {
					wayLayerShp = workFolder+FILE_SEP+fileStem+"_Orig.shp";
				} else {
					wayLayerShp = workFolder+FILE_SEP+fileStem+".shp";
				}
				mfUtils.RemoveShapeFileSet(wayLayerShp);
				if ( gType == wkbPoint ) {
					CSLSetNameValue( papszShpOptions, "SHPT", "POINT" );
				} else if ( gType == wkbLineString ) {
					CSLSetNameValue( papszShpOptions, "SHPT", "ARC" );
				} else if ( gType == wkbPolygon  ) {
					CSLSetNameValue( papszShpOptions, "SHPT", "POLYGON" );
				}
				OGRDataSource* roDataset = (OGRDataSource*)pVectorDriver->CreateDataSource(wayLayerShp.c_str(),papszShpOptions);
				CSLDestroy(papszShpOptions);
				if ( roDataset) {
					OGRLayer* pCopy = roDataset->CopyLayer(pInputLayer,outTableName.c_str());
					if ( pCopy ) {
						GDALClose(pCopy);
					} else {
						ReportLastError();
					}
				} else {
					ReportLastError();
				}
			}
			if ( VRV ) {
				if ( gType == wkbPoint || gType == wkbLineString) {
					char **papszShpOptions = NULL;
					std::string wayLayerShp;
					if (gType == wkbPoint) {
						CSLSetNameValue( papszShpOptions, "SHPT", "POINT" );
						wayLayerShp = workFolder+FILE_SEP+fileStem+"_PNT.shp";
					} else {
						CSLSetNameValue( papszShpOptions, "SHPT", "ARC" );
						wayLayerShp = workFolder+FILE_SEP+fileStem+"_LS.shp";
					}
					mfUtils.RemoveShapeFileSet(wayLayerShp);
					OGRDataSource* roDataset = (OGRDataSource*)pVectorDriver->CreateDataSource(wayLayerShp.c_str(),papszShpOptions);
					CSLDestroy(papszShpOptions);
					if ( roDataset) {
						OGRLayer* pCopy = roDataset->CopyLayer(pInputLayer,outTableName.c_str());
						if ( pCopy ) {
							GDALClose(pCopy);
						} else {
							ReportLastError();
						}
					} else {
						ReportLastError();
					}
				} else if (gType == wkbLineString ) {
					char **papszShpOptions = NULL;
					std::string wayLayerShp = workFolder+FILE_SEP+fileStem+"_LS.shp";
					mfUtils.RemoveShapeFileSet(wayLayerShp);
					CSLSetNameValue( papszShpOptions, "SHPT", "ARC" );
					OGRDataSource* roDataset = (OGRDataSource*)pVectorDriver->CreateDataSource(wayLayerShp.c_str(),papszShpOptions);
					if ( roDataset) {
						vectorTools;
						OGRLayer* simpleLayer = vectorTools.Simplify(roDataset,outTableName.c_str(),pInputLayer,resolution,true);
						if ( simpleLayer ) {
							GDALClose(simpleLayer);
						} else {
							ReportLastError();
						}
					} else {
						ReportLastError();
					}
				} else { // It's a POLYGON
					logger->Log(INFO,"%s count=%d",className.c_str(),featureCount);
					OGRFeature* pFeature = pInputLayer->GetFeature(0);
					OGRGeometry* pGeometry = pFeature->GetGeometryRef();
					char* wkt;
					OGRSpatialReference SRS;
					SRS.importFromEPSG(2157);
					SRS.exportToWkt(&wkt);

					double rasterizeResolution = resolution/2.0;

					// The raster range is the smaller of the layer envelope and the area of interest range
					OGREnvelope envelope;
					pInputLayer->GetExtent(&envelope);
					MBRect layerRange, rasterRange;
					setMBR(layerRange,envelope);
					bool intersects = range.Intersect(rasterRange,layerRange);
					double rrarea = rasterRange.Area();
#ifdef _DEBUG
					double rarea = range.Area();
					double larea = layerRange.Area();

#endif
					int xSize = static_cast<int>(ceil(rasterRange.XRange())/rasterizeResolution);
					int ySize = static_cast<int>(ceil(rasterRange.YRange())/rasterizeResolution);
					double geotransform[6];
					geotransform[0] = rasterRange.minX - (rasterizeResolution/2.0);
					geotransform[1] = rasterizeResolution;
					geotransform[2] = 0.0;
					geotransform[3] = rasterRange.maxY + (rasterizeResolution/2.0);
					geotransform[4] = 0.0;
					geotransform[5] = -rasterizeResolution;

					std::string wayGTiff = workFolder+FILE_SEP+fileStem+".tif";
					remove(wayGTiff.c_str());
					GDALDataset* pRasterDataset = pRasterDriver->Create(wayGTiff.c_str(),xSize,ySize,1,GDT_Byte,NULL);
					if ( pRasterDataset ) {
						pRasterDataset->SetProjection((const char*)wkt);
						pRasterDataset->SetGeoTransform(geotransform);
						logger->Log(INFO,"Rasterizing to %.1f meter over area of %.1f sm grid size=%d",rasterizeResolution,rrarea,xSize*ySize);
						GDALDataset* pds = imageTools.Rasterize2(pInputLayer,rasterizeResolution,&range,true,pRasterDataset);
						if ( pds ) {
							if ( fill ) {
								ByteBuffer filler;
								filler.ReadBuffer(pds);
								filler.setVerbose(verbose);
								filler.setMaxPasses(28);
								unsigned int maxHoleSize = static_cast<unsigned int>((resolution/rasterizeResolution)*8.0);
								logger->Log(INFO,"Filling holes smaller than %.1f sm",(double)maxHoleSize*rasterizeResolution);
								filler.Fill(maxHoleSize);
								filler.SaveBuffer(pds);
							}
							if ( erode ) {
								ByteBuffer eroder;
								eroder.setVerbose(verbose);
								eroder.ReadBuffer(pds);
								eroder.setMaxPasses(66);
								unsigned int minSpineSize = static_cast<unsigned int>(resolution/rasterizeResolution)/2;
								logger->Log(INFO,"Eroding spines");
								eroder.Erode(6);
								eroder.SaveBuffer(pds);
							}

							GDALRasterBand* rastBand = pRasterDataset->GetRasterBand(1);
							OGRLayer* polyLayer = NULL;
							OGRDataSource* pMemoryDS = (OGRDataSource*)memoryDriver->CreateDataSource("memory",NULL);
							if (pMemoryDS) {
								OGRLayer* pVector = pMemoryDS->CreateLayer("GEOMETRY",NULL,wkbPolygon,NULL);
								if ( pVector ) {
									OGRFieldDefn oField( "VALUE", OFTReal );
									logger->Log(INFO,"Polygonizing");
									OGRLayer* polyLayer = vectorTools.Polygonize(rastBand,pVector, simplifyInPolygonize);
									int polyFeatureCount = polyLayer->GetFeatureCount();
									logger->Log(INFO,"%s post polygonization=%d",className.c_str(),polyFeatureCount);

									char **papszShpOptions = NULL;
									std::string wayLayerShp = workFolder+FILE_SEP+fileStem+".shp";
									mfUtils.RemoveShapeFileSet(wayLayerShp);
									CSLSetNameValue( papszShpOptions, "SHPT", "POLYGON" );
									OGRDataSource* pVectorDataSource = (OGRDataSource*)pVectorDriver->CreateDataSource(wayLayerShp.c_str(),papszShpOptions);
									CSLDestroy(papszShpOptions);

									OGRLayer* simpleLayer = NULL;
									if ( simplifyInPolygonize ) {
										simpleLayer = pVectorDataSource->CopyLayer(polyLayer,outTableName.c_str());										                                                   logger->Log(INFO, "Polygonizing complete");
									} else {
										logger->Log(INFO, "Polygonizing complete, Simplifying");
										simpleLayer = vectorTools.Simplify(pVectorDataSource,"simple",polyLayer,resolution,true);
									}
									if ( simpleLayer ) {
										logger->Log(INFO,"Simplify reduced %d points to %d", vectorTools.inPointCount, vectorTools.outPointCount);
										if ( db ) {
											int newRows = simpleLayer->GetFeatureCount();
											Ogr2ogr ogr2ogr;

											std::string workTableName = outTableName;
											mfUtils.UUID(workTableName);
											workTableName = "TMP_"+workTableName.substr(26);
											logger->Log(INFO,"Loading database table %s with %d rows marked product %d",workTableName.c_str(),newRows,productId);
											int err = ogr2ogr.Run(workTableName, dim, srid, geomColumn, attrColumns, false, this, pVectorDataSource);
											GDALClose(pVectorDataSource);
											pVectorDataSource = NULL;
											if ( err == 0 ) {
												std::string sql;
												sql.append("insert into ");
												sql.append(outTableName);
												sql.append(" (id,product,");
												sql.append(classField);
												sql.append(",");
												sql.append(geomColumn);
												sql.append(") select ");
												sql.append(sequenceName);
												sql.append(".nextval,");
												sprintf(buffer,"%d,%d",productId,outClass);
												sql.append(std::string(buffer));
												sql.append(",");
												sql.append(geomColumn);
												sql.append(" from ");
												sql.append(workTableName);
												logger->Log(INFO,"%s",sql.c_str());
												pOciDS->ExecuteSQL(sql.c_str(),NULL,NULL);
												sql.clear();
												sql.append("drop table ");
												sql.append(workTableName);
												sql.append(" purge");
												logger->Log(INFO,"%s",sql.c_str());
												pOciDS->ExecuteSQL(sql.c_str(),NULL,NULL);
												logger->Log(INFO,"Database table %s loa",outTableName.c_str());
											} else {
												ReportLastError();
											}
										}
									} else {
										ReportLastError();
									}
									if ( pVectorDataSource)
										GDALClose(pVectorDataSource);
								} else {
									ReportLastError();
								}
								if ( pMemoryDS ) 
									GDALClose(pMemoryDS);
							} else {
								ReportLastError();
							}
							if ( pRasterDataset )
								GDALClose(pRasterDataset);
							pRasterDataset = NULL;
						} else {
							ReportLastError();
						}
						if ( wkt ) 
							OGRFree(wkt);
					} else {
						ReportLastError();
					}
				}
			}
		}
		if ( pInputLayer ) 
			GDALClose(pInputLayer);
		pInputLayer = NULL;
	} else {
		ReportLastError();
	}
	lastRunTime = static_cast<double>(time(NULL) - startTime);
	if ( verbose ) {
		logger->Log(INFO,"DLM render time for %s %.0f seconds",outTableName.c_str(),lastRunTime);
	}

}
PRIVATE void GDALOCIConnector::ReportLastError() {	
	const char* message=NULL;
	GetLastError(&message);
	logger->Log(ERR,"%s",message);
}