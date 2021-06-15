#include "MultiMap.h"
#include "Runner.h"
#include "Logger.h"

DISABLE_WARNINGS
#include <tchar.h>
#include <stdio.h> 
#include <strsafe.h>
#include <iostream>
#include <fstream>
#if defined (_WIN32)  || defined(_WIN64)
#include <windows.h>
#include <AclAPI.h>
#endif
ENABLE_WARNINGS

Runner::~Runner(void) {
	Close();
}
Runner::Runner(void) {
	reason = NULL;

	memset(&startupInformation,0,sizeof(startupInformation));
	startupInformation.cb = sizeof(startupInformation);
	memset(&processInformation,0,sizeof(processInformation));

	g_hChildStd_OUT_Rd = NULL;
	g_hChildStd_OUT_Wr = NULL;

	timeout = INFINITE;
	pipeBufferSize = 0;

	MFUtils mfUtils;
	mfUtils.UUID(guid);
}
Runner::Runner(std::string _commandLine) {
	commandLine = _commandLine;
	memset(&startupInformation,0,sizeof(startupInformation));
	startupInformation.cb = sizeof(startupInformation);
	memset(&processInformation,0,sizeof(processInformation));
}
bool Runner::Run(void) {
	bool started = false;
	if ( commandLine.length() > 0 ) {
		started = Run(commandLine);
	}
	return started;
}
bool Runner::Run(std::string _commandLine) {
	bool started = false;
	if ( CreatePipes() ) {
		commandLine = _commandLine;
		LPTSTR pCommandLine = const_cast<char*>(commandLine.c_str());

		ZeroMemory( &processInformation, sizeof(PROCESS_INFORMATION) );

		// Set up members of the STARTUPINFO structure. 
		// This structure specifies the STDIN and STDOUT handles for redirection.

		ZeroMemory( &startupInformation, sizeof(STARTUPINFO) );
		startupInformation.cb = sizeof(STARTUPINFO); 
		startupInformation.hStdError = g_hChildStd_OUT_Wr;
		startupInformation.hStdOutput = g_hChildStd_OUT_Wr;
		startupInformation.dwFlags |= STARTF_USESTDHANDLES;

		// Create the child process. 
		BOOL bSuccess = FALSE;     
		bSuccess = CreateProcess(NULL, 
			pCommandLine,     // command line 
			NULL,          // process security attributes 
			NULL,          // primary thread security attributes 
			TRUE,          // handles are inherited 
			0,             // creation flags 
			NULL,          // use parent's environment 
			NULL,          // use parent's current directory 
			&startupInformation,  // STARTUPINFO pointer 
			&processInformation);  // receives PROCESS_INFORMATION 

		started = bSuccess==0?false:true;
	}
	return started;
}
void Runner::SetTimeoutMinutes( unsigned long minutes) {
	timeout = minutes * 60000;
}
unsigned long Runner::Wait(void) {
	unsigned long result = WaitForSingleObject( processInformation.hProcess, timeout);
	DWORD exitCode = 0;
	BOOL s = GetExitCodeProcess(processInformation.hProcess,&exitCode);
	return exitCode;
}
void Runner::Close(void) {
	ClosePipes();
	if ( processInformation.hProcess ) {
		CloseHandle(processInformation.hProcess);
		CloseHandle(processInformation.hThread);
	}
	memset(&processInformation,0,sizeof(processInformation));
}

/**
 * WARNING: The pipeBufferSize may default to as little as 4K bytes. 
 * If the buffer size is exceeded the process will halt, waiting on 
 * room to be made in the pipe.  This happens when the Wait request
 * times out.  After which the process continues out of sequence 
 * with the wait.
 * UtilLauncher, which uses Runner sets the pipeBufferSize to
 * a much bigger number (262144 as of this note).
**/
PRIVATE bool Runner::CreatePipes(void) {
	reason = NULL;
	// Set the bInheritHandle flag so pipe handles are inherited. 
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
	saAttr.bInheritHandle = TRUE; 
	saAttr.lpSecurityDescriptor = NULL; 

	if ( ! CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, pipeBufferSize) ) 
		reason = (char*)RNR_OUTPUT_FAILURE;

	// Ensure the read handle to the pipe for STDOUT is not inherited.
	if ( ! SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT,  pipeBufferSize) )
		reason = (char*)RNR_OUTPUT_FAILURE;

	return reason==NULL?true:false;
}

