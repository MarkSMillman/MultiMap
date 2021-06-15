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
#include "Logger.h"
#include "MFUtils.h"
#include <boost/thread.hpp>

static boost::mutex    logMutex;

/**
* TODO: REPLACE WITH 
#include <boost/log/trivial.hpp> 
**/
static const char* SEPARATOR =  "______________________________________________________________________\n";

Logger* Logger::pInstance(NULL);

Logger::~Logger(){
	ErrorLogClose();
	AboutLogClose();
	LogFileClose();
}

Logger::Logger(const Logger& other) {
	*this = other;
}

Logger& Logger::operator=(const Logger& other) {
	pStdLogFile = other.pStdLogFile;
	pAboutFile = other.pAboutFile;
	pErrorFile = other.pErrorFile;
	verbose = other.verbose;
	level = other.level;
	trace = other.trace;
	logToFile = other.logToFile;
	timeMode = other.timeMode;
	threadLock = other.threadLock;

	return *this;
}

STATIC bool Logger::Exists() {
	bool exists = false;
		if ( pInstance != NULL ) {
			exists = true;
		}
	return exists;
}

STATIC void Logger::SetLoggerInstance(Logger* customLogger) {
	pInstance = customLogger;
}

STATIC Logger* Logger::Instance (bool threaded, Logger* customLogger) {
	if ( pInstance == NULL ) {
		if ( customLogger == NULL ) {
			pInstance = new Logger();
		} else {
			pInstance = customLogger;
		}
	}

	if ( threaded ) {
		// threadLock is turned on the first time it is requested
		// once requested it cannot be turned off
		pInstance->threadLock = true;
	}
	
	pInstance->referenceCount++;

	return pInstance;
}

void Logger::Dereference(void) {
	referenceCount--;
}
size_t Logger::ReferenceCount(void) {
	return referenceCount;
}
Logger::Logger(void){
	startTime = clock();
	//stream = &std::cout;
	pStdLogFile = NULL;
	pAboutFile = NULL;
	pErrorFile = NULL;
	verbose = false;
	timeMode = 3;
	level = ERR;
	trace = false;
	logToFile = 0;
}

void Logger::SetTrace(bool trace){
	if (threadLock) Lock();
	this->trace = trace;
	if (threadLock) Unlock();
}

bool Logger::IsTrace(){
	return trace;
}

void Logger::Log (Level _level, std::string format, ...){
	if (threadLock) Lock();
	va_list argp;
	va_start(argp,format);
	if ( _level!=OFF && (_level == ALWAYS || level == DBG || _level >= level) ) {
		if (GetToFile()) {
			va_list argp;
			va_start(argp,format);
			fprintf(pStdLogFile, (GetSeverity(_level) + mfUtils.TimeIs(timeMode) + " ").c_str());
			vfprintf(pStdLogFile,format.c_str(),argp);
			fprintf(pStdLogFile,"\n");
			fflush(pStdLogFile);
			va_end(argp);
		} 
		if (GetToScreen()) {
			vsprintf(buffer,format.c_str(), argp);
			va_end(argp);
			if ( _level < ERR ) {
			    std::cout << GetSeverity(_level) << mfUtils.TimeIs(timeMode) << " " << buffer << std::endl;
			} else {
			    std::cerr << GetSeverity(_level) << mfUtils.TimeIs(timeMode) << " " << buffer << std::endl;
			}
		}
	}
	if (threadLock) Unlock();
}

void Logger::Log (unsigned long _mask, Level _level, std::string format, ...){
	if (threadLock) Lock();
	va_list argp;
	va_start(argp,format);
	if ( _level!=OFF && ( _level == ALWAYS || level == DBG || _level >= level ) ) {
		if (GetToFile()) {
			va_list argp;
			va_start(argp,format);
			fprintf(pStdLogFile, (GetSeverity(_level) + mfUtils.TimeIs(timeMode) + " ").c_str());
			vfprintf(pStdLogFile,format.c_str(),argp);
			fprintf(pStdLogFile,"\n");
			fflush(pStdLogFile);
			va_end(argp);
		} 
		if (GetToScreen()) {
			vsprintf(buffer,format.c_str(), argp);
			va_end(argp);
			if ( _level < ERR ) {
			    std::cout << GetSeverity(_level) << mfUtils.TimeIs(timeMode) << " " << buffer << std::endl;
			} else {
			    std::cerr << GetSeverity(_level) << mfUtils.TimeIs(timeMode) << " " << buffer << std::endl;
			}
		}
	}
	if (threadLock) Unlock();
}

