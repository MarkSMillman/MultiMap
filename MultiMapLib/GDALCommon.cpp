#include "MultiMap.h"
#include "GDALCommon.h"
#include "MBRect.h"

DISABLE_WARNINGS
#include "cpl_error.h"
#include "cpl_string.h"
#include "gdal.h"
ENABLE_WARNINGS

MULTIMAP_API GDALCommon::~GDALCommon() {
}
MULTIMAP_API GDALCommon::GDALCommon() {
	defaultErrorHandler = NULL;
    pfnProgress = NULL;
	pProgressArg = NULL;
	verbose = false;

	breakIfErrorType = CE_Fatal;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////   ERROR HANDLERS   /////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
MULTIMAP_API void GDALCommon::ErrorReset() {
	CPLErrorReset ();
}
MULTIMAP_API void GDALCommon::SetInfo(int errCode, const char* message, bool appendLastErrorMessage) {
	std::string totalMessage = std::string(message);
	if ( appendLastErrorMessage ) {
		totalMessage.append(" : ");
		totalMessage.append(std::string(CPLGetLastErrorMsg()));
	}
	CPLError(CE_None, errCode, "%s", totalMessage.c_str()); 
}
MULTIMAP_API void GDALCommon::SetWarning(int errCode, const char* message, bool appendLastErrorMessage) {
	std::string totalMessage = std::string(message);
	if ( appendLastErrorMessage ) {
		totalMessage.append(" : ");
		totalMessage.append(std::string(CPLGetLastErrorMsg()));
	}
	CPLError(CE_Warning, errCode, "%s", totalMessage.c_str()); 
}
MULTIMAP_API void GDALCommon::SetError(int errCode, const char* message, bool appendLastErrorMessage) {
	std::string totalMessage = std::string(message);
	if ( appendLastErrorMessage ) {
		totalMessage.append(" : ");
		totalMessage.append(std::string(CPLGetLastErrorMsg()));
	}
	CPLError(CE_Failure, errCode, "%s", totalMessage.c_str()); 
}
MULTIMAP_API void GDALCommon::SetDebug(int errCode, const char* message, bool appendLastErrorMessage) {
	std::string totalMessage = std::string(message);
	if ( appendLastErrorMessage ) {
		totalMessage.append(" : ");
		totalMessage.append(std::string(CPLGetLastErrorMsg()));
	}
	CPLError(CE_Debug, errCode, "%s", totalMessage.c_str()); 
}
MULTIMAP_API void GDALCommon::SetFatal(int errCode, const char* message, bool appendLastErrorMessage) {
	std::string totalMessage = std::string(message);
	if ( appendLastErrorMessage ) {
		totalMessage.append(" : ");
		totalMessage.append(std::string(CPLGetLastErrorMsg()));
	}
	CPLError(CE_Fatal, errCode, "%s", totalMessage.c_str()); 
}
MULTIMAP_API int GDALCommon::GetLastError(const char** message ) {
	int errCode = CPLGetLastErrorNo();
	if ( message ) {
		if ( errCode == CE_None ) {
			*message = NULL;
		} else {
			*message = CPLGetLastErrorMsg();
		}
	}
	return errCode;
}
MULTIMAP_API CPLErr GDALCommon::GetLastErrorType(void) {
	return CPLGetLastErrorType();
}
MULTIMAP_API void GDALCommon::SetDefaultErrorHandler(void) {
	defaultErrorHandler = CPLSetErrorHandler(defaultErrorHandler);
}

MULTIMAP_API void GDALCommon::SetApplicationErrorHandler(void) {
	CPLSetErrorHandler(CPLQuietErrorHandler);
}

MULTIMAP_API void GDALCommon::setMBR(MBRect & mbr, OGREnvelope & envelope) {
	mbr.minX = envelope.MinX;
	mbr.maxX = envelope.MaxX;
	mbr.minY = envelope.MinY;
	mbr.maxY = envelope.MaxY;
}

/**
 * @return NULL indicates success, else points to a message which must be freed with CPLFree(message) by caller
**/
MULTIMAP_API STATIC char* GDALCommon::CheckExtensionConsistency(const char* pszDestFilename, const char* pszDriverName) {
	char* errorMessage = NULL;

	char* pszDestExtension = CPLStrdup(CPLGetExtension(pszDestFilename));
	if (pszDestExtension[0] != '\0')
	{
		int nDriverCount = GDALGetDriverCount();
		CPLString osConflictingDriverList;
		for (int i = 0; i<nDriverCount; i++)
		{
			GDALDriverH hDriver = GDALGetDriver(i);
			const char* pszDriverExtension =
				GDALGetMetadataItem(hDriver, GDAL_DMD_EXTENSION, NULL);
#pragma warning (suppress:4996)
			if (pszDriverExtension && EQUAL(pszDestExtension, pszDriverExtension))
			{
				if (GDALGetDriverByName(pszDriverName) != hDriver)
				{
					if (osConflictingDriverList.size())
						osConflictingDriverList += ", ";
					osConflictingDriverList += GDALGetDriverShortName(hDriver);
				}
				else
				{
					/* If the request driver allows the used extension, then */
					/* just stop iterating now */
					osConflictingDriverList = "";
					break;
				}
			}
		}
		if (osConflictingDriverList.size())
		{
			errorMessage = (char*)CPLCalloc(1024, 1);
			sprintf(errorMessage,
				"Warning: The target file has a '%s' extension, which is normally used by the %s driver%s, "
				"but the requested output driver is %s.",
				pszDestExtension,
				osConflictingDriverList.c_str(),
				strchr(osConflictingDriverList.c_str(), ',') ? "s" : "",pszDriverName);
		}
	}

	CPLFree(pszDestExtension);

	return errorMessage;
}