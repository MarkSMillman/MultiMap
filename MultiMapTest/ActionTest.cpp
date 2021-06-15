#include "MultiMap.h"
#include "MultiMapTest.h"
#include "MFUtils.h"
#include "ActionProcessor.h"
#include "GDALOCIConnector.h"
#include "Ogr2ogr.h"
#include "ThreadTools.h"
#include "MapTextAPI.h"

#include "gdal.h"
#include "ogr_api.h"
#include "ogr_srs_api.h"
#include "ogr_spatialref.h"
#include "ogrsf_frmts.h"
#include "cpl_string.h"

//#include "ogr_oci.h"
#include "cpl_conv.h"
#include "cpl_error.h"

#include <boost/test/unit_test.hpp>
/**
3, 6, & 15 metres reductions centered about or containing 

1.	X 526145, Y 734525
2.	X 530121, Y 726277
3.	X 538404, Y 727619
4.	X 549352, Y 727307 
5.	X 504476, Y 737575
6.	X 529616, Y 724998.

**/
BOOST_AUTO_TEST_CASE(ActionTester)
{
	MFUtils mfUtils;
	std::string testFolder = MultiMapTest::TestFolder("Test");
	std::string actionsFile = testFolder + "\\actions.json";

	BOOST_REQUIRE(mfUtils.FileExists(actionsFile)==0);

	ActionProcessor processor;
	bool result = processor.Run(actionsFile);
}

BOOST_AUTO_TEST_CASE(ForkTester)
{
	MFUtils mfUtils;
	std::string testFolder = MultiMapTest::TestFolder("Test");
	std::string actionsFile = testFolder + "\\actions.json";

	BOOST_REQUIRE(mfUtils.FileExists(actionsFile)==0);

	std::string command;
	command.append("ogr2ogr");
	command.append(" -skipfailures");
	//command.append(" -append");
	//command.append(" -update");
	command.append(" -preserve_fid");
	command.append(" -f OCI");
	command.append(" -dim 2");
	command.append(" -select GUID,FORM_ID,FUNC_ID");
	command.append(" -geomfield POLY_GEOM_VALUE");
	//command.append(" -lco TRUNCATE=YES");
	command.append(" -lco INDEX=YES");
	command.append(" -lco SRID=2157");
	command.append(" -lco GEOMETRY_NAME=POLY_GEOM_VALUE");
	command.append(" -lco DIMINFO_X=400000,800000,0.0005");
	command.append(" -lco DIMINFO_Y=500000,1000000,0.0005");
	command.append(" -lco DIMINFO_Z=-100000,100000,0.0005");
	command.append(" -nln DLM12_WAY"); // new table name
	command.append(" OCI:prime2_enhance/prime2_enhance@domain13:DLM_WAY_ONE"); // dst_datasource
	//command.append(" OCI:prime2_enhance/prime2_enhance@domain13:DLM_WAY_ONE"); // src_datasource
	command.append(" H:\\Source\\SVN-CPP\\MultiMap\\SampleData\\Test\\RastOraLayer.shp");

	ThreadTools threadFactory;
	int errCode = threadFactory.RunAndWait(command.c_str());
	BOOST_CHECK(errCode==0);

	std::vector<std::string> response = threadFactory.GetResponse();
	for ( std::string line : response ) {
		MultiMapTest::Message("%s",line.c_str());
	}
	/**
	errCode = threadFactory.RunAndWait("dir C:\\Temp\\");
	BOOST_CHECK(errCode==0);

	response = threadFactory.GetResponse();
	for ( std::string line : response ) {
	MultiMapTest::Message("%s",line.c_str());
	}
	threadFactory.ClearResponse();
	BOOST_CHECK(threadFactory.GetResponse().size()==0);
	**/
}

