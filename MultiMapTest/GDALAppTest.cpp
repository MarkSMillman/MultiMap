#pragma warning ( disable : 4251 )
#include "MultiMap.h"
#include "MultiMapTest.h"
#include "MFUtils.h"
#ifdef USE_OCI
#include "GDALOCIConnector.h"
#endif
#include "GDALDem.h"
#include "GDALImageTools.h"
#include "ColorTable.h"
#include "OGRStyle.h"
#include "GDALGrid_.h"

#include "gdal.h"
#include "gdal_priv.h"
#include "ogr_api.h"
#include "ogr_srs_api.h"
#include "ogr_spatialref.h"
#include "ogrsf_frmts.h"
#include "cpl_string.h"

#include <time.h>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(GdalHillshadeTester)
{
	MFUtils mfUtils;

	std::string colorTableFolder = MultiMapTest::TestFolder("color");
	std::string colorTableFile = colorTableFolder + "\\Ireland.ctl";

	BOOST_CHECK(mfUtils.FileExists(colorTableFile)==0);

	std::string folder = MultiMapTest::TestFolder("test");
	std::string demFile = folder+FILE_SEP+"N53W010.tif";
	BOOST_REQUIRE(mfUtils.FileExists(demFile)==0);

	std::string crFile = folder+FILE_SEP+"CR_N53W010CR.tif";
	GDALAllRegister();
	GDALDatasetH hSrcDS = GDALOpen(demFile.c_str(),GA_ReadOnly);
	BOOST_REQUIRE(hSrcDS);
	if ( hSrcDS ) {
		GDALDataset* pSrcDS = (GDALDataset*)hSrcDS;
		GDALDriver* pDriver = pSrcDS->GetDriver();
		GDALDatasetH hDstCrDS = NULL;
		GDALDatasetH hDstHsDS = NULL;
		{
			GDALDem gdalDem(COLOR_RELIEF);
			gdalDem.bZevenbergenThorne = TRUE;
			gdalDem.bCombined = TRUE;
			gdalDem.CreateColorRelief(&hDstCrDS,crFile,pDriver,hSrcDS,colorTableFile);
			BOOST_CHECK(hDstCrDS);
			BOOST_CHECK(mfUtils.FileExists(crFile)==0);

		}
		{
			std::string hsFile = folder+FILE_SEP+"HS_N53W010CR.tif";
			GDALDem gdalDemHS(HILL_SHADE);
			gdalDemHS.bZevenbergenThorne = TRUE;
			gdalDemHS.bCombined = TRUE;
			gdalDemHS.CreateHillshade(&hDstHsDS,hsFile,pDriver,hSrcDS);
			BOOST_CHECK(mfUtils.FileExists(hsFile)==0);
			BOOST_CHECK(hDstHsDS);
		}
		std::string blFile = folder+FILE_SEP+"BL_N53W010CR.tif";
		GDALImageTools imageTools;
		GDALDataset* pMergedDS;
		imageTools.MergeHillshade(&pMergedDS,blFile.c_str(),(GDALDataset*)hDstCrDS,(GDALDataset*)hDstHsDS,0.25f);
		GDALClose(hDstCrDS);
		GDALClose(hDstHsDS);
		GDALClose(hSrcDS);
	}
}

BOOST_AUTO_TEST_CASE(ogr_style) {
	const char* szStyle = "PEN(c:#FF0000,w:5px)";

	OGRPen* ogrPen = (OGRPen*)OGRStyle::Instance(szStyle);
	BOOST_CHECK_EQUAL(ogrPen->color,0xFF0000);
	BOOST_CHECK_EQUAL(ogrPen->width,5.0);

	BOOST_TEST_MESSAGE("ogr_style completed");
}

BOOST_AUTO_TEST_CASE(grid) {

	time_t startTime;
	time(&startTime);

	GDALGrid gdalGrid;
	std::vector<std::string> args;
	args.push_back(std::string(""));
	args.push_back(std::string("-a"));
	args.push_back(std::string("invdist:radius1=10:radius2=10"));
	args.push_back(std::string("-of"));
	args.push_back(std::string("GTiff"));
	args.push_back(std::string("-ot"));
	args.push_back(std::string("Float32"));
	args.push_back(std::string("-txe"));
	args.push_back(std::string("550500"));
	args.push_back(std::string("551200"));
	args.push_back(std::string("-tye"));
	args.push_back(std::string("673800"));
	args.push_back(std::string("674300"));
	args.push_back(std::string("-outsize"));
	args.push_back(std::string("700"));
	args.push_back(std::string("500"));
	args.push_back(std::string("-a_srs"));
	args.push_back(std::string("EPSG:2157"));
	//args.push_back(std::string("-z_multiply"));
	//args.push_back(std::string("10"));
	args.push_back(std::string("-sql"));
	args.push_back(std::string("select geom from sample2 where val_d1>=550500 and val_d1<=551200 and val_d2>=673800 and val_d2<674300"));
	args.push_back(std::string("oci:elevate/elevate@domain15"));
	args.push_back(std::string("C:\\TEMP\\Laplace2T.tif")); 
	int result = gdalGrid.Run(args);  
	BOOST_CHECK(result == 0);

	time_t stopTime;
	time(&stopTime);
	MultiMapTest::ReportRunTime("GRID runtime ", static_cast<double>(stopTime - startTime));
}

BOOST_AUTO_TEST_CASE(wgrid) {

	time_t startTime;
	time(&startTime);

	GDALGrid gdalGrid;
	//gdalGrid.useLaplace = true;
	std::vector<std::string> args;
	args.push_back(std::string(""));
	args.push_back(std::string("-a"));
	//args.push_back(std::string("invdist:radius1=10:radius2=10"));
	args.push_back(std::string("invdist:radius1=200:radius2=200"));
	args.push_back(std::string("-of"));
	args.push_back(std::string("GTiff"));
	args.push_back(std::string("-ot"));
	args.push_back(std::string("Float32"));
	args.push_back(std::string("-txe"));
	args.push_back(std::string("1383075"));
	args.push_back(std::string("1385315"));
	args.push_back(std::string("-tye"));
	args.push_back(std::string("517800"));
	args.push_back(std::string("522200"));
	args.push_back(std::string("-outsize"));
	//args.push_back(std::string("2240"));
	//args.push_back(std::string("4400"));
	args.push_back(std::string("1120"));
	args.push_back(std::string("2200"));
	args.push_back(std::string("-a_srs"));
	args.push_back(std::string("EPSG:32618"));
	//args.push_back(std::string("-z_multiply"));
	//args.push_back(std::string("10"));
	args.push_back(std::string("-sql"));
	args.push_back(std::string("select geom from wsample"));
	args.push_back(std::string("oci:elevate/elevate@domain15"));
	if (gdalGrid.useLaplace) {
		args.push_back(std::string("C:\\TEMP\\WSAMPLE-LaPlace.tif"));
	}
	else {
		args.push_back(std::string("C:\\TEMP\\WSAMPLE-Invdist.tif"));
	}

	int result = gdalGrid.Run(args);
	BOOST_CHECK(result == 0);

	time_t stopTime;
	time(&stopTime);
	MultiMapTest::ReportRunTime("GRID runtime ", static_cast<double>(stopTime - startTime));
}