#include "MultiMap.h"
#include "OCITools.h"
#include "oci.h"
#include "ociap.h"
#pragma warning ( disable : 4267 )
#pragma warning ( disable : 4101 )
#define checkerr(errhp, status) checkerr0((errhp), OCI_HTYPE_ERROR, (status))
#define checkenv(envhp, status) checkerr0((envhp), OCI_HTYPE_ENV, (status))
PRIVATE void OCITools::checkerr0(void *handle, ub4 htype, sword status)
{

	switch (status)
	{
	case OCI_SUCCESS:
		break;
	case OCI_SUCCESS_WITH_INFO:
		strcpy(lastErrorMsg,"Error - OCI_SUCCESS_WITH_INFO");
		break;
	case OCI_NEED_DATA:
		strcpy(lastErrorMsg,"Error - OCI_NEED_DATA");
		break;
	case OCI_NO_DATA:
		strcpy(lastErrorMsg,"Error - OCI_NODATA");
		break;
	case OCI_ERROR:
		strcpy(lastErrorMsg,"Error - OCI_ERROR");
		if (handle) {
			text errbuf[2048];
			sb4 errcode;
			(void) OCIErrorGet(handle, 1, (text *) NULL, &errcode, errbuf, (ub4) sizeof(errbuf), htype);
			sprintf(lastErrorMsg,"%s Error - %.*s",lastErrorMsg, sizeof(errbuf), errbuf);
		} else {
			strcat(lastErrorMsg," NULL handle");            
			strcat(lastErrorMsg," Unable to extract detailed diagnostic information");
		}
		break;
	case OCI_INVALID_HANDLE:
		strcpy(lastErrorMsg,"Error - OCI_INVALID_HANDLE");
		break;
	case OCI_STILL_EXECUTING:
		strcpy(lastErrorMsg,"Error - OCI_STILL_EXECUTE");
		break;
	case OCI_CONTINUE:
		strcpy(lastErrorMsg,"Error - OCI_CONTINUE");
		break;
	default:
		break;
	}
}
MULTIMAP_API OCITools::~OCITools(void) {



}
MULTIMAP_API OCITools::OCITools(void) {
	myenvhp = NULL;    /* the environment handle */
	mysrvhp = NULL;    /* the server handle */
	myerrhp = NULL;    /* the error handle */
	myusrhp = NULL;    /* user session handle */
	mysvchp = NULL;    /* the  service handle */
}
MULTIMAP_API OCIError* OCITools::connect(std::string instance, std::string username, std::string password) {

	/* initialize the mode to be the threaded and object environment */
	(void) OCIEnvCreate(&myenvhp, OCI_THREADED|OCI_OBJECT, (void*)0, 0, 0, 0, (size_t) 0, (void**)0);

	/* allocate a server handle */
	(void) OCIHandleAlloc ((void  *)myenvhp, (void  **)&mysrvhp, OCI_HTYPE_SERVER, 0, (void  **) 0);

	/* allocate an error handle */
	(void) OCIHandleAlloc ((void  *)myenvhp, (void  **)&myerrhp, OCI_HTYPE_ERROR, 0, (void  **) 0);

	/* create a server context */
	(void) OCIServerAttach (mysrvhp, myerrhp, (text *)instance.c_str(), (sb4)instance.size(), OCI_DEFAULT);

	/* allocate a service handle */
	(void) OCIHandleAlloc ((void  *)myenvhp, (void  **)&mysvchp, OCI_HTYPE_SVCCTX, 0, (void  **) 0);

	/* set the server attribute in the service context handle*/
	(void) OCIAttrSet ((void  *)mysvchp, OCI_HTYPE_SVCCTX, (void  *)mysrvhp, (ub4) 0, OCI_ATTR_SERVER, myerrhp);

	/* allocate a user session handle */
	(void) OCIHandleAlloc ((void  *)myenvhp, (void  **)&myusrhp,OCI_HTYPE_SESSION, 0, (void  **) 0);

	/* set user name attribute in user session handle */
	(void) OCIAttrSet ((void  *)myusrhp, OCI_HTYPE_SESSION, (void  *)username.c_str(), (ub4)username.size(), OCI_ATTR_USERNAME, myerrhp);

	/* set password attribute in user session handle */
	(void) OCIAttrSet ((void  *)myusrhp, OCI_HTYPE_SESSION,	(void  *)password.c_str(), (ub4)password.size(), OCI_ATTR_PASSWORD, myerrhp);

	(void) OCISessionBegin (mysvchp, myerrhp, myusrhp, OCI_CRED_RDBMS, OCI_DEFAULT);

	/* set the user session attribute in the service context handle*/
	(void) OCIAttrSet ((void  *)mysvchp, OCI_HTYPE_SVCCTX, (void  *)myusrhp, (ub4) 0, OCI_ATTR_SESSION, myerrhp);

	return myerrhp;
}
MULTIMAP_API OCIError* OCITools::select(std::vector<std::string> columns, std::vector<GenericType> values, std::string select) {
	OCIStmt *stmthp;

	ub4 region_id;
	char region_name[100];
	ub4 num_rows = 1;

	int err = 0;
	err = OCIStmtPrepare2(mysvchp, &stmthp, myerrhp, (const OraText *) select.c_str(), select.size(), NULL, 0, OCI_NTV_SYNTAX, OCI_DEFAULT);
	err = OCIStmtExecute(mysvchp, stmthp, myerrhp, 0, 0, (OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_DEFAULT);
	return 0;
}
MULTIMAP_API int OCITools::createTableLike(std::string newTableName, std::string existingTableName) {
	return 0;
}
MULTIMAP_API int OCITools::createTableLike(std::string newTableName, std::string existingTableName, std::vector<std::string> & omitColumns) {
	return 0;
}