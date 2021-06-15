#pragma once
#include "MultiMap.h"

DISABLE_WARNINGS
#if defined (_WIN32)  || defined(_WIN64)
#include <windows.h>
#endif
#include <stdio.h>
#include <tchar.h>
ENABLE_WARNINGS

static const char* RNR_INPUT_FAILURE = "Could not create pipe for input file";
static const char* RNR_OUTPUT_FAILURE = "Could not create pipe for output file";
static const char* RNR_ERROR_FAILURE = "Could not create pipe for error file";

class Runner {
public:
	~Runner(void);
	Runner(void);
	Runner(std::string _commandLine);

	bool Run(void);
	bool Run(std::string _commandLine);
	void SetTimeoutMinutes( unsigned long minutes);
	unsigned long Wait(void);
	bool CopyOutputPipes(FILE* hOut = NULL);
	bool CopyOutputPipes(std::vector<std::string> & output);
	bool CopyOutputPipes(std::string filePath, bool append = false);
	void ClosePipes(void);

	void Close(void);

	unsigned char                 guid[16];
	std::string                commandLine;
	std::string                   stdInput;
#if defined (_WIN32)  || defined(_WIN64)
	PROCESS_INFORMATION processInformation;
	STARTUPINFO         startupInformation;
#endif
	unsigned long                  timeout;
	unsigned long           pipeBufferSize; // 0 uses system default

	char* reason;
private:
	bool Runner::CreatePipes(void);
#if defined (_WIN32)  || defined(_WIN64)
	SECURITY_ATTRIBUTES saAttr;

	HANDLE g_hChildStd_OUT_Rd;
	HANDLE g_hChildStd_OUT_Wr;
#endif
};