#pragma once
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
#include <time.h>
#include <stdio.h>
#include <boost/filesystem.hpp>
#include "MultiMap.h"
#include "MBRect.h"

/*
* DO NOT PUT ANY VARIABLES IN THE MFUtils Class M.Millman
*/
#define AU_DATE_ISO 0
#define AU_DATE_YYMMDD 1
#define AU_DATE_MMDDYY 2
#define AU_DATE_DDMMYY 3

class MFUtils
{
public:
	MULTIMAP_API ~MFUtils();
	MULTIMAP_API MFUtils();

	MULTIMAP_API double ElapsedSeconds(time_t begin, time_t end);
	MULTIMAP_API double ElapsedSeconds(clock_t begin, clock_t end);
	MULTIMAP_API size_t PauseUntilKeyStruck(std::string message="", size_t seconds=0);
	MULTIMAP_API std::string Now();
	MULTIMAP_API std::string NowISO();
	MULTIMAP_API std::string HostName();
	MULTIMAP_API std::string ExecutableName();
	MULTIMAP_API std::string ExecutablePath();
	MULTIMAP_API std::string GetUsername();
	MULTIMAP_API std::string GetComputername();
	MULTIMAP_API std::string GetOperatingSystem(size_t *majorVersion=NULL, size_t *minorVersion=NULL, size_t *build=NULL);
	MULTIMAP_API std::string TimeIs(int mode = 3);
	MULTIMAP_API time_t GetRemainingSeconds(std::string filePath = "");
	MULTIMAP_API bool PatchKillTime(std::string filePath, time_t killDate);

	MULTIMAP_API void UUID(unsigned char uuid[16]);
	MULTIMAP_API void UUID(std::string & uuid);
	MULTIMAP_API void UUIDFromString(unsigned char uuid[16],std::string & suuid);
	MULTIMAP_API std::string UUIDToString(unsigned char uuid[16]);
	MULTIMAP_API bool UUIDCompare(unsigned char uuid1[16], unsigned char uuid2[16]);

	MULTIMAP_API bool Near(double value1, double value2, double precision);
	MULTIMAP_API bool Near(double value1, double value2);

	MULTIMAP_API std::string TimeFileName(void);
	MULTIMAP_API void Dump(uint16_t* mem, int words);
	MULTIMAP_API int FolderExists(std::string folderName, bool createIfNot = false);
	MULTIMAP_API int FileExists(std::string fileName, bool createIfNot = false);
	MULTIMAP_API unsigned long long FileSize(std::string filePath);
	MULTIMAP_API bool IsPortableFileName(std::string fileName);
	MULTIMAP_API std::string GetPortableFileName(std::string fileName, char substitute='_');
	MULTIMAP_API bool FileNamesEquivalent(std::string filename1, std::string filename2);
	MULTIMAP_API int EmptyFolder(std::string & folderName);
	MULTIMAP_API void NewFileName(std::string & newFileName, const std::string oldFileName, const std::string appendChars = "A");
	MULTIMAP_API std::string NextString(std::string inputString);
	MULTIMAP_API void Redirect( std::string & newFilePath, const std::string newDirectory, const std::string oldFilePath );
	MULTIMAP_API void Retype( std::string & newFilePath, const std::string newType, const std::string oldFilePath );
	MULTIMAP_API void Rename( std::string & newFilePath, const std::string newName, const std::string oldFilePath );
	MULTIMAP_API std::string SerializeName(const std::string fileName, int* previousNumber);
	MULTIMAP_API void EndCapFolder(std::string & folder);
	MULTIMAP_API int GetForeAft(std::string _fileName);

	MULTIMAP_API std::string ToLower(std::string astring);
	MULTIMAP_API std::string ToUpper(std::string astring);
	MULTIMAP_API bool MatchRegex(std::string source, std::string regexPattern);
	MULTIMAP_API std::string ReplaceRegex(std::string source, std::string regexPattern, std::string substitute);
	MULTIMAP_API std::string ReplaceAll(std::string original, std::string replaceThis, std::string withThis);
	MULTIMAP_API std::string ReplaceFirst(std::string original, std::string replaceThis, std::string withThis);
	MULTIMAP_API std::string ReplaceLast(std::string original, std::string replaceThis, std::string withThis);
	MULTIMAP_API bool BeginsWith(std::string astring, std::string pattern, bool caseInsensitive = false);
	MULTIMAP_API bool EndsWith(std::string astring, std::string pattern, bool caseInsensitive = false);
	MULTIMAP_API bool Contains(std::string astring, std::string pattern, bool caseInsensitive = false);
	MULTIMAP_API bool Compare(std::string astring, std::string pattern, bool caseInsensitive);
	MULTIMAP_API std::string CurrentPath(void);
	MULTIMAP_API std::string TemporaryFile (const std::string folder);
	MULTIMAP_API bool RemoveFile(std::string filename);
	MULTIMAP_API int RemoveShapeFileSet(std::string shapefile);