void Logger::LogHexDump(unsigned long _mask, Level _level, unsigned char* location, int bytes, int mark) {
	if (threadLock) Lock();
	if ( _level!=OFF && ( _level == ALWAYS || level == DBG || _level >= level ) ) {
		std::string hexDump = Logger::LogHexDumpToString(location,bytes,mark);
		Log(_mask,_level,hexDump);
	}
	if (threadLock) Unlock();
}

STATIC std::string Logger::LogHexDumpToString(unsigned char* location, int bytes, int mark) {
	int lines = (bytes)/16;
	int leader = 18;
	int trailer = 2;
	int bytesPrinted = 0;

	char* buffer = new char[(bytes*5)+((lines+1)*(leader+trailer))];
	char* pBuffer = buffer;
	char* offset = 0;

	sprintf(pBuffer,"%d Bytes starting at address %p\n",bytes,(unsigned int)location+(unsigned int)offset);
	pBuffer += strlen(buffer);
	sprintf(pBuffer,"  Offset  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
	pBuffer = buffer + strlen(buffer);
	unsigned char byteValue;
	unsigned char* pByteValue;

	for ( size_t l=0; l<lines; l++ ) {
		sprintf(pBuffer,"%08x",offset);
		pBuffer += 8;
		for ( size_t b=0; b<16 && (l*16+b<bytes); b++ ) {
			pByteValue = location+(unsigned long)offset++;
			byteValue = *pByteValue;
			if ( bytesPrinted == mark ) {
				sprintf(pBuffer,"(%02x",byteValue);
				if ( (b+1)%16==0 ) {
					sprintf(pBuffer+3,")");
					mark = -1;
					pBuffer++;
				}
			} else if ( bytesPrinted == mark+1 ) {
				sprintf(pBuffer,")%02x",byteValue);
			} else {
				sprintf(pBuffer," %02x",byteValue);
			}
			pBuffer += 3;

			bytesPrinted++;
		}
		sprintf(pBuffer,"\n");
		pBuffer++;
	}

	//std::cout << buffer << std::endl;
	return std::string(buffer);
}

void Logger::SetLogFile(std::string logfile){
	if (threadLock) Lock();
	logToFile = true;
	if ( pStdLogFile == NULL ) {
	    pStdLogFile = fopen(logfile.c_str(), "w");
	}
	if (threadLock) Unlock();
}

void Logger::LogFileClose(void) {
	if (threadLock ) Lock();
	if ( pStdLogFile != NULL ) {
		fclose(pStdLogFile);
		pStdLogFile = NULL;
	}
	if (threadLock) Unlock();
}
void Logger::SetLevel(Level lvl){
	if (threadLock) Lock();
	level = lvl;
	if (threadLock) Unlock();
}
void Logger::SetLevel(int lvl){
	if (threadLock) Lock();
	level = (Level)lvl;
	if (threadLock) Unlock();
}
Level Logger::GetLevel(){
	return level;
}

STATIC Level Logger::GetLevel(char* _level) {
	Level level;
	if ( strncmp(_level,"ALW",3) == 0 ) {
		level = ALWAYS;
	} else if ( strncmp(_level,"DEB",3) == 0 ) {
		level = DBG;
	} else if ( strncmp(_level,"DBG",3) == 0 ) {
		level = DBG;
	} else if ( strncmp(_level,"INF",3) == 0 ) {
		level = INFO;
	} else if ( strncmp(_level,"WAR",3) == 0 ) {
		level = WARNING;
	} else if ( strncmp(_level,"ERR",3) == 0 ) {
		level = ERR;
	} else if ( strncmp(_level,"FAT",3) == 0 ) {
		level = FATAL;
	} else if ( strncmp(_level,"OFF",3) == 0 ) {
		level = OFF;
	} else {
		level = ERR;
	}
	return level;
}

STATIC Level Logger::GetLevel(int _level)
{
	Level level;

	switch (_level)
	{
	case ALWAYS:
		level = ALWAYS;
		break;
	case INFO:
		level = INFO;
		break;
	case WARNING:
		level = WARNING;
		break;
	case ERR:
		level = ERR;
		break;
	case FATAL:
		level = FATAL;
		break;
	case DBG:
		level = DBG;
		break;
	case OFF:
		level = OFF;
		break;
	default:
		level = ERR;
		break;
	};

	return level;
}

static const char* szALWAYS   = "        ";
static const char* szDEBUG    = "DEBUG   ";
static const char* szINFO     = "INFO    ";
static const char* szWARNING  = "WARNING ";
static const char* szERROR    = "ERROR   ";
static const char* szFATAL    = "FATAL   ";
static const char* szOFF      = "OFF     ";
static const char* szNONE     = "        ";

const char* Logger::GetSeverity(Level level){
	if ( level == INFO )
		return szINFO;
	else if ( level == WARNING )
		return szWARNING;
	else if ( level == ERR )
		return szERROR;
	else if ( level == FATAL )
		return szFATAL;
	else if ( level == DBG )
		return szDEBUG;
	else if ( level == ALWAYS )
		return szALWAYS;
	else if ( level == OFF )
		return szOFF;
	else 
		return szNONE;
}

const char* Logger::GetSeparator(void) {
	return SEPARATOR;
}

void Logger::Separator(FILE * pFile) {
	fprintf(pFile,"%s\n",*SEPARATOR);
}

FILE * Logger::ErrorLogOpen(std::string mode){
	if (threadLock ) Lock();
	if ( pErrorFile == NULL && LogFolderDefined() ) {
		if ( mfUtils.FolderExists(GetLogFolderName()) == 0 ) {
			std::string errorLogPath = GetLogFolderName()+FILE_SEP+"ERROR.log";
			mfUtils.FileExists(errorLogPath,true);
			pErrorFile = fopen(errorLogPath.c_str(),mode.c_str());
		}
	}
	if (threadLock) Unlock();
	return pErrorFile;
}

void Logger::ErrorLogAppend(std::string format, ...){
	if (threadLock ) Lock();
	if ( pErrorFile == NULL ) {
		if ( ErrorLogOpen("a") ) {
			va_list argp;
			va_start(argp,format);
			vfprintf(pErrorFile,format.c_str(),argp);
			fflush(pErrorFile);
			va_end(argp);
		}
	}
	if (threadLock) Unlock();
}

void Logger::ErrorLogClose(){
	if (threadLock ) Lock();
	if ( pErrorFile != NULL ) {
		fclose(pErrorFile);
		pErrorFile = NULL;
	}
	if (threadLock) Unlock();
}

FILE * Logger::AboutLogOpen(std::string mode){
	if (threadLock ) Lock();
	if ( pAboutFile == NULL && LogFolderDefined() ) {
		if ( mfUtils.FolderExists(GetLogFolderName()) == 0) {
			std::string aboutLogPath = GetLogFolderName()+FILE_SEP+"ABOUT.log";
			mfUtils.FileExists(aboutLogPath,true);
			pAboutFile = fopen(aboutLogPath.c_str(),mode.c_str());
		}
	}
	if (threadLock) Unlock();
	return pAboutFile;
}

void Logger::AboutLogClose(){
	if (threadLock ) Lock();
	if ( pAboutFile != NULL ) {
		fclose(pAboutFile);
		pAboutFile = NULL;
	}
	if (threadLock) Unlock();
}

PRIVATE std::string Logger::GetLogFolderName(void) {
	return logFolder;
}
PRIVATE bool Logger::LogFolderDefined(void) {
	GetLogFolderName();
	return logFolder.length()>0?true:false;
}

void Logger::SetToFile(bool log) {
	logToFile = log;
}
bool Logger::GetToFile(void) {
	return logToFile;
}
void Logger::SetToScreen(bool log) {
	logToScreen = log;
}
bool Logger::GetToScreen(void) {
	return logToScreen;
}

PRIVATE STATIC void Logger::Lock(void) {
	logMutex.lock();
}
PRIVATE STATIC void Logger::Unlock(void) {
	logMutex.unlock();
}