BOOST_AUTO_TEST_CASE(Ogr2ogrTester)
{
	MFUtils mfUtils;
	std::string testFolder = MultiMapTest::TestFolder("Test");
	std::string actionsFile = testFolder + "\\actions.json";

	BOOST_REQUIRE(mfUtils.FileExists(actionsFile)==0);

	std::vector<std::string> args;
	args.push_back("ogr2ogr");
	args.push_back("-skipfailures");
	args.push_back("-preserve_fid");
	args.push_back("-append");
	args.push_back("-f");
	args.push_back("OCI");
	args.push_back("-dim");
	args.push_back("2");
	args.push_back("-select");
	args.push_back("GUID,FORM_ID,FUNC_ID");
	args.push_back("-geomfield");
	args.push_back("POLY_GEOM_VALUE");
	args.push_back("-lco");
	args.push_back("INDEX=YES");
	args.push_back("-lco");
	args.push_back("SRID=2157");
	args.push_back("-lco");
	args.push_back("GEOMETRY_NAME=POLY_GEOM_VALUE");
	args.push_back("-lco");
	args.push_back("DIMINFO_X=400000,800000,0.0005");
	args.push_back("-lco");
	args.push_back("DIMINFO_Y=500000,1000000,0.0005");
	args.push_back("-nln"); // new table name
	args.push_back("DLM12_WAY"); // new table name
	args.push_back("OCI:mm_dlm/mm_dlm@domain13:DLM_0_WAY"); // dst_datasource
	//args.push_back("C:\\Users\\mark.millman\\Source\\SVN-CPP\\MultiMap\\SampleData\\Test\\_SimlifiedPostPolygonize.shp");

	OGRRegisterAll();

	OGRDataSource* pSrcDS = NULL;
	const char *pszDriverName = "ESRI Shapefile";
	OGRSFDriver *shpDriver = (OGRSFDriver *)OGRGetDriverByName(pszDriverName);
	if ( shpDriver ) {
		pSrcDS = (OGRDataSource* )shpDriver->Open("C:\\Users\\mark.millman\\Source\\SVN-CPP\\MultiMap\\SampleData\\Test\\_SimlifiedPostPolygonize.shp",FALSE);
		if ( !pSrcDS ) {
			const char* message = CPLGetLastErrorMsg();
			if ( message ) {
				MultiMapTest::Message("%s",message);
			}
		}
	} else {
		const char* message = CPLGetLastErrorMsg();
		if ( message ) {
			MultiMapTest::Message("%s",message);
		}
	}

	GDALOCIConnector connector;
	connector.setUsername("mm_dlm");
	connector.setPassword("mm_dlm");
	connector.setTnsname("domain13");
	connector.setTableName("dlm_0_way");
	connector.setDiminfoX(400000.0,  800000.0, .0005);
	connector.setDiminfoY(500000.0, 1000000.0, .0005);
	connector.setDim(2);

	Ogr2ogr ogr2ogr;
	std::string newTableName = "dlm12_way";
	int dim = 2;
	int srid=2157;
	std::string geomField = "POLY_GEOM_VALUE";
	std::string select = "GUID,FORM_ID,FUNC_ID";
	bool index = true;
	int errCode = connector.CreateLayer(newTableName, dim, srid, geomField, select, index, pSrcDS);
	//int errCode = ogr2ogr.Run(newTableName, dim, srid, geomField, select, index, &connector, pSrcDS);
	//int errCode = ogr2ogr.Run(args,pSrcDS);
	BOOST_CHECK(errCode==0);
	const char* message = CPLGetLastErrorMsg();
	if ( message ) {
		MultiMapTest::Message("%s",message);
	}

	OGRDataSource::DestroyDataSource(pSrcDS);
}

BOOST_AUTO_TEST_CASE(MapTextTester) {

	std::string metadataFolder = "C:\\Label-EZ\\Examples\\ArcView\\SanFrancisco";
	std::string specFile = metadataFolder+"\\SanFrancisco.spe";
	std::string configFile = metadataFolder+"\\SanFrancisco.mcf";
	std::string dataFolder = "C:\\Label-EZ\\Examples\\ArcView\\SanFrancisco";

	MapTextAPI mtAPI;
	mtAPI.RunArcView(configFile,specFile,dataFolder);
}