	MULTIMAP_API void Common(char* szCommon, size_t _size, const char* szFileName1, const char* szFileName2);
	MULTIMAP_API void Common(char* szCommon, size_t _size, std::string fileName1, std::string fileName2);

	MULTIMAP_API MMM_IOBTYPE ExtensionType(std::string filename);
	MULTIMAP_API std::string GetCompletePath(std::string fileName, bool tolower = false);
	MULTIMAP_API std::string GetExtension(std::string filename, bool toLower = true);
	MULTIMAP_API std::string GetName(std::string filename, bool toLower = false);
	MULTIMAP_API std::string GetFolder(std::string filename, bool toLower = false);

	MULTIMAP_API void FileSystemSearchByExt(std::vector<std::string>& targetFiles, std::string targetExt, boost::filesystem::directory_iterator& itr);
	MULTIMAP_API void FileSystemSearchByRegex(std::vector<std::string>& targetFiles, std::string regexExp, boost::filesystem::directory_iterator& itr);
	MULTIMAP_API bool FolderFileNameGood(std::string fname, bool msonly=true); 
	MULTIMAP_API bool IsRegularFile(std::string fname);
	MULTIMAP_API bool IsRegularFileName(std::string fname, bool checkPortable=false);

	MULTIMAP_API void FileSystemSearchByExt2(std::vector<std::string>& targetFiles, std::string targetExt, boost::filesystem::directory_iterator& itr, bool recursive=true, bool caseSensitive = true);
	MULTIMAP_API void FileSystemSearchByRegex2(std::vector<std::string>& targetFiles, std::string regexExp, boost::filesystem::directory_iterator& itr, bool recursive=true, bool caseSensitive = true);

	MULTIMAP_API unsigned long FileSystemSearch(std::vector<std::string>& targetFiles, std::string filesPath, bool append = false, bool lidarOnly = false);
	MULTIMAP_API unsigned long FileSystemSearch(std::vector<std::string>& targetFiles, std::string searchFolder, std::string fileMask, bool append = false, bool filesOnly = true, bool lidarOnly = false);
	MULTIMAP_API bool DecomposePath(std::string & parentFolder, std::string & fileName, std::string fullPath);

	MULTIMAP_API long long AvailableSpace(std::string deviceName);

	MULTIMAP_API long double CheckSum(double* pDoubles, size_t count);
	MULTIMAP_API long double CheckSum(float* pFloats, size_t count);
	MULTIMAP_API long long CheckSum(int* pInts, size_t count);
	MULTIMAP_API long long CheckSum(unsigned int* pUInts, size_t count);
	MULTIMAP_API long long CheckSum(long* pLongs, size_t count);
	MULTIMAP_API long long CheckSum(unsigned long* pULongs, size_t count);

	MULTIMAP_API std::string ToString(int number, size_t minDigits=0);

	MULTIMAP_API bool KnownListType(std::string filepath);
	MULTIMAP_API bool KnownLidarType(std::string filepath, bool strict = false);
	MULTIMAP_API bool LoadFiles(std::vector<std::string> & files, std::string filePath, bool append=true);
	MULTIMAP_API bool ReadFiles(std::vector<std::string> & files, std::string filePath, bool append=true);
	MULTIMAP_API size_t FindMatchingFile(std::vector<std::string> & files, std::string fileToMatchByName);
	MULTIMAP_API size_t FindMatchingFile(std::vector<std::string> & files, std::string fileToMatchByName, size_t minimumTailToHeadMatch);
	MULTIMAP_API double Round(double number);	

	MULTIMAP_API bool GetDayOfYear(int *year, int *dayOfYear, std::string date = "", int format=0);
	MULTIMAP_API bool GetDayOfYear(int *dayOfYear, int year, int month, int day);
	MULTIMAP_API bool GetDate(int *month, int *day, int year, int dayOfYear);
	MULTIMAP_API std::string GetDate(int format=0, bool gmt = true);
	MULTIMAP_API bool CropWindowsFromShapeFile(std::string shapeFile, std::string gridLabel, std::vector<MBRect> & cropWindows, std::vector<std::string> & windowNames);

	MULTIMAP_API int GetLastErrorCode(void);
	MULTIMAP_API std::string GetLastErrorMessage(void);
    MULTIMAP_API bool NoErrors(void);
	MULTIMAP_API void ClearError(void);

	MULTIMAP_API void Sleep(unsigned int milliseconds);

	MULTIMAP_API void ParseList(std::vector<int> & ints, std::string commaSeparatedString);
	MULTIMAP_API void ParseList(std::vector<double> & doubles, std::string commaSeparatedString);

    MULTIMAP_API void ToStrings(std::vector<std::string> & strings, std::string inputString, std::string breakString);
	MULTIMAP_API void ToStrings(std::vector<std::string> & strings, char* zeroTerminatedChars, size_t sizeOfZTC);
	MULTIMAP_API void FromStrings(char* zeroTerminatedChars[], size_t *sizeOfZTC, std::vector<std::string> strings);
private:
	boost::system::error_code last_error;
};

