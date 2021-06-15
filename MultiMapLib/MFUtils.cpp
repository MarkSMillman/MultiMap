/**
* Copyright 2014 Mizar, LLC
* All Rights Reserved.
*
* This file is part of Mizar's MultiMap software library.
* MultiMap is licensed under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version a copy of which is available at http://www.gnu.org/licenses/
*
* MultiMap is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You may NOT remove this copyright notice; it must be retained in any modified
* version of the software.
**/
#include "MultiMap.h"
#include "ShapeReader.h"
#include <sstream>

#pragma warning ( disable : 4996 )
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/asio.hpp>

#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <time.h>

#include "MFUtils.h"
#include <conio.h>
#include <stdio.h>
#undef min
#undef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
const double DEFAULT_PRECISION = 0.000000001;

MULTIMAP_API MFUtils::~MFUtils() {
}
MULTIMAP_API MFUtils::MFUtils() {
}

MULTIMAP_API double MFUtils::ElapsedSeconds(time_t begin, time_t end){
	return difftime(end, begin);
}
MULTIMAP_API double MFUtils::ElapsedSeconds(clock_t begin, clock_t end){
	return (end - begin) / 1000.0;
}
MULTIMAP_API std::string MFUtils::HostName(){
	char computerName[MAX_COMPUTERNAME_LENGTH + 1];
	boost::system::error_code ec;
	boost::asio::detail::socket_ops::gethostname(computerName, sizeof(computerName), ec);
	return std::string(computerName);
}
/**
* NOTE: we should modify this method so that the user has to hit two keys.
* @param seconds maximum number of seconds to wait for user input to continue.  0 waits forever.
**/
MULTIMAP_API size_t MFUtils::PauseUntilKeyStruck(std::string message, size_t seconds) {
	size_t actualSeconds = 0;
	if (seconds == 0) {
		seconds = ULLONG_MAX;
	}
	if (message.length() > 0) {
		std::cout << message << std::endl;
	}
	std::cout << "Pausing for " << seconds << " or until any key struck" << std::endl;
	for (size_t s = 0; s < seconds * 2; s++) {
		Sleep(static_cast<unsigned long>(500UL));
		if (s % 2) {
			actualSeconds++;
		}
		if (_kbhit() != 0) {
			_getch(); // this throws the key away otherwise it will remain in the queue for the next call to this metho
			break;
		}
	}
	return actualSeconds;
}

MULTIMAP_API bool MFUtils::Near(double value1, double value2, double precision){
	if (fabs(value1 - value2) > precision){
		return false;
	}
	else {
		return true;
	}
}

MULTIMAP_API bool MFUtils::Near(double value1, double value2){
	if (fabs(value1 - value2) > DEFAULT_PRECISION){
		return false;
	}
	else {
		return true;
	}
}

/**
* @param mode 0=empty string, 2=hh:mm, 3=hh:mm:ss, 5=yyyy:mo:da-hh:mm, 6=yyyy:mo:da-hh:mm:ss
* @return a string containing the requested time
**/
MULTIMAP_API std::string MFUtils::TimeIs(int mode){
	time_t now;
	time(&now);
	struct tm *c = localtime(&now);
	char timeis[22];
	switch (mode) {
	case 0:
		timeis[0] = 0;
		break;
	case 2:
		sprintf(timeis, "%02i:%02i", c->tm_hour, c->tm_min);
		break;
	case 3:
		sprintf(timeis, "%02i:%02i:%02i", c->tm_hour, c->tm_min, c->tm_sec);
		break;
	case 5:
		sprintf(timeis, "20%02i.%02i.%02i-%02i:%02i", c->tm_year - 100, c->tm_mon + 1, c->tm_mday, c->tm_hour, c->tm_min);
		break;
	case 6:
	default:
		// Note that this method is NOT 2100 compatible ;-)
		sprintf(timeis, "20%02i.%02i.%02i-%02i:%02i:%02i", c->tm_year - 100, c->tm_mon + 1, c->tm_mday, c->tm_hour, c->tm_min, c->tm_sec);
		break;
	}
	return std::string(timeis);
}

MULTIMAP_API std::string MFUtils::TimeFileName(void){
	time_t now;
	time(&now);
	struct tm *c = localtime(&now);
	char timeis[22];
	// Note that this method is NOT 2100 compatible ;-)
	sprintf(timeis, "20%02i.%02i.%02i-%02i.%02i.%02i", c->tm_year - 100, c->tm_mon + 1, c->tm_mday, c->tm_hour, c->tm_min, c->tm_sec);
	return std::string(timeis);
}

MULTIMAP_API void MFUtils::Dump(uint16_t* mem, int words) {
	for (int i = 0; i < words; i++){
		printf("%4x ", *(mem + i));
		if ((i + 1) % 16 == 0 && i != 0){
			printf("\n");
		}
	}
	printf("\n");
}

MULTIMAP_API int MFUtils::FolderExists(std::string folderName, bool createIfNot){
	int exists = 7;
	ClearError();
	try {
		boost::filesystem::path folderPath(folderName);
		if (folderPath.extension().string().length() > 0) {
			folderPath = folderPath.parent_path();
		}

		if (boost::filesystem::is_directory(folderPath, last_error)) {
			exists = 0;
		}
		else if (boost::filesystem::is_regular_file(folderPath, last_error)) {
			exists = FolderExists(folderPath.parent_path().string());
		}
		else if (boost::filesystem::is_other(folderPath, last_error)) {
			exists = 8;
		}
		else if (createIfNot) {
			try {
				if (boost::filesystem::create_directories(folderPath, last_error)) {
					exists = 0;
				}
				else {
					exists = 7;
				}
			}
			catch (...) {
				exists = 7;
			}
		}
	}
	catch (const boost::filesystem::filesystem_error& ex) {
		last_error = ex.code();
	}
	return exists;
}

MULTIMAP_API std::string MFUtils::ToLower(std::string astring) {
	return boost::algorithm::to_lower_copy(astring);
}

