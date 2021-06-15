#pragma once
#include "MultiMap.h"
#include "GenericType.h"

DISABLE_WARNINGS
#include "oci.h"
ENABLE_WARNINGS

class OCITools {
public:
	MULTIMAP_API ~OCITools(void);
	MULTIMAP_API OCITools(void);

	MULTIMAP_API OCIError* connect(std::string instance, std::string username, std::string password);
	MULTIMAP_API OCIError* select(std::vector<std::string> columns, std::vector<GenericType> values, std::string select);
	MULTIMAP_API int createTableLike(std::string newTableName, std::string existingTableName);
	MULTIMAP_API int createTableLike(std::string newTableName, std::string existingTableName, std::vector<std::string> & omitColumns);

private:
	void checkerr0(void *handle, ub4 htype, sword status);
	char lastErrorMsg[4096];
	int lastErrorNo;

OCIEnv     *myenvhp;    /* the environment handle */
OCIServer  *mysrvhp;    /* the server handle */
OCIError   *myerrhp;    /* the error handle */
OCISession *myusrhp;    /* user session handle */
OCISvcCtx  *mysvchp;    /* the  service handle */
};