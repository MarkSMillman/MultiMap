#include "MultiMapJNI.h"
#include "MultiMap.h"
#ifdef USE_OCI
#include "GDALOCIConnector.h"
#endif
#include "jni.h"
#include "jni_md.h"

/**
@see http://www.javamex.com/tutorials/jni/getting_started.shtml
@see https://thenewcircle.com/static/bookshelf/java_fundamentals_tutorial/_java_native_interface_jni.html
**/

JNIEXPORT jint JNICALL Java_com_mizar_jni_MizaRaster_getDouble (JNIEnv *env, jclass clazz, jint n) {
	return n*2;
}
JNIEXPORT jint JNICALL Java_com_mizar_jni_MizaRaster_openGDALDatasource (JNIEnv *env, jclass obj, jstring _username, jstring _password, jstring _tnsname, jstring _tableNames) {
	void* pDS = nullptr;
#ifdef USE_OCI
	jboolean isCopy = true;
    const char *username = env->GetStringUTFChars(_username,&isCopy);
	const char *password = env->GetStringUTFChars(_password,&isCopy);
	const char *tnsname = env->GetStringUTFChars(_tnsname,&isCopy);
	const char *tableNames = env->GetStringUTFChars(_tableNames,&isCopy);
	GDALOCIConnector connector;
	pDS = connector.Connect(username,password,tnsname,tableNames);
#endif
	return (int) pDS;
}
JNIEXPORT jint JNICALL Java_com_mizar_jni_MizaRaster_closeGDALDatasource (JNIEnv *, jclass, jint) {
	return 0;
}