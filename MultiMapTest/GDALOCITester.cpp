#pragma warning ( disable : 4251 )
#include "MultiMap.h"
#include "MultiMapTest.h"
#include "MFUtils.h"
#include "GDALOCIConnector.h"
#include "GDALImageTools.h"
#include "GDALVectorTools.h"
#include "ByteBuffer.h"
#include "Logger.h"
#include "POTrace.h"

#include "gdal.h"
#include "gdal_priv.h"
#include "ogr_api.h"
#include "ogr_srs_api.h"
#include "ogr_spatialref.h"
#include "ogrsf_frmts.h"
#include "cpl_string.h"

#include "ogr_oci.h"
#include "cpl_conv.h"

#include <boost/test/unit_test.hpp>

static int LOGGER_LEVEL = ALWAYS;
static std::string workFolder = "C:\\MultiMap\\WorkFolder\\100002\\";
static double resolution = 1.0;
static std::string outTableStem = "dlm_1_";

void ReportLastError(GDALOCIConnector & connector) {	
	const char* message=NULL;
	connector.GetLastError(&message);
	MultiMapTest::Message("%s",message);
}

void RunDefault(std::string _outTableName, int outClass, std::string _inTableName, double resolution, std::string _geomColumn, std::string _attrColumns="") {
	MFUtils mfUtils;
	std::string testFolder = MultiMapTest::TestFolder("Test");

	GDALOCIConnector connector;
	connector.setLoggerLevel(LOGGER_LEVEL);
	connector.setWorkFolder(workFolder);
	OGRDataSource* pDS = connector.Connect();
	BOOST_REQUIRE(pDS);

	MBRect range(524000.0,724000.0,536000.0,730000.0);
	std::string inTableName = _inTableName;
	std::string outTableName = _outTableName;
	std::string alias = "g";
	std::string geomColumn = _geomColumn;
	std::string attrColumns = "ID,PRODUCT,SCALE,FORM_ID,FUNC_ID,STATUS,PRIMARY_NAME,SECONDARY_NAME,Z_ORDER,GUID";
	if ( _attrColumns.length() > 0 ) {
		attrColumns = _attrColumns;
	}
	std::string sqlString;
	std::string classField = "FUNC_ID";
	const int nClasses = 1;
	int classes[1] = {-1};
	const char* classNames[1] = {""};

	connector.CreateDLM(outTableName, resolution, inTableName, range, geomColumn, attrColumns, classField, nClasses, classes, classNames);
}