MULTIMAP_API std::string MFUtils::ToUpper(std::string astring) {
	return boost::algorithm::to_upper_copy(astring);
}
MULTIMAP_API bool MFUtils::BeginsWith(std::string astring, std::string pattern, bool caseInsensitive) {
	if (caseInsensitive) {
		astring = boost::algorithm::to_lower_copy(astring);
		pattern = boost::algorithm::to_lower_copy(pattern);
	}
	return boost::starts_with(astring, pattern);
}
MULTIMAP_API bool MFUtils::EndsWith(std::string astring, std::string pattern, bool caseInsensitive) {
	if (caseInsensitive) {
		astring = boost::algorithm::to_lower_copy(astring);
		pattern = boost::algorithm::to_lower_copy(pattern);
	}
	return boost::ends_with(astring, pattern);
}
MULTIMAP_API bool MFUtils::Contains(std::string astring, std::string pattern, bool caseInsensitive) {
	if (caseInsensitive) {
		astring = boost::algorithm::to_lower_copy(astring);
		pattern = boost::algorithm::to_lower_copy(pattern);
	}
	return boost::contains(astring, pattern);
}
MULTIMAP_API bool MFUtils::Compare(std::string astring, std::string pattern, bool caseInsensitive) {
	if (caseInsensitive) {
		astring = boost::algorithm::to_lower_copy(astring);
		pattern = boost::algorithm::to_lower_copy(pattern);
	}
	return boost::algorithm::equals(astring, pattern);
}
MULTIMAP_API bool MFUtils::MatchRegex(std::string source, std::string regexPattern) {
	bool match = false;
	boost::regex portablePattern(regexPattern);
	match = boost::regex_search(source, portablePattern);
	return match;
}
MULTIMAP_API std::string MFUtils::ReplaceRegex(std::string source, std::string regexPattern, std::string substitute) {
	boost::regex portablePattern(regexPattern);
	std::string target = boost::regex_replace(source, portablePattern, substitute, boost::format_sed | boost::match_posix);
	return target;
}
MULTIMAP_API std::string MFUtils::ReplaceAll(std::string original, std::string replaceThis, std::string withThis) {
	std::string rstring = original;
	size_t index = rstring.find(replaceThis);
	size_t len = replaceThis.length();
	while (index != std::string::npos) {
		rstring.replace(index, len, withThis);
		index = rstring.find(replaceThis);
	}
	return rstring;
}
MULTIMAP_API std::string MFUtils::ReplaceFirst(std::string original, std::string replaceThis, std::string withThis) {
	std::string rstring = original;
	size_t index = rstring.find_first_of(replaceThis);
	size_t len = replaceThis.length();
	if (index != std::string::npos) {
		rstring.replace(index, len, withThis);
	}
	return rstring;
}
MULTIMAP_API std::string MFUtils::ReplaceLast(std::string original, std::string replaceThis, std::string withThis) {
	std::string rstring = original;
	size_t index = rstring.find_last_of(replaceThis);
	size_t len = replaceThis.length();
	if (index != std::string::npos) {
		rstring.replace(index, len, withThis);
	}
	return rstring;
}
MULTIMAP_API bool MFUtils::IsPortableFileName(std::string fileName) {
	return boost::filesystem::portable_name(fileName);
}
MULTIMAP_API std::string MFUtils::GetPortableFileName(std::string fileName, char substitute) {
	static const boost::regex portablePattern("[^0-9a-zA-Z._-]");
	if (!IsPortableFileName(fileName)) {
		fileName = boost::regex_replace(fileName, portablePattern, "_", boost::format_sed | boost::match_posix);
	}
	return fileName;
}
MULTIMAP_API bool MFUtils::FileNamesEquivalent(std::string filename1, std::string filename2) {
	bool equivalent = false;

	std::string path1 = boost::filesystem::complete(filename1).string();
	std::string path2 = boost::filesystem::complete(filename2).string();
	path1 = ToLower(path1);
	path2 = ToLower(path2);
	path1 = ReplaceAll(path1, "\\.\\", "\\");
	path2 = ReplaceAll(path2, "\\.\\", "\\");
	if (path1.compare(path2) == 0) {
		equivalent = true;
	}
	return equivalent;
}

MULTIMAP_API std::string MFUtils::CurrentPath(void) {
	boost::filesystem::path full_path(boost::filesystem::current_path());
	return full_path.string();
}
MULTIMAP_API int MFUtils::FileExists(std::string fileName, bool createIfNot){
	int exists = 9;
	ClearError();
	try {
		boost::filesystem::path filePath(fileName);

		if (boost::filesystem::is_regular_file(filePath, last_error)) {
			exists = 0;
		}
		else if (boost::filesystem::is_directory(filePath, last_error)) {
			exists = 1;
		}
		else if (!boost::filesystem::is_other(filePath, last_error)) {
			if (createIfNot) {
				boost::filesystem::path ppath = filePath.parent_path();
				if (ppath.string().length() > 0) {
					boost::filesystem::create_directories(ppath, last_error);
				}
				FILE* f = fopen(fileName.c_str(), "w");
				fclose(f);
				exists = 0;
			}
		}
		else {
			exists = 9;
		}
	}
	catch (const boost::filesystem::filesystem_error& ex) {
		last_error = ex.code();
	}
	return exists;
}

MULTIMAP_API unsigned long long MFUtils::FileSize(std::string filePath) {
	unsigned long long size = 0;
	if (FileExists(filePath) == 0) {
		ClearError();
		size = boost::filesystem::file_size(filePath, last_error);
	}
	return size;
}
MULTIMAP_API int MFUtils::EmptyFolder(std::string & folderName){
	ClearError();
	return (int)boost::filesystem::remove_all(folderName, last_error);
}

MULTIMAP_API std::string MFUtils::Now(){
	boost::posix_time::ptime now = boost::posix_time::second_clock::universal_time();
	std::string szNow = to_simple_string(now);
	return szNow;
}

MULTIMAP_API void MFUtils::Common(char* szCommon, size_t _size, const char* szFileName1, const char* szFileName2) {
	memset(szCommon, 0, _size);
	size_t len1 = strlen(szFileName1);
	size_t len2 = strlen(szFileName2);
	size_t len = min(len1, len2);
	len = min(len, _size);
	int b = 0;
	for (size_t i = 0; i < len; i++) {
		if (szFileName1[i] == szFileName2[i]) {
			szCommon[b++] = szFileName1[i];
		}
	}
}

MULTIMAP_API void MFUtils::Common(char* szCommon, size_t _size, std::string fileName1, std::string fileName2) {
	memset(szCommon, 0, _size);
	const char* szFileName1 = fileName1.c_str();
	const char* szFileName2 = fileName2.c_str();

	size_t len1 = fileName1.length();
	size_t len2 = fileName2.length();
	size_t len = min(len1, len2);
	len = min(len, _size);

	int b = 0;
	for (size_t i = 0; i < len; i++) {
		if (szFileName1[i] == szFileName2[i]) {
			szCommon[b++] = szFileName1[i];
		}
	}
}

MULTIMAP_API std::string MFUtils::NowISO(){
	boost::posix_time::ptime now = boost::posix_time::second_clock::universal_time();
	std::string szNow = to_iso_string(now);
	size_t i = szNow.find_first_of(","); // trim any trailing fractional seconds
	if (i != std::string::npos) {
		szNow = szNow.substr(0, i);
	}
	return szNow;
}
MULTIMAP_API std::string MFUtils::GetComputername() {
	std::string computername;
	char szBuffer[2048];
	DWORD count = 2048;
	if (!GetComputerName(szBuffer, &count)) {
		computername = std::string(szBuffer);
	}
	return computername;
}
MULTIMAP_API std::string MFUtils::GetUsername() {
	std::string username;
	char szBuffer[2048];
	DWORD count = 2048;
	if (!GetUserName(szBuffer, &count)) {
		username = std::string(szBuffer);
	}
	return username;
}
MULTIMAP_API std::string MFUtils::GetOperatingSystem(size_t *majorVersion, size_t *minorVersion, size_t *build) {
	std::string os;
	DWORD dwVersion = 0;
	DWORD dwMajorVersion = 0;
	DWORD dwMinorVersion = 0;
	DWORD dwBuild = 0;

	dwVersion = GetVersion();

	dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));
	dwMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion)));
	if (dwVersion < 0x80000000) {
		dwBuild = (DWORD)(HIWORD(dwVersion));
	}
	if (majorVersion) {
		*majorVersion = static_cast<size_t>(dwMajorVersion);
	}
	if (minorVersion) {
		*minorVersion = static_cast<size_t>(dwMinorVersion);
	}
	if (build) {
		*build = static_cast<size_t>(dwBuild);
	}

	if (dwMajorVersion == 6) {
		if (dwMinorVersion == 3) {
			os = "Windows 8.1";
		}
		else if (dwMinorVersion == 2) {
			os = "Windows 8";
		}
		else if (dwMinorVersion == 1) {
			os = "Windows 7";
		}
		else if (dwMinorVersion == 0) {
			os = "Windows Vista";
		}
	}
	else if (dwMajorVersion == 5) {
		if (dwMinorVersion == 2) {
			os = "Windows XP Pro x64";
		}
		else if (dwMinorVersion == 1) {
			os = "Windows XP";
		}
		else if (dwMinorVersion == 0) {
			os = "Windows 2000";
		}
	}

	return os;
}
MULTIMAP_API std::string MFUtils::ExecutablePath() {
	std::string executablePath = "";
	char szModuleName[2048];
	if (GetModuleFileName(NULL, szModuleName, sizeof(szModuleName)) > 0) {
		executablePath = std::string(szModuleName);
	}
	return executablePath;
}
MULTIMAP_API std::string MFUtils::ExecutableName() {
	std::string executableName = "";
	char szModuleName[2048];
	if (GetModuleFileName(NULL, szModuleName, sizeof(szModuleName)) > 0) {
		std::string modulePath = std::string(szModuleName);
		std::string moduleName = GetName(modulePath);
		boost::filesystem::path stem = boost::filesystem::path(moduleName).stem();
		executableName = stem.string();
	}
	return executableName;
}

