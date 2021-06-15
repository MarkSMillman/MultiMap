#pragma warning ( disable : 4251 )
#include "MultiMap.h"
#include "ThreadTools.h"
#include "MFUtils.h"
#include <iostream>
#include <stdio.h>

#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
//#include <boost/process.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/system/error_code.hpp>

#pragma warning ( disable : 4996 )

#ifdef _WIN32
#define POPEN _popen
#define PCLOSE _pclose
#else
#define POPEN popen
#define PCLOSE pclose
#endif

MULTIMAP_API ThreadTools::~ThreadTools(void) {
	ClearResponse();
}
MULTIMAP_API ThreadTools::ThreadTools(void) {
}
/**
 * this doesn't work yet.  mmillman

MULTIMAP_API int ThreadTools::Run(std::string commandline) {
	namespace bf = boost::filesystem;;
	namespace bp = boost::process;
	namespace bpi = boost::process::initializers;
	namespace bio = boost::iostreams;

#if defined(BOOST_POSIX_API) 
#define SHELL_COMMAND_PREFIX "-c "
#elif defined(BOOST_WINDOWS_API) 
#define SHELL_COMMAND_PREFIX "/c "
#endif
	bp::pipe p = bp::create_pipe();
	bio::file_descriptor_sink sink(p.sink, bio::close_handle);
	boost::system::error_code ec;
	bf::path myShellPath = bp::shell_path();
	std::string cl = myShellPath.string() + SHELL_COMMAND_PREFIX;
	cl += commandline;
	bp::execute( bpi::set_cmd_line(cl),	bpi::bind_stdout(sink), bpi::set_on_error(ec));

	int rtc = ec.value();

	bio::file_descriptor_source source(p.source, bio::close_handle);
	bio::stream<bio::file_descriptor_source> is(source);

	std::string result;
	char buffer[4096];
	is.getline(buffer,sizeof(buffer));
    result = std::string(buffer);
	MFUtils mfUtils;
	mfUtils.ToStrings(response,result,"\n");

	return rtc;
}
**/
/**
 * This is a stopgap method as it cannot be called from a "Windows" program and will only run in command line mode
**/
MULTIMAP_API int ThreadTools::RunAndWait(std::string commandline) {
	int returnCode = 0;

	FILE* pipe = POPEN(commandline.c_str(), "r");
	if ( pipe ) {
		char buffer[4096];
		std::string result = "";
		while(!feof(pipe)) {
			if(fgets(buffer, sizeof(buffer), pipe) != NULL) {
				result += buffer;
			}
		}
		PCLOSE(pipe);
		MFUtils mfUtils;
		mfUtils.ToStrings(response,result,"\n");
	} else {
		returnCode = 1;
	}
	return returnCode;
}
MULTIMAP_API std::vector<std::string> ThreadTools::GetResponse(void) {

	return response;
}
MULTIMAP_API void ThreadTools::ClearResponse(void) {
	response.clear();
	response.resize(0);
}