#define BUFSIZE 1024
bool Runner::CopyOutputPipes(std::string filePath, bool append) {
	bool status = false;
	if ( filePath.length() > 0 ) {
		FILE* file = NULL;
		if ( append ) {
			file = fopen(filePath.c_str(),"w+a");
		} else {
			file = fopen(filePath.c_str(),"w");
		}
		status = CopyOutputPipes(file);
		fclose(file);
	}
	return status;
}
bool Runner::CopyOutputPipes(FILE* hOut) {
	bool aok = true;
	DWORD dwRead = 0; 
	DWORD dwWritten = 0; 
	CHAR chBuf[BUFSIZE]; 
	BOOL bSuccess = FALSE;
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	if ( startupInformation.hStdOutput ) {
		DWORD fsize = GetFileSize(g_hChildStd_OUT_Rd,NULL);
		if ( fsize > 0 ) {
			sprintf(chBuf,"Output for Commandline: %s\n",commandLine.c_str());
			bSuccess = WriteFile(hStdOut, chBuf, (DWORD)strlen(chBuf), &dwWritten, NULL); 
			do { 
				bSuccess = ReadFile( g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
				if( ! bSuccess || dwRead == 0 ) break; 
				if ( hOut ) {
					DWORD read = dwRead;
					for ( size_t pi=0,po=0; pi<dwRead; pi++ ) {
						if ( chBuf[pi] != '\r' ) {
							chBuf[po++] = chBuf[pi];
						}else {
							read--;
						}
					}
					fwrite(chBuf,1,read,hOut);
				} else {
					bSuccess = WriteFile(hStdOut, chBuf, dwRead, &dwWritten, NULL);
				}
				if (! bSuccess ) break; 
			} while (dwRead == BUFSIZE);
			if ( hOut ) {
				fwrite("\n",1,1,hOut);
			} else {
				strcpy(chBuf,"\n");
				WriteFile(hStdOut, chBuf, (DWORD)strlen(chBuf), &dwWritten, NULL);
			}
		}
	}
	return aok;
}
bool Runner::CopyOutputPipes(std::vector<std::string> & output) {
	bool aok = true;
	DWORD dwRead = 0; 
	DWORD dwWritten = 0; 
	CHAR chBuf[BUFSIZE+1]; 
	BOOL bSuccess = FALSE;
	std::string lines;
	std::string line;
	DWORD pos = 0;
	if ( startupInformation.hStdOutput ) {
		DWORD fsize = GetFileSize(g_hChildStd_OUT_Rd,NULL);
		if ( fsize > 0 ) {
			sprintf(chBuf,"Output for Commandline: %s\n",commandLine.c_str());
			output.push_back(std::string(chBuf));
			do { 
				bSuccess = ReadFile( g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
				if( ! bSuccess || dwRead == 0 ) break; 
				int offset=0;
				pos = 0;
				while ( pos < dwRead ) {
					chBuf[dwRead] = 0;
					char* n = strchr(&chBuf[pos],'\n');
					if ( n != NULL ) {
						char* r = strchr(&chBuf[pos],'\r');
						if ( r!=NULL ) {
							n = r;
							offset=1;
						} else {
							offset=0;
						}
						int npos = static_cast<int>(n - chBuf);
						int len = npos - pos + 1;
						line += std::string(&chBuf[pos],len-1);
						output.push_back(line);
						line.clear();
						pos = pos + len + offset;
					} else {
						int len = dwRead - pos;
						line += std::string(&chBuf[pos],len);
						pos = pos + len + offset;
					}
				}

				if (! bSuccess ) break; 
			} while (dwRead == BUFSIZE);
			if ( line.length() > 0 ) {
				output.push_back(line);
			}
		}
	}
	return aok;
}
PRIVATE void Runner::ClosePipes(void) {

	if (g_hChildStd_OUT_Rd ) {
		CloseHandle(g_hChildStd_OUT_Rd);
		g_hChildStd_OUT_Rd = NULL;
	}
	if ( g_hChildStd_OUT_Wr ) {
		CloseHandle(g_hChildStd_OUT_Wr);
		g_hChildStd_OUT_Wr = NULL;
	}

}