void RunWaterTester() {
	MFUtils mfUtils;
	std::string testFolder = MultiMapTest::TestFolder("Test");

	GDALOCIConnector connector;
	connector.setLoggerLevel(LOGGER_LEVEL);
	connector.setWorkFolder(workFolder);
	OGRDataSource* pDS = connector.Connect();
	BOOST_REQUIRE(pDS);

	MBRect range(524000.0,724000.0,536000.0,730000.0);
	std::string inTableName = "dlm_0_water";
	std::string outTableName = outTableStem+"water";
	std::string alias = "g";
	std::string geomColumn = "POLY_GEOM_VALUE";
	std::string attrColumns = "ID,PRODUCT,SCALE,FORM_ID,FUNC_ID,STATUS,PRIMARY_NAME,SECONDARY_NAME,Z_ORDER,GUID";
	std::string sqlString;
	std::string classField = "form_id";
	const int nClasses = 6;
	int classes[nClasses] = {67,207,321,324,390,438};
	const char* classNames[nClasses] = {
		"Canal",
		"Lake",
		"ReservoirOpen",
		"River",
		"Stream",
		"Waterfall"
	};	

	connector.CreateDLM(outTableName,resolution,inTableName,range,geomColumn,attrColumns,classField,nClasses,classes,classNames);
	//CreateShapeFiles(connector,resolution,range,testFolder,tableName,v,alias,geomColumn,attrColumns,classField,nClasses,classes,classNames);
}
void RunStructureTester() {

	MFUtils mfUtils;

	GDALOCIConnector connector;
	connector.setLoggerLevel(LOGGER_LEVEL);
	connector.setWorkFolder(workFolder);
	OGRDataSource* pDS = connector.Connect();
	BOOST_REQUIRE(pDS);

	MBRect range(524000.0,724000.0,536000.0,730000.0);
	std::string inTableName = "dlm_0_structure";
	std::string outTableName = outTableStem+"structure";
	std::string alias = "g";
	std::string geomColumn = "GEOM_VALUE";
	std::string attrColumns = "ID,PRODUCT,SCALE,FORM_ID,FUNC_ID,STATUS,PRIMARY_NAME,SECONDARY_NAME,Z_ORDER,GUID,WAY_GUID";
	std::string sqlString;
	std::string classField = "g.geom_value.sdo_gtype";
	const int nClasses = 3;
	int classes[nClasses] = {2001,2002,2003};
	const char* classNames[nClasses] = {
		"Point",
		"Line",
		"Poly"
	};	

	connector.CreateDLM(outTableName,resolution,inTableName,range,geomColumn,attrColumns,classField,nClasses,classes,classNames);
}
void RunDefaultsArtificial() {

	GDALOCIConnector connector;
	connector.setLoggerLevel(LOGGER_LEVEL);
	connector.setWorkFolder(workFolder);

	connector.setSequenceName("DLM_3_ARTIFICIAL_SEQ");
	connector.setProductId(33);
	OGRDataSource* pDS = connector.Connect();
	BOOST_REQUIRE(pDS);

	bool db=true;
	bool local=true;
	bool keepOrig=true;

	MBRect range(524000.0,724000.0,536000.0,730000.0);
	std::string inTableName = "dlm_0_artificial";
	std::string outTableName = outTableStem+"artificial";
	std::string geomColumn = "POLY_GEOM_VALUE";
	std::string attrColumns = "ID,PRODUCT,SCALE,FORM_ID,FUNC_ID,STATUS,PRIMARY_NAME,SECONDARY_NAME";
	std::string sqlString;
	std::string classField = "FUNC_ID";

	//ARTIFICIAL APRON             FUNC_ID  537
	//ARTIFICIAL CONCRETE          FUNC_ID  581,547
	//ARTIFICIAL GREENERY          FUNC_ID  551,207
	//ARTIFICIAL MANMADE           FUNC_ID  538,544,576
	//ARTIFICIAL PARKING           FUNC_ID  545
	//ARTIFICIAL RAIL              FUNC_ID  359,567
	//ARTIFICIAL RAILBED           FUNC_ID  566
	//ARTIFICIAL TRAFFIC_AREA      FUNC_ID  162,586,573,577

	{
		const int nClasses = 1;
		int classes[nClasses] = {57};
		int outClassNum = classes[0];
		std::string className = "Apron";
		connector.CreateDLM(outTableName, outClassNum, resolution, inTableName, range, geomColumn, attrColumns,  classField, nClasses, classes, className, db, local, keepOrig);
		BOOST_TEST_CHECKPOINT("APRON DONE");
	}
	{
		const int nClasses = 2;
		int classes[nClasses] = {581,547};
		int outClassNum = classes[0];
		std::string className = "Concrete";
		connector.CreateDLM(outTableName, outClassNum, resolution, inTableName, range, geomColumn, attrColumns,  classField, nClasses, classes, className, db, local, keepOrig);
		BOOST_TEST_CHECKPOINT("CONCRETE DONE");
	}
	{
		const int nClasses = 4;
		int classes[nClasses] = {162,586,573,577};
		int outClassNum = classes[0];
		std::string className = "TrafficArea";
		connector.CreateDLM(outTableName, outClassNum, resolution, inTableName, range, geomColumn, attrColumns,  classField, nClasses, classes, className, db, local, keepOrig);
		BOOST_TEST_CHECKPOINT("TRAFFIC DONE");
	}
	{
		const int nClasses = 3;
		int classes[nClasses] = {538,544,576};
		int outClassNum = classes[0];
		std::string className = "ManMade";
		connector.CreateDLM(outTableName, outClassNum, resolution, inTableName, range, geomColumn, attrColumns,  classField, nClasses, classes, className, db, local, keepOrig);
		BOOST_TEST_CHECKPOINT("MANMADE DONE");
	}

	{
		const int nClasses = 1;
		int classes[nClasses] = {545};
		int outClassNum = classes[0];
		std::string className = "Parking";
		connector.CreateDLM(outTableName, outClassNum, resolution, inTableName, range, geomColumn, attrColumns,  classField, nClasses, classes, className, db, local, keepOrig);
		BOOST_TEST_CHECKPOINT("PARKING DONE");
	}
	{
		const int nClasses = 2;
		int classes[nClasses] = {359,567};
		int outClassNum = classes[0];
		std::string className = "Rail";
		connector.CreateDLM(outTableName, outClassNum, resolution, inTableName, range, geomColumn, attrColumns,  classField, nClasses, classes, className, db, local, keepOrig);
		BOOST_TEST_CHECKPOINT("RAIL DONE");
	}
	{
		const int nClasses = 1;
		int classes[nClasses] = {566};
		int outClassNum = classes[0];
		std::string className = "Railbed";
		connector.CreateDLM(outTableName, outClassNum, resolution, inTableName, range, geomColumn, attrColumns,  classField, nClasses, classes, className, db, local, keepOrig);
		BOOST_TEST_CHECKPOINT("RAILBED DONE");
	}
	{
		const int nClasses = 2;
		int classes[nClasses] = {551,207};
		int outClassNum = classes[0];
		std::string className = "Greenery";
		connector.CreateDLM(outTableName, outClassNum, resolution, inTableName, range, geomColumn, attrColumns,  classField, nClasses, classes, className, db, local, keepOrig);
		BOOST_TEST_CHECKPOINT("GREENERY DONE");
	}
}
void RunDefaultsTester() {
	RunDefault("dlm_1_building",0,"dlm_0_building",1.0,"POLY_GEOM_VALUE","poly");
	RunDefault("dlm_1_building",0,"dlm_0_building",1.0,"PNT_GEOM_VALUE","pnt");
	RunDefault("dlm_1_exposed",0,"dlm_0_exposed",2.0,"POLY_GEOM_VALUE");
	RunDefault("dlm_1_locale",0,"dlm_0_locale",2.0,"POLY_GEOM_VALUE","poly");
	RunDefault("dlm_1_locale",0,"dlm_0_locale",2.0,"PNT_GEOM_VALUE","pnt");
	RunDefault("dlm_1_rail_segment",0, "dlm_0_rail_segment",1.0, "LINE_GEOM_VALUE", "ID,PRODUCT,SCALE,FORM_ID,FUNC_ID,STATUS,RAIL_TYPE,Z_ORDER,GUID");
	RunDefault("dlm_1_site",0,"dlm_0_site",1.0,"POLY_GEOM_VALUE");
	RunDefault("dlm_1_vegetation",0,"dlm_0_vegetation",2.0,"POLY_GEOM_VALUE");
}