MULTIMAP_API long double MFUtils::CheckSum(double* pDoubles, size_t count){
	long double sum = 0;
	for (size_t i = 0; i < count; i++) {
		sum += pDoubles[i];
	}
	return sum;
}
MULTIMAP_API long double MFUtils::CheckSum(float* pFloats, size_t count) {
	long double sum = 0;
	for (size_t i = 0; i < count; i++) {
		sum += pFloats[i];
	}
	return sum;
}
MULTIMAP_API long long MFUtils::CheckSum(int* pInts, size_t count) {
	long long sum = 0;
	for (size_t i = 0; i < count; i++) {
		sum += pInts[i];
	}
	return sum;
}
MULTIMAP_API long long MFUtils::CheckSum(unsigned int* pUInts, size_t count) {
	long long sum = 0;
	for (size_t i = 0; i < count; i++) {
		sum += pUInts[i];
	}
	return sum;
}
MULTIMAP_API long long MFUtils::CheckSum(long* pLongs, size_t count) {
	long long sum = 0;
	for (size_t i = 0; i < count; i++) {
		sum += pLongs[i];
	}
	return sum;
}
MULTIMAP_API long long MFUtils::CheckSum(unsigned long* pULongs, size_t count) {
	long long sum = 0;
	for (size_t i = 0; i < count; i++) {
		sum += pULongs[i];
	}
	return sum;
}

MULTIMAP_API std::string MFUtils::ToString(int number, size_t minDigits) {
	char buffer[16];
	char format[16];
	if (minDigits > 11) {
		minDigits = 11;
	}
	if (minDigits > 0) {
		sprintf(format, "%%0%dd", minDigits);
	}
	else {
		strcpy(format, "%d");
	}
	sprintf(buffer, format, number);
	return std::string(buffer);
}

/**
EXAMPLE code snipit from calling class:
...
std::string startpath = "D:\\A_TofProjects\\NewTOFProj\\tof_data";
std::string targetExt = ".LAS";
boost::filesystem::path p(startpath);
boost::filesystem::directory_iterator itr(p);
std::vector<std::string> lasFiles;
mfUtils.FileSystemSearchByExt(lasFiles, targetExt, itr);
...
**/
MULTIMAP_API void  MFUtils::FileSystemSearchByExt(std::vector<std::string>& targetFiles,
	std::string targetExt, boost::filesystem::directory_iterator &itr)
{
	boost::filesystem::directory_iterator end_itr;
	ClearError();
	for (itr; itr != end_itr; ++itr) {
		boost::filesystem::path np = itr->path();
		if (is_regular_file(np, last_error)) {
			if (np.extension() == targetExt.c_str()) {
				std::string tmp = np.generic_string();
				boost::replace_all(tmp, "/", "\\");
				targetFiles.push_back(tmp.c_str());
			}
		}
		else {
			boost::filesystem::directory_iterator itr(np);
			FileSystemSearchByExt(targetFiles, targetExt, itr);
		}
	}
}

/**
EXAMPLE code snipit from calling class:
...
std::string startpath = "D:\\A_TofProjects\\NewTOFProj\\tof_data";
std::string regexExp = "^.*\\.LAS$";
boost::filesystem::path p(startpath);
boost::filesystem::directory_iterator itr(p);
std::vector<std::string> lasFiles;
mfUtils.FileSystemSearchByRegex(lasFiles, regexExp, itr);
...
**/

MULTIMAP_API void  MFUtils::FileSystemSearchByRegex(std::vector<std::string>& targetFiles,
	std::string regexExp, boost::filesystem::directory_iterator &itr)
{
	boost::filesystem::directory_iterator end_itr;
	ClearError();
	for (itr; itr != end_itr; ++itr) {
		boost::filesystem::path np = itr->path();
		if (is_regular_file(np, last_error)) {
			boost::regex expr(regexExp);
			std::string tmp = np.generic_string();
			if (boost::regex_match(tmp, expr)) {
				boost::replace_all(tmp, "/", "\\");
				targetFiles.push_back(tmp.c_str());
			}
		}
		else {
			boost::filesystem::directory_iterator itr(np);
			FileSystemSearchByRegex(targetFiles, regexExp, itr);
		}
	}
}

MULTIMAP_API bool MFUtils::FolderFileNameGood(std::string fname, bool msonly)
{
	std::string test = fname;
	boost::regex msexpr("[\\s<>:/\\\\|?*\"]");
	boost::regex ixexpr("[\\s/\\0]"); // thare maybe more this is at minimum
	bool hit = msonly ? boost::regex_search(test.begin(), test.end(), msexpr) :
		boost::regex_search(test.begin(), test.end(), ixexpr);
	return !hit;
}

/**
* @returns true if file exists and is a regular file (not a directory or symlink)
**/
MULTIMAP_API bool MFUtils::IsRegularFile(std::string fname) {
	ClearError();
	boost::filesystem::path p(fname);
	return boost::filesystem::is_regular_file(p, last_error);
}

/**
* @param filePath a file name or full file path
* @param checkPortable [false] if true ensure that name is portable.  Default is to check for valid native OS name.
* @returns true if the file name is a valid file name
**/
MULTIMAP_API bool MFUtils::IsRegularFileName(std::string filePath, bool checkPortable) {
	using namespace boost::filesystem;
	bool isRegularFileName = false;
	path p(filePath);
	std::string fileName = p.filename().string();
	if (checkPortable) {
		isRegularFileName = portable_name(fileName);
	}
	else {
		isRegularFileName = native(fileName);
	}

	if (isRegularFileName) {
		if (filePath.find_first_of('*') != std::string::npos) {
			isRegularFileName = false; // wild card
		}
		else if (filePath.find_first_of('?') != std::string::npos) {
			isRegularFileName = false; // wild card
		}
	}
	return isRegularFileName;
}

MULTIMAP_API std::string MFUtils::GetName(std::string filename, bool toLower /* =false */) {
	using namespace boost::filesystem;
	std::string name = "";
	if (toLower) {
		name = boost::algorithm::to_lower_copy(path(filename).filename().string());
	}
	else {
		name = path(filename).filename().string();
	}

	return name;
}

MULTIMAP_API std::string MFUtils::GetFolder(std::string filename, bool toLower /* =false */) {
	using namespace boost::filesystem;
	std::string folder = "";
	if (toLower) {
		folder = boost::algorithm::to_lower_copy(path(filename).parent_path().string());
	}
	else {
		folder = path(filename).parent_path().string();
	}
	return folder;
}

