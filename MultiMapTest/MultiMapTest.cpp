#define BOOST_TEST_MAIN
#include "MultiMap.h"
#include "MultiMapTest.h"
#include "MFUtils.h"
#include <boost/test/unit_test.hpp>
#include <stdarg.h>

bool MultiMapTest::verbose = true;

STATIC std::string MultiMapTest::TestFolder(std::string subfolder) {
	MFUtils mfUtils;
	std::string testFolder = "";

	int argc = ::boost::unit_test::framework::master_test_suite().argc;
	if ( argc > 1 ) {
		char **argv = ::boost::unit_test::framework::master_test_suite().argv;
		testFolder = std::string(argv[1]);
	}

	if ( testFolder.length() == 0 ) {
		char *pValue;
		size_t len;
		errno_t err = _dupenv_s( &pValue, &len, "MIZARASTER_TEST_FOLDER" );
		if ( !err && pValue ) {
			testFolder = std::string(pValue);
		}
	}

	if ( testFolder.length() == 0 ) {

		if ( mfUtils.FolderExists("C:\\\\Source\\SVN-CPP\\MultiMap\\SampleData\\Test", false) == 0 ) {
			testFolder = "C:\\\\Source\\SVN-CPP\\MultiMap\\SampleData\\Test";
		} else 		if ( mfUtils.FolderExists("D:\\\\Source\\SVN-CPP\\MultiMap\\SampleData\\Test", false) == 0 ) {
			testFolder = "D:\\\\Source\\SVN-CPP\\MultiMap\\SampleData\\Test";
		} else 		if ( mfUtils.FolderExists("E:\\\\Source\\SVN-CPP\\MultiMap\\SampleData\\Test", false) == 0 ) {
			testFolder = "E:\\\\Source\\SVN-CPP\\MultiMap\\SampleData\\Test";
		} else 		if ( mfUtils.FolderExists("F:\\\\Source\\SVN-CPP\\MultiMap\\SampleData\\Test", false) == 0 ) {
			testFolder = "F:\\\\Source\\SVN-CPP\\MultiMap\\SampleData\\Test";
		} else 		if ( mfUtils.FolderExists("G:\\\\Source\\SVN-CPP\\MultiMap\\SampleData\\Test", false) == 0 ) {
			testFolder = "G:\\\\Source\\SVN-CPP\\MultiMap\\SampleData\\Test";
		} else 		if ( mfUtils.FolderExists("H:\\\\Source\\SVN-CPP\\MultiMap\\SampleData\\Test", false) == 0 ) {
			testFolder = "H:\\\\Source\\SVN-CPP\\MultiMap\\SampleData\\Test";
		}
	}

	if ( testFolder.length() > 0 && subfolder.length() > 0 ) {
		std::string projectFolder = std::string(testFolder) + FILE_SEP + subfolder;
		if ( mfUtils.FolderExists(projectFolder,false) == 0 ) {
			testFolder = projectFolder.c_str();
		} else {
			testFolder = "";
		}
	}

	return testFolder;
}

STATIC void MultiMapTest::Message(const char* format, ...) {
	char buffer[12048];
	va_list argp;
	va_start(argp,format);
	vsprintf(buffer,format, argp);
	va_end(argp);
	BOOST_TEST_MESSAGE(buffer);
}
STATIC void MultiMapTest::Printf(const char* format, ...) {
	char buffer[12048];
	va_list argp;
	va_start(argp,format);
	vsprintf(buffer,format, argp);
	va_end(argp);
	printf("%s\n",buffer);
}
STATIC void MultiMapTest::ReportRunTime(std::string message, double seconds ) {
	if ( seconds < 60.0 ) {
		MultiMapTest::Message("%s run time %.2lf s", message.c_str(), seconds);
	} else if (seconds < 3600.0 ) {
		size_t minutes = static_cast<size_t>(seconds/60.0);
		seconds -= static_cast<double>(minutes)*60.0;
		MultiMapTest::Message("%s run time %d m %.2lf s", message.c_str(), minutes, seconds);
	} else {
		size_t hours  = static_cast<size_t>(seconds/3600.0);
		seconds -= static_cast<double>(hours)*3600.0;
		size_t minutes = static_cast<size_t>(seconds/60.0);
		seconds -= static_cast<double>(minutes)*60.0;
		MultiMapTest::Message("%s run time %d h %d m %.2lf s", message.c_str(), hours, minutes, seconds);
	}
}