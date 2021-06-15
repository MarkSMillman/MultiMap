#pragma once
#include "MultiMap.h"
#include "potracelib.h"

class OGRDataSource;
class OGRLayer;
class POTrace {
public:
	MULTIMAP_API ~POTrace();
	MULTIMAP_API POTrace();

	MULTIMAP_API void SetGeotransform(double geotransform[6]);
	MULTIMAP_API int SetBitmap(potrace_bitmap_t *bitMap, int width, int height);
	MULTIMAP_API int SetBitmap(unsigned char *bitMap, int width, int height);
	MULTIMAP_API int Trace(potrace_bitmap_t *bitMap = NULL);
	MULTIMAP_API int SaveToOGRLayer(const char* layerName, OGRDataSource* pDatasource, OGRLayer** ppLayer, double geotransform[6], bool polygon=false, potrace_state_t *st = NULL);
	MULTIMAP_API int SaveToEPS(const char* filePath, potrace_state_t *st = NULL);
	MULTIMAP_API int SaveToEPS(FILE* file, potrace_state_t *st = NULL);

	MULTIMAP_API void FreeBitMap(potrace_bitmap_t **bitMap = NULL ) ;
	MULTIMAP_API void FreeVectors(potrace_state_t **state = NULL ) ;
	MULTIMAP_API void FreeParameters(potrace_param_t **parameters = NULL ) ;

	MULTIMAP_API int Test(void);

private:
	potrace_bitmap_t* AllocateBitmap(int w, int h);

	int createTestBitmap();

	potrace_bitmap_t   *bm;
	potrace_param_t *param;
	potrace_state_t    *st;

	double geotransform[6];
};