MULTIMAP_API std::string MFUtils::GetExtension(std::string filename, bool toLower /* =true */) {
	using namespace boost::filesystem;
	std::string extension = "";
	if (!is_directory(filename)) {
		if (toLower) {
			extension = boost::algorithm::to_lower_copy(path(filename).extension().string());
		}
		else {
			extension = path(filename).extension().string();
		}
	}
	if (extension.length() > 0) {
		if (extension.find_first_of(".") == 0) {
			extension = extension.substr(1, std::string::npos);
		}
	}
	return extension;
}

MULTIMAP_API MMM_IOBTYPE MFUtils::ExtensionType(std::string filename) {
	using namespace boost::filesystem;

	MMM_IOBTYPE fileType = IOT_UNKNOWN;

	if (is_directory(filename)) {
		fileType = IOT_FOLDER;
	}
	else {
		std::string extension = boost::algorithm::to_upper_copy(path(filename).extension().string());

		if (extension.compare(".BIN") == 0) {
			fileType = IOT_BIN;
		}
		else if (extension.compare(".LAS") == 0) {
			fileType = IOT_LAS;
		}
		else if (extension.compare(".LAZ") == 0) {
			fileType = IOT_LAZ;
		}
		else if (extension.compare(".TXT") == 0) {
			fileType = IOT_ASCII;
		}
		else if (extension.compare(".TIF") == 0 || extension.compare(".TIFF") == 0 || extension.compare(".TF8") == 0 || extension.compare(".BTF") == 0) {
			fileType = IOT_TIF;
		}
		else if (extension.compare(".JPG") == 0 || extension.compare(".JPEG") == 0) {
			fileType = IOT_JPG;
		}
		else if (extension.compare(".SHP") == 0) {
			fileType = IOT_SHP;
		}
		else if (extension.compare(".DBF") == 0) {
			fileType = IOT_DBF;
		}
		else if (extension.compare(".LDR") == 0) {
			fileType = IOT_LDR;
		}
	}

	return fileType;
}

MULTIMAP_API void  MFUtils::FileSystemSearchByExt2(std::vector<std::string>& targetFiles,
	std::string _targetExt, boost::filesystem::directory_iterator &itr, bool recursive, bool caseSensitive)
{
	std::string targetExt;
	if (caseSensitive) {
		targetExt = _targetExt;
	}
	else {
		targetExt = boost::algorithm::to_upper_copy(_targetExt);
	}
	if (targetExt.substr(0, 1).compare(".") != 0) {
		targetExt = "." + targetExt;
	}
	boost::filesystem::directory_iterator end_itr;
	for (itr; itr != end_itr; ++itr) {
		boost::filesystem::path np = itr->path();
		if (is_regular_file(np)) {
			std::string npExtension;
			if (caseSensitive) {
				npExtension = np.extension().string();
			}
			else {
				npExtension = boost::algorithm::to_upper_copy(np.extension().string());
			}
			if (npExtension.compare(targetExt) == 0) {
				std::string tmp = np.generic_string();
				boost::replace_all(tmp, "/", "\\");
				targetFiles.push_back(tmp.c_str());
			}
		}
		else if (recursive && boost::filesystem::is_directory(np)) {
			boost::filesystem::directory_iterator itr(np);
			FileSystemSearchByExt(targetFiles, targetExt, itr);
		}
	}
}

MULTIMAP_API bool PatchRegexStar(std::string & regexExp) {
	bool patched = false;
	size_t pos = 0;
	while ((pos = regexExp.find_first_of("*", pos)) != std::string::npos) {
		if (pos == 0 || regexExp.substr(pos - 1, 1).compare(".") != 0) {
			regexExp.replace(pos, 1, ".*");
			pos += 2;
			patched = true;
		}
	}
	pos = 0;
	while ((pos = regexExp.find_first_of("?", pos)) != std::string::npos) {
		if (pos == 0 || regexExp.substr(pos - 1, 1).compare(".") != 0) {
			regexExp.replace(pos, 1, ".");
			pos += 2;
			patched = true;
		}
	}
	return patched;
}

/**
EXAMPLE code snipit from calling class:
...
std::string startpath = "D:\\A_TofProjects\\NewTOFProj\\tof_data";
std::string regexExp = "^.*\\.LAS$";
boost::filesystem::path p(startpath);
boost::filesystem::directory_iterator itr(p);
std::vector<std::string> lasFiles;
mfUtils.FileSystemSearchByRegex(lasFiles, regexExp, itr);
...
**/
MULTIMAP_API void  MFUtils::FileSystemSearchByRegex2(std::vector<std::string>& targetFiles,
	std::string regexExp, boost::filesystem::directory_iterator &itr, bool recursive, bool caseSensitive)
{
	PatchRegexStar(regexExp);
	boost::filesystem::directory_iterator end_itr;
	for (itr; itr != end_itr; ++itr) {
		boost::filesystem::path np = itr->path();
		if (is_regular_file(np)) {
			boost::filesystem::path npp = np.parent_path();
			boost::regex expr;
			if (caseSensitive) {
				expr = boost::regex(expr);
			}
			else {
				expr = boost::regex(regexExp, boost::regex::icase);
			}
			std::string tmp = np.filename().generic_string();
			if (boost::regex_match(tmp, expr)) {
				boost::replace_all(tmp, "/", "\\");
				targetFiles.push_back(npp.string() + "\\\\" + np.filename().string());
			}
		}
		else if (recursive && boost::filesystem::is_directory(np)) {
			boost::filesystem::directory_iterator itr(np);
			FileSystemSearchByRegex(targetFiles, regexExp, itr);
		}
	}
}
MULTIMAP_API bool MFUtils::RemoveFile(std::string filename) {
	bool fileRemoved = false;
	std::string filePath = boost::filesystem::complete(filename).string();
	int attempts = 0;
	//std::cout << "RemoveFile opening " + filePath << std::endl;
	ClearError();
	while (!fileRemoved && attempts < 10) {
		fileRemoved = boost::filesystem::remove(filePath, last_error);
		if (!fileRemoved) {
			//std::cout << "RemoveFile failed trying again" << std::endl;
			Sleep(500); // wait half-a-sec and try again
			attempts++;
		}
		else {
			//std::cout << "RemoveFile succeeded" << std::endl;
		}
	}
	return fileRemoved;
}
/**
* The method looks for files with the same stem as shapefile and extensions of ".shp", ".dbf", ".shx", and ".prj" and
* removes them if they exist.
* @params shapefile name of a shape file.  The extension is ignored.
* @return the number of files removed.
**/
MULTIMAP_API int MFUtils::RemoveShapeFileSet(std::string shapefile){
	int filesRemoved = 0;
	boost::filesystem::path path = boost::filesystem::path(shapefile).parent_path();
	boost::filesystem::path stem = boost::filesystem::path(shapefile).stem();
	boost::filesystem::path extension = boost::filesystem::path(shapefile).extension();
	boost::filesystem::path newFileName;

	ClearError();
	newFileName = boost::filesystem::path(path.string() + FILE_SEP + stem.string() + ".shp");
	if (boost::filesystem::exists(newFileName, last_error)) {
		if (remove(newFileName.string().c_str()) == 0) {
			filesRemoved++;
		}
	}
	newFileName.clear();
	newFileName = boost::filesystem::path(path.string() + FILE_SEP + stem.string() + ".dbf");
	if (boost::filesystem::exists(newFileName, last_error)) {
		if (remove(newFileName.string().c_str()) == 0) {
			filesRemoved++;
		}
	}
	newFileName.clear();
	newFileName = boost::filesystem::path(path.string() + FILE_SEP + stem.string() + ".shx");
	if (boost::filesystem::exists(newFileName, last_error)) {
		if (remove(newFileName.string().c_str()) == 0) {
			filesRemoved++;
		}
	}
	newFileName.clear();
	newFileName = boost::filesystem::path(path.string() + FILE_SEP + stem.string() + ".prj");
	if (boost::filesystem::exists(newFileName, last_error)) {
		if (remove(newFileName.string().c_str()) == 0) {
			filesRemoved++;
		}
	}
	newFileName.clear();
	return filesRemoved;
}
/*
* This function is used as a generator function for MFUtils::NewFileName
* It is limited to 100 varients if the template character is a digit and
* 676 varients if the template character is alpha.
*/
MULTIMAP_API std::string MFUtils::NextString(std::string inputString) {
	std::string outputString = inputString;
	size_t l = inputString.length();
	char lc = inputString[l - 1];
	char lcp = 0;
	if (l > 1) {
		lcp = inputString[l - 2];
	}
	if (lc >= '0' && lc <= '9') {
		if (lc < '9') {
			outputString[l - 1] = lc + 1;
		}
		else {
			if (lcp) {
				outputString[l - 2] = lcp + 1;
				outputString[l - 1] = '0';
			}
		}
	}
	else if (lc >= 'A' && lc <= 'Z') {
		if (lc < 'Z') {
			outputString[l - 1] = lc + 1;
		}
		else {
			if (lcp) {
				outputString[l - 2] = lcp + 1;
				outputString[l - 1] = 'A';
			}
		}
	}
	else if (lc >= 'a' && lc <= 'z') {
		if (lc < 'z') {
			outputString[l - 1] = lc + 1;
		}
		else {
			if (lcp) {
				outputString[l - 2] = lcp + 1;
				outputString[l - 1] = 'a';
			}
		}
	}
	return outputString;
}
MULTIMAP_API void MFUtils::NewFileName(std::string & newFileName, const std::string oldFileName, const std::string appendChars) {

	boost::filesystem::path path = boost::filesystem::path(oldFileName).parent_path();
	boost::filesystem::path filename = boost::filesystem::path(oldFileName).filename();
	boost::filesystem::path stem = boost::filesystem::path(oldFileName).stem();
	boost::filesystem::path extension = boost::filesystem::path(oldFileName).extension();

	newFileName = path.string() + FILE_SEP + stem.string() + appendChars + extension.string();
}
MULTIMAP_API void MFUtils::EndCapFolder(std::string & folder) {
	size_t len = folder.length();
	if (!folder.find_last_of("/") == len - 1) {
		if (!folder.find_last_of("\\") == len - 1) {
			folder = folder + FILE_SEP;
		}
	}
}
MULTIMAP_API void MFUtils::Redirect(std::string & newFilePath, const std::string newDirectory, const std::string oldFilePath) {
	boost::filesystem::path dirPath = boost::filesystem::path(newDirectory);
	ClearError();
	if (!boost::filesystem::is_directory(dirPath, last_error)) {
		dirPath = dirPath.parent_path();
	}
	boost::filesystem::path filename = boost::filesystem::path(oldFilePath).filename();
	newFilePath = dirPath.string() + FILE_SEP + filename.string();
}

