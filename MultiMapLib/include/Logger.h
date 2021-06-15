#pragma once
#include "MultiMap.h"
#include "MFUtils.h"

DISABLE_WARNINGS
#include <time.h>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <stdarg.h>

#include <boost/function.hpp>
ENABLE_WARNINGS

// if this is changed you must also update Logger::GetLevel(int level)
typedef enum {ALWAYS=0,DBG=1,INFO=2,WARNING=4,ERR=8,FATAL=16,OFF=1024} Level;

class Logger {
public:
	Logger::~Logger(void);

	// Implement the logic here to instantiate the class for the first time by validating the
	// member pointer. If member pointer is already pointing to some valid memory it means
	// that the first object is created and it should not allow for the next instantiation
	// so simply return the member pointer without calling its ctor.
	static Logger* Logger::Instance(bool theaded=false, Logger* customLogger = NULL);
	static void Logger::SetLoggerInstance(Logger* customLogger);
	static bool Logger::Exists();
	static Level Logger::GetLevel(int _level);
	static Level Logger::GetLevel(char* _level);

	virtual void Logger::Dereference(void);
	virtual size_t Logger::ReferenceCount(void);

	virtual const char* GetSeverity(Level level);
	virtual void Logger::SetLevel(Level level);
	virtual void Logger::SetLevel(int level);
	virtual void Logger::SetLogFile(std::string logfile);
	virtual void Logger::LogFileClose(void);

	virtual Level Logger::GetLevel();
	virtual bool Logger::IsTrace();
	virtual void Logger::SetTrace(bool trace);

	virtual void Logger::Separator(FILE * pFile);
	virtual const char* Logger::GetSeparator(void);

	virtual void Logger::Log(Level level, std::string format, ...);
	virtual void Logger::Log(unsigned long mask, Level level, std::string format, ...);

	virtual FILE * Logger::AboutLogOpen(std::string mode = "a");
	virtual void Logger::AboutLogClose(void);

	virtual FILE * Logger::ErrorLogOpen(std::string mode = "a");
	virtual void Logger::ErrorLogAppend(std::string format, ...);
	virtual void Logger::ErrorLogClose(void);

	std::string Logger::GetLogFolderName(void);
	void Logger::SetToScreen(bool log=true);
	bool Logger::GetToScreen(void);
	void Logger::SetToFile(bool log=true);
	bool Logger::GetToFile(void);

	bool verbose;
	int timeMode;

	bool logToFile;
	bool logToScreen;
	std::string logFolder;

	static const Level INFO = INFO;
	static const Level WARNING = WARNING;
	static const Level ERR = ERR;
	static const Level DBG = DBG;
	static const Level OFF = OFF;

	// These would normally be private but TLogger needs them

	FILE * pAboutFile;
	FILE * pErrorFile;
	FILE * pStdLogFile;
	Level level;

protected:
	// Default ctor, copy-ctor and assignment operators should be as private so that nobody
	// from outside can call those functions and instantiate it
	Logger(void);
	Logger(const Logger&);
	Logger& operator= (const Logger&);
	
	bool Logger::LogFolderDefined(void);


	MFUtils mfUtils;

	bool   threadLock;
	char buffer[4096];
	char buffer2[4096];
	clock_t startTime;
	bool trace;

	static void Lock(void);
	static void Unlock(void);

	static Logger* pInstance;
	size_t referenceCount;
};