void RunWayLineTester() {
	MFUtils mfUtils;

	GDALOCIConnector connector;
	connector.setLoggerLevel(LOGGER_LEVEL);
	connector.setWorkFolder(workFolder);
	OGRDataSource* pWayDS = connector.Connect();
	BOOST_CHECK(pWayDS);
	if ( pWayDS ) {
		MBRect range50K(524000.0,724000.0,536000.0,730000.0);
		char buffer[4096];
		sprintf(buffer,"DLM_0_WAY_LINE_%d_%d_%dx%d",524,724,12,6);
		std::string fileStem = std::string(buffer);
		std::string testFolder = MultiMapTest::TestFolder("Test");
		std::string wayLayerShp = testFolder+FILE_SEP+fileStem+".shp";

		mfUtils.RemoveShapeFileSet(wayLayerShp);
		OGRSFDriver* pOciDriver = pWayDS->GetDriver();
		BOOST_CHECK(pOciDriver);

		const char* driverName = pOciDriver->GetName();
		BOOST_CHECK_EQUAL(driverName,"OCI");
		GDALDriver* pDriver = (GDALDriver*)pOciDriver;	
		char **papszMetadata = pDriver->GetMetadata();

		std::string tableName = "dlm_0_way";
		std::string alias = "g";
		std::string columnName = "LINE_GEOM_VALUE";
		std::string sqlString;
		sqlString.append("select ID,PRODUCT,SCALE,FORM_ID,FUNC_ID,STATUS,WAY_TYPE,PRIMARY_NAME,SECONDARY_NAME,ROUTE_NUM,Z_ORDER,GUID,LENGTH,LINE_GEOM_VALUE ");
		sqlString.append("from ");
		sqlString.append(tableName);
		sqlString.append(" ");
		sqlString.append(alias);
		sqlString.append(" ");
		sqlString.append("where (primary_name is not null or route_num is not null) and ");
		sqlString.append(GDALOCIConnector::GetSDO_AnyinteractClause(alias,columnName,range50K,2157));
		const char* sql = sqlString.c_str();
		MultiMapTest::Message("Executing SQL");
		MultiMapTest::Message(sql);
		OGRLayer* pWayLayer = pWayDS->ExecuteSQL(sql,NULL,NULL);
		BOOST_REQUIRE(pWayLayer);

		const char *pszDriverName = "ESRI Shapefile";
		OGRSFDriver *pVectorDriver = (OGRSFDriver *)OGRGetDriverByName(pszDriverName);
		OGRSFDriver *memoryDriver = (OGRSFDriver *)OGRGetDriverByName("Memory");
		char **papszShpOptions = NULL;
		CSLSetNameValue( papszShpOptions, "SHPT", "POLYGON" );
		OGRDataSource* roDataset = (OGRDataSource*)pVectorDriver->CreateDataSource(wayLayerShp.c_str(),papszShpOptions);
		CSLDestroy(papszShpOptions);
		MultiMapTest::Message("Writing %s",wayLayerShp.c_str());
		OGRLayer* pCopy = roDataset->CopyLayer(pWayLayer,"DLM_0_WAY_LINE");
		GDALOCIConnector::WritePRJ(wayLayerShp,2157);
		GDALClose(pCopy);

		GDALClose(pWayLayer);
		pWayLayer = NULL;
	}
}
void RunWayTester() {
	MFUtils mfUtils;

	GDALOCIConnector connector;
	connector.setWorkFolder(workFolder);
	connector.setLoggerLevel(LOGGER_LEVEL);
	OGRDataSource* pDS = connector.Connect();
	if ( pDS ) {
		std::string testFolder = MultiMapTest::TestFolder("Test");
		MBRect range(524000.0,724000.0,536000.0,730000.0);
		const int nClasses=9;
		int classes[nClasses] = {179,409,475,191,177,419,413,266,-1};
		const char* classNames[nClasses] = {
			"1stClass",
			"2ndClass",
			"3rdClass",
			"4thClass",
			"5thClass",
			"6thClass",
			"7thClass",
			"MainRoad",
			"Other"
		};					
		std::string inTableName = "dlm_0_way";
		std::string outTableName = outTableStem+"way";
		std::string alias = "g";
		std::string geomColumn = "POLY_GEOM_VALUE";
		std::string attrColumns = "ID,PRODUCT,SCALE,FORM_ID,FUNC_ID,STATUS,WAY_TYPE,PRIMARY_NAME,SECONDARY_NAME,ROUTE_NUM,Z_ORDER,GUID,LENGTH";
		std::string sqlString;
		std::string classField = "func_id";

		connector.CreateDLM(outTableName, resolution, inTableName, range, geomColumn, attrColumns, classField, nClasses, classes, classNames, true, true, false);
	}
}
BOOST_AUTO_TEST_CASE(VRVTester) {
	RunDefaultsArtificial();

	/*
	RunWayTester();
	RunWaterTester();
	RunDefaultsTester();
	RunStructureTester();
	RunWayLineTester();
	*/
}