MULTIMAP_API void MFUtils::Retype(std::string & newFilePath, const std::string newType, const std::string oldFilePath) {
	boost::filesystem::path path = boost::filesystem::path(oldFilePath).parent_path();
	boost::filesystem::path stem = boost::filesystem::path(oldFilePath).stem();
	newFilePath = path.string() + FILE_SEP + stem.string() + newType;
}

MULTIMAP_API void MFUtils::Rename(std::string & newFilePath, const std::string newName, const std::string oldFilePath) {
	boost::filesystem::path path = boost::filesystem::path(oldFilePath).parent_path();
	boost::filesystem::path stem = boost::filesystem::path(oldFilePath).stem();
	boost::filesystem::path ext = boost::filesystem::path(oldFilePath).extension();
	newFilePath = path.string() + FILE_SEP + newName + ext.string();
}
MULTIMAP_API std::string MFUtils::SerializeName(const std::string fileName, int* previousNumber) {
	boost::filesystem::path path = boost::filesystem::path(fileName).parent_path();
	boost::filesystem::path stem = boost::filesystem::path(fileName).stem();
	boost::filesystem::path ext = boost::filesystem::path(fileName).extension();
	(*previousNumber)++;
	char buffer[1024];
	sprintf(buffer, "%s/%s_%03d%s", path.string().c_str(), stem.string().c_str(), (*previousNumber), ext.string().c_str());
	return std::string(buffer);
}
MULTIMAP_API std::string MFUtils::TemporaryFile(const std::string folder) {
	boost::filesystem::path temp = boost::filesystem::unique_path();

	std::string tempFile = folder + FILE_SEP + temp.string();
	return tempFile;
}

#pragma comment(lib, "User32.lib")
MULTIMAP_API unsigned long MFUtils::FileSystemSearch(std::vector<std::string>& targetFiles, std::string searchFolder, std::string fileMask, bool append, bool filesOnly, bool lidarOnly) {
	unsigned long dwError = 0;

	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	ClearError();
	try {
		boost::filesystem::path searchPath = boost::filesystem::path(searchFolder);

		if (NoErrors() && boost::filesystem::is_directory(searchPath, last_error)) {
			std::string filesPath = searchPath.string() + FILE_SEP + fileMask;
			hFind = FindFirstFile(filesPath.c_str(), &ffd);

			if (INVALID_HANDLE_VALUE != hFind) {
				if (!append) {
					targetFiles.clear();
				}
				do {
					if (!filesOnly || !(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
						if (searchPath.string().length() > 0) {
							std::string fPath = searchPath.string() + FILE_SEP + std::string(ffd.cFileName);
							if (!lidarOnly || KnownLidarType(fPath)) {
								targetFiles.push_back(searchPath.string() + FILE_SEP + std::string(ffd.cFileName));
							}
						}
						else {
							if (!lidarOnly || KnownLidarType(std::string(ffd.cFileName))) {
								targetFiles.push_back(std::string(ffd.cFileName));
							}
						}
					}
				} while (FindNextFile(hFind, &ffd) != 0);
				dwError = GetLastError();
				if (dwError == ERROR_NO_MORE_FILES) {
					dwError = 0;
				}
				FindClose(hFind);
			}
			else {
				dwError = GetLastError();
			}
		}
	}
	catch (const boost::filesystem::filesystem_error& ex) {
		last_error = ex.code();
		dwError = (unsigned long)last_error.value();
	}
	return dwError;
}
MULTIMAP_API unsigned long MFUtils::FileSystemSearch(std::vector<std::string>& targetFiles, std::string filesPath, bool append, bool lidarOnly) {

	unsigned long dwError = 0;
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	ClearError();
	try {
		boost::filesystem::path parentDir = boost::filesystem::path(filesPath);

		if (NoErrors() && boost::filesystem::is_directory(parentDir)) {
			filesPath.append(FILE_SEP);
			filesPath.append("*");
		}
		else {
			parentDir = boost::filesystem::path(filesPath).parent_path();
		}

		hFind = FindFirstFile(filesPath.c_str(), &ffd);

		if (INVALID_HANDLE_VALUE != hFind) {
			if (!append) {
				targetFiles.clear();
			}
			do {
				if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
					if (parentDir.string().length() > 0) {
						std::string fPath = parentDir.string() + FILE_SEP + std::string(ffd.cFileName);
						if (!lidarOnly || KnownLidarType(fPath, true)) {
							targetFiles.push_back(parentDir.string() + FILE_SEP + std::string(ffd.cFileName));
						}
					}
					else {
						if (!lidarOnly || KnownLidarType(std::string(ffd.cFileName, true))) {
							targetFiles.push_back(std::string(ffd.cFileName));
						}
					}
				}
			} while (FindNextFile(hFind, &ffd) != 0);
			dwError = GetLastError();
			if (dwError == ERROR_NO_MORE_FILES) {
				dwError = 0;
			}
			FindClose(hFind);
		}
		else {
			dwError = GetLastError();
		}
	}
	catch (const boost::filesystem::filesystem_error& ex) {
		last_error = ex.code();
	}
	return dwError;
}

