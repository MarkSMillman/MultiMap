#pragma once
#include "MultiMap.h"
#include "ColorTable.h"
#include "ogr_core.h"
#include "cpl_error.h"
#include "cpl_progress.h"

class MBRect;
class GDALCommon {
public:
	MULTIMAP_API ~GDALCommon();
	MULTIMAP_API GDALCommon();

	MULTIMAP_API void ErrorReset();
	MULTIMAP_API int GetLastError(const char** message = 0);
	MULTIMAP_API CPLErr GetLastErrorType(void);
	MULTIMAP_API void SetInfo(int errCode, const char* message, bool appendLastErrorMessage = true);
	MULTIMAP_API void SetWarning(int errCode, const char* message, bool appendLastErrorMessage = true);
	MULTIMAP_API void SetError(int errCode, const char* message, bool appendLastErrorMessage = true);
	MULTIMAP_API void SetDebug(int errCode, const char* message, bool appendLastErrorMessage = true);
	MULTIMAP_API void SetFatal(int errCode, const char* message, bool appendLastErrorMessage = true);

	MULTIMAP_API void SetDefaultErrorHandler(void);
	MULTIMAP_API void SetApplicationErrorHandler(void);

	MULTIMAP_API void setMBR(MBRect & range, OGREnvelope & envelope);

	MULTIMAP_API static char* CheckExtensionConsistency(const char* pszDestFilename, const char* pszDriverName);

	bool verbose;

protected:
	CPLErrorHandler defaultErrorHandler;
	CPLErr breakIfErrorType;

	GDALProgressFunc pfnProgress;
	void * pProgressArg;
};