MULTIMAP_API long long MFUtils::AvailableSpace(std::string _filePath) {
	boost::filesystem::space_info infon;
	long long availableSpace = 0;
	ClearError();
	try {
		boost::filesystem::path p = boost::filesystem::path(_filePath);
		if (NoErrors()) {
			boost::filesystem::path a = boost::filesystem::absolute(p);
			boost::filesystem::path rn = a.root_name();
			infon = boost::filesystem::space(rn);
			availableSpace = (long long)infon.available;
		}
	}
	catch (const boost::filesystem::filesystem_error& ex) {
		last_error = ex.code();
	}
	return availableSpace;
}

MULTIMAP_API bool MFUtils::KnownListType(std::string filename) {
	using namespace boost::filesystem;
	bool isValid = false;
	boost::filesystem::path filePath = path(filename);
	std::string extension = boost::algorithm::to_upper_copy(filePath.extension().string());

	if (extension.compare(".LST") == 0) {
		isValid = true;
	}

	return isValid;
}

MULTIMAP_API bool MFUtils::KnownLidarType(std::string filepath, bool strict) {
	bool isValid = false;

	MMM_IOBTYPE type = ExtensionType(filepath);
	boost::filesystem::path path = boost::filesystem::path(filepath);
	boost::filesystem::path ppath = path.parent_path();

	if (type == IOT_BIN || type == IOT_LAS || type == IOT_LAZ || type == IOT_LDR) { // || type == FOLDER) { // add FOLDER later
		isValid = true;
	}
	else if (strict) {
		isValid = false;
	}
	else if (boost::filesystem::is_directory(path)) {
		isValid = true;
	}
	else if (boost::filesystem::is_directory(ppath)) {
		if (boost::filesystem::exists(ppath)) {
			std::vector<std::string> files;
			boost::filesystem::directory_iterator itr = boost::filesystem::directory_iterator(path.parent_path());
			FileSystemSearchByExt2(files, std::string("LAS"), itr, false, false);
			if (files.size() > 0) {
				isValid = true;
			}
			else {
				itr = boost::filesystem::directory_iterator(path.parent_path());
				FileSystemSearchByExt2(files, std::string("LAZ"), itr, false, false);
				if (files.size() > 0) {
					isValid = true;
				}
				else {
					itr = boost::filesystem::directory_iterator(path.parent_path());
					FileSystemSearchByExt2(files, std::string("BIN"), itr, false, false);
					if (files.size() > 0) {
						isValid = true;
					}
				}
			}
		}
	}
	return isValid;
}

MULTIMAP_API bool MFUtils::DecomposePath(std::string & parentFolder, std::string & fileName, std::string fullPath) {
	bool success = false;
	ClearError();
	try {
		boost::filesystem::path path = boost::filesystem::path(fullPath);
		if (NoErrors()) {
			parentFolder = path.parent_path().string();
			fileName = path.filename().string();
			success = true;
		}
	}
	catch (const boost::filesystem::filesystem_error& ex) {
		last_error = ex.code();
	}
	return success;
}
/**
* @param files a vector to load with the LidarFiles found in filePath
* @param filePath a folder or wild-card folderfile\*.[BIN|LAS] type descriptor indicating where to get the files or a LST file containing a list of files
* @param append [true] indicates whether to append to files or to clear it first
**/
MULTIMAP_API bool MFUtils::LoadFiles(std::vector<std::string> & files, std::string filePath, bool append) {
	bool filesFound = false;
	if (!append) {
		files.clear();
	}
	if (filePath.length() > 0) {
		if (IsRegularFile(filePath)) {
			if (KnownListType(filePath)) {
				ReadFiles(files, filePath, append);
				if (files.size() > 0) {
					filesFound = true;
				}
			}
			else if (KnownLidarType(filePath)) {
				files.push_back(filePath);
				filesFound = true;
			}
		}
		else {
			bool lidarOnly = true;
			FileSystemSearch(files, filePath, append, lidarOnly);
			if (files.size() > 0) {
				filesFound = true;
			}
		}
	}
	return filesFound;
}

MULTIMAP_API bool MFUtils::ReadFiles(std::vector<std::string> & files, std::string filePath, bool append) {
	bool filesFound = false;
	if (!append) {
		files.clear();
	}
	if (filePath.length() > 0) {
		FILE* pFile = fopen(filePath.c_str(), "r");
		char buffer[1024];
		while (!feof(pFile)){
			int n = fscanf(pFile, "%s%*[^\n]", buffer);
			if (n > 0) {
				files.push_back(std::string(buffer));
			}
		}
		fclose(pFile);
	}
	return filesFound;
}

MULTIMAP_API size_t MFUtils::FindMatchingFile(std::vector<std::string> & files, std::string fileToMatchByName) {
	size_t index = UINT_MAX;
	ClearError();
	try {
		boost::filesystem::path sp = boost::filesystem::path(fileToMatchByName);
		if (NoErrors()) {
			std::string sstem = sp.stem().string();
			for (size_t i = 0; i < files.size(); i++) {
				boost::filesystem::path tp = boost::filesystem::path(files[i]);
				if (NoErrors()) {
					std::string tstem = tp.stem().string();
					if (sstem.compare(tstem) == 0) {
						index = i;
						break;
					}
				}
				else {
					break;
				}
			}
		}
	}
	catch (const boost::filesystem::filesystem_error& ex) {
		last_error = ex.code();
	}
	return index;
}

/**
* This version of FindMatchingFile looks for the best match based on end of file name matching.
* Translator::Compare uses this to match files based on the last characters in the file name.
* @param files
* @param fileToMatchByName
* @param minimumTailToHeadMatch the minimum number of ending characters that the file names must match to be considered matching
**/
MULTIMAP_API size_t MFUtils::FindMatchingFile(std::vector<std::string> & files, std::string fileToMatchByName, size_t minimumTailToHeadMatch) {
	size_t index = UINT_MAX;
	ClearError();
	try {
		boost::filesystem::path sp = boost::filesystem::path(fileToMatchByName);
		if (NoErrors()) {
			std::string sstem = sp.stem().string();
			size_t slen = sstem.length();
			std::vector<size_t> tailToHeadMatch(0);
			tailToHeadMatch.resize(files.size(), 0);
			size_t maxMatch = 0;
			size_t maxMatchIndex = 0;
			for (size_t i = 0; i < files.size(); i++) {
				boost::filesystem::path tp = boost::filesystem::path(files[i]);
				if (NoErrors()) {
					std::string tstem = tp.stem().string();
					if (tstem.length() == slen) {
						for (size_t p = slen - 1; p > 0; p--) {
							if (sstem[p] == tstem[p]) {
								tailToHeadMatch[i]++;
							}
							else {
								break;
							}
						}
						if (tailToHeadMatch[i] > maxMatch) {
							maxMatch = tailToHeadMatch[i];
							maxMatchIndex = i;
						}
					}
				}
				else {
					break;
				}
			}
			if (NoErrors() && maxMatch >= minimumTailToHeadMatch) {
				size_t maxMatchCount = 0;
				for (size_t i = 0; i < files.size(); i++) {
					if (tailToHeadMatch[i] == maxMatch) {
						maxMatchCount++;
					}
				}
				if (maxMatchCount == 1) {
					index = maxMatchIndex;
				}
			}
		}
	}
	catch (const boost::filesystem::filesystem_error& ex) {
		last_error = ex.code();
	}
	return index;
}

MULTIMAP_API double MFUtils::Round(double number) {
	return number < 0.0 ? ceil(number - 0.5) : floor(number + 0.5);
}

static const char* SIGNATURE = "WOOLPERT";
MULTIMAP_API time_t MFUtils::GetRemainingSeconds(std::string filePath) {
	time_t secondsRemaining = LLONG_MAX;
	if (filePath.length() == 0) {
		filePath = ExecutablePath();
	}
	FILE* file = fopen(filePath.c_str(), "rb");
	if (file) {
		long long offset = -static_cast<long long>(sizeof(time_t) + sizeof(SIGNATURE));
		long long rr = _fseeki64(file, offset, SEEK_END);
		bool patched = true;
		for (size_t i = 0; i < sizeof(SIGNATURE); i++) {
			char c;
			c = fgetc(file);
			if (c != SIGNATURE[i]) {
				patched = false;
				break;
			}
		}
		if (patched) {
			time_t patchTime;
			char* p = (char*)&patchTime;
			for (size_t i = 0; i < sizeof(time_t); i++) {
				*p = fgetc(file);
				p++;
			}
			fclose(file);
			file = NULL;
			time_t now = time(NULL);
			if (patchTime >= now) {
				secondsRemaining = patchTime - now;
			}
			else {
				secondsRemaining = 0;
			}
		}
	}
	return secondsRemaining;
}

MULTIMAP_API bool MFUtils::PatchKillTime(std::string filePath, time_t killDate) {
	bool patched = false;
	time_t existingPatch = LLONG_MAX; //GetRemainingSeconds(filePath);
	long long r = 0LL;
	FILE* file = fopen(filePath.c_str(), "a+b");
	if (file) {
		if (existingPatch < LLONG_MAX) {
			// the patch already exists
			long long offset = -static_cast<long long>(sizeof(time_t));
			r = _fseeki64(file, offset, SEEK_END);
		}
		else {
			r = fseek(file, 0L, SEEK_END);
			size_t s = sizeof(SIGNATURE);
			fwrite(SIGNATURE, sizeof(SIGNATURE), 1, file);
		}
		fwrite(&killDate, sizeof(killDate), 1, file);
		fclose(file);
		patched = true;
	}
	return patched;
}

MULTIMAP_API bool MFUtils::GetDayOfYear(int *year, int *dayOfYear, std::string sdate, int format) {
	using namespace boost::gregorian;
	bool success = true;
	date d;
	try {
		if (sdate.length() > 0) {
			if (format == AU_DATE_ISO) {
				d = date(date_from_iso_string(sdate));
			}
			else if (format == AU_DATE_YYMMDD) {
				int year, month, day;
				sscanf(sdate.c_str(), "%2d%2d%2d", &year, &month, &day);
				d = date(year + 2000, month, day);
			}
			else if (format == AU_DATE_MMDDYY) {
				int year, month, day;
				sscanf(sdate.c_str(), "%2d%2d%2d", &month, &day, &year);
				d = date(year + 2000, month, day);
			}
			else if (format == AU_DATE_DDMMYY) {
				int year, month, day;
				sscanf(sdate.c_str(), "%2d%2d%2d", &day, &month, &year);
				d = date(year + 2000, month, day);
			}
		}
		else {
			d = date(day_clock::universal_day());
		}
		*year = d.year();
		*dayOfYear = d.day_of_year();
	}
	catch (...) {
		success = false;
	}
	return success;
}
MULTIMAP_API bool MFUtils::GetDayOfYear(int *dayOfYear, int year, int month, int day) {
	using namespace boost::gregorian;
	bool success = true;
	try {
		date d(year, month, day);
		*dayOfYear = d.day_of_year();
	}
	catch (...) {
		success = false;
	}
	return success;
}
MULTIMAP_API std::string MFUtils::GetDate(int format, bool gmt) {
	using namespace boost::gregorian;
	std::string dateString;
	date d;
	if (gmt) {
		d = date(day_clock::universal_day());
	}
	else {
		d = date(day_clock::local_day());
	}
	if (format == AU_DATE_ISO) {
		dateString = to_iso_string(d);
	}
	else if (format == AU_DATE_YYMMDD) {
		dateString = to_iso_string(d);
		dateString = dateString.substr(2, std::string::npos);
	}
	else if (format == AU_DATE_MMDDYY) {
		int year = d.year();
		int month = d.month();
		int day = d.day();
		char buffer[8];
		sprintf(buffer, "%02d%02d%02d", month, day, year - 2000);
		dateString = std::string(buffer);
	}
	else if (format == AU_DATE_DDMMYY) {
		int year = d.year();
		int month = d.month();
		int day = d.day();
		char buffer[8];
		sprintf(buffer, "%02d%02d%02d", day, month, year - 2000);
		dateString = std::string(buffer);
	}
	else {
		dateString = to_iso_string(d);
	}
	return dateString;
}

MULTIMAP_API bool MFUtils::GetDate(int *month, int *day, int year, int dayOfYear){
	using namespace boost::gregorian;
	bool success = true;
	if (dayOfYear < 1 || dayOfYear > 366) {
		success = false;
	}
	else {
		try {
			date d(year, 1, 1);
			date_duration dd(dayOfYear - 1);
			d = d + dd;
			*month = d.month();
			*day = d.day();
		}
		catch (...) {
			success = false;
		}
	}
	return success;
}
MULTIMAP_API std::string MFUtils::GetCompletePath(std::string fileName, bool tolower) {
	std::string filePath = boost::filesystem::complete(fileName).string();
	if (tolower) {
		filePath = ToLower(filePath);
	}
	return filePath;
}
MULTIMAP_API int MFUtils::GetForeAft(std::string _fileName) {
	std::string fileName = GetName(_fileName, true);
	int foreAft = 0;
	if (fileName.find("aft") != std::string::npos && fileName.find("for") != std::string::npos) {
		foreAft = 3;
	}
	else if (fileName.find("aft") != std::string::npos) {
		foreAft = 2;
	}
	else if (fileName.find("for") != std::string::npos) {
		foreAft = 1;
	}
	else {
		// if it is not in the file name, check the directory names.
		fileName = GetCompletePath(_fileName, true);
		if (fileName.find("aft") != std::string::npos && fileName.find("for") != std::string::npos) {
			foreAft = 3;
		}
		else if (fileName.find("aft") != std::string::npos) {
			foreAft = 2;
		}
		else if (fileName.find("for") != std::string::npos) {
			foreAft = 1;
		}
		else {
			foreAft = 3;
		}
	}
	return foreAft;
}

MULTIMAP_API void MFUtils::UUID(unsigned char uuid[16]) {
	boost::uuids::uuid u = boost::uuids::random_generator()();
	memcpy(uuid, &u, 16);
}
MULTIMAP_API void MFUtils::UUID(std::string & uuid) {
	boost::uuids::uuid u = boost::uuids::random_generator()();
	uuid = boost::lexical_cast<std::string>(u);
}
MULTIMAP_API void MFUtils::UUIDFromString(unsigned char uuid[16], std::string & suuid) {
	boost::uuids::string_generator gen;
	boost::uuids::uuid u = gen(suuid);
	memcpy(uuid, &u, 16);
}
MULTIMAP_API std::string MFUtils::UUIDToString(unsigned char uuid[16]) {
	boost::uuids::uuid u;
	memcpy(&u, uuid, 16);
	return boost::lexical_cast<std::string>(u);
}
MULTIMAP_API bool MFUtils::UUIDCompare(unsigned char uuid1[16], unsigned char uuid2[16]) {
	bool same = true;
	for (int i = 0; i < 16; i++) {
		if (uuid1[i] != uuid2[i]) {
			same = false;
			break;
		}
	}
	return same;
}

MULTIMAP_API bool MFUtils::CropWindowsFromShapeFile(std::string shapeFile, std::string gridLabel, std::vector<MBRect> & cropWindows, std::vector<std::string> & windowNames) {
	bool success = false;
	if (shapeFile.length() > 0) {
		if (boost::filesystem::is_regular_file(shapeFile)) {
			MFUtils mfUtils;
			MMM_IOBTYPE extType = mfUtils.ExtensionType(shapeFile);
			if (extType == IOT_SHP) {
				SHPHandle hShape = SHP_Open(shapeFile.c_str(), "r");
				std::string dbfFileName = boost::filesystem::change_extension(shapeFile, ".dbf").string();
				DBFHandle hDbf = DBFOpen(dbfFileName.c_str(), "r");
				if (hShape != NULL) {
					int nRecords, shapeType;
					int nDbfRecords = 0;
					int labelIndex = -1;
					double padfMinBound[4], padfMaxBound[4];
					SHP_GetInfo(hShape, &nRecords, &shapeType, padfMinBound, padfMaxBound);
					if (hDbf) {
						nDbfRecords = hDbf->nRecords;
						if (nDbfRecords == nRecords) {
							std::string gridLabelUC = boost::algorithm::to_upper_copy(gridLabel);
							for (int f = 0; f < hDbf->nFields; f++){
								char pszFieldName[64];
								int pnWidth, pnDecimals;
								DBFGetFieldInfo(hDbf, f, pszFieldName, &pnWidth, &pnDecimals);
								std::string fieldName = boost::algorithm::to_upper_copy(std::string(pszFieldName));
								//std::cout<< fieldName << std::endl;
								if (fieldName.compare(gridLabelUC) == 0) {
									labelIndex = f;
									break;
								}
							}
						}
						if (labelIndex < 0) {
							DBFClose(hDbf);
							hDbf = NULL;
						}
					}
					cropWindows.resize(nRecords);
					windowNames.resize(nRecords);
					for (int i = 0; i < nRecords; i++) {
						SHPObject* record = SHP_ReadObject(hShape, i);
						if (SHPIsClosedShape(nRecords, shapeType, record)) {
							cropWindows[i].Reset();
							for (int j = 0; j < record->nVertices; j++) {
								cropWindows[i].minX = min(cropWindows[i].minX, record->padfX[j]);
								cropWindows[i].maxX = max(cropWindows[i].maxX, record->padfX[j]);
								cropWindows[i].minY = min(cropWindows[i].minY, record->padfY[j]);
								cropWindows[i].maxY = max(cropWindows[i].maxY, record->padfY[j]);
							}
							if (hDbf && labelIndex >= 0) {
								char* name = (char*)DBFReadStringAttribute(hDbf, i, labelIndex);
								windowNames[i] = std::string(name);
							}
							else {
								double maxmax = max(cropWindows[i].maxX - cropWindows[i].minX, cropWindows[i].maxY - cropWindows[i].minY);
								long xname = static_cast<long>(floor(cropWindows[i].minX / 1000.0));
								long yname = static_cast<long>(floor(cropWindows[i].minY / 1000.0));
								char wname[1024];
								sprintf(wname, "%d_%d", xname, yname);
								windowNames[i] = std::string(wname);
							}
							success = true;
						}
					}
					SHP_Close(hShape);
					hShape = NULL;
					if (hDbf) {
						DBFClose(hDbf);
						hDbf = NULL;
					}
				}
			}
		}
	}
	return success;
}
MULTIMAP_API int MFUtils::GetLastErrorCode(void) {
	return last_error.value();
}
MULTIMAP_API std::string MFUtils::GetLastErrorMessage(void) {
	if (NoErrors()) {
		return "";
	}
	else {
		return last_error.message();
	}
}
MULTIMAP_API bool MFUtils::NoErrors(void) {
	return last_error.value() == boost::system::errc::success ? true : false;
}
MULTIMAP_API void MFUtils::ClearError(void) {
	last_error.clear();
}
MULTIMAP_API void MFUtils::Sleep(unsigned int milliseconds) {
	boost::this_thread::sleep_for(boost::chrono::milliseconds(milliseconds));
}
MULTIMAP_API void MFUtils::ToStrings(std::vector<std::string> & strings, std::string inputString, std::string breakString) {
	strings.clear();
	strings.resize(0);
	size_t len = 0;
	size_t pos = 0;
	size_t sizeOfInputString = inputString.size();

	while (pos < sizeOfInputString) {
		size_t npos = inputString.find(breakString, pos);
		std::string subString = inputString.substr(pos, npos - pos);
		strings.push_back(subString);
		if (npos == std::string::npos) {
			break;
		}
		else {
			pos = npos + 1;
		}
		subString.clear();
	}
}
MULTIMAP_API void MFUtils::ToStrings(std::vector<std::string> & strings, char* zeroTerminatedChars, size_t sizeOfZTC) {
	strings.clear();
	strings.resize(0);
	size_t len = 0;
	size_t pos = 0;
	while (pos < sizeOfZTC) {
		strings.push_back(std::string(&zeroTerminatedChars[pos]));
		pos = pos + strlen(&zeroTerminatedChars[pos]) + 1;
	}
}
MULTIMAP_API void MFUtils::FromStrings(char* zeroTerminatedChars[], size_t *sizeOfZTC, std::vector<std::string> strings) {
	*sizeOfZTC = 0;
	for (size_t s = 0; s < strings.size(); s++) {
		*sizeOfZTC += strings[s].length() + 1;
	}
	*zeroTerminatedChars = new char[*sizeOfZTC];
	memset(*zeroTerminatedChars, 0, *sizeOfZTC);
	size_t pos = 0;
	for (size_t s = 0; s < strings.size(); s++) {
		strcpy(&(*zeroTerminatedChars)[pos], strings[s].c_str());
		pos = strings[s].length() + 1;
	}
}
MULTIMAP_API void MFUtils::ParseList(std::vector<int> & ints, std::string commaSeparatedString) {
	ints.clear();
	std::vector<std::string> intStrings;
	ToStrings(intStrings, commaSeparatedString, ",");
	for (size_t i = 0; i < intStrings.size(); i++) {
		int inti = atoi(intStrings[i].c_str());
		ints.push_back(inti);
	}
}
MULTIMAP_API void MFUtils::ParseList(std::vector<double> & doubles, std::string commaSeparatedString) {
	doubles.clear();
	std::vector<std::string> dfStrings;
	ToStrings(dfStrings, commaSeparatedString, ",");
	for (size_t i = 0; i < dfStrings.size(); i++) {
		double df = atof(dfStrings[i].c_str());
		doubles.push_back(df);
	}
}