#pragma once
#include "MultiMap.h"
#include "GenericBuffer.h"
#include <string>

class FloatNeighborStats {
public:
	FloatNeighborStats(){ lowX = lowY = count = rimCount = 0; average = stddev = rimAverage = rimStddev = 0.f; }
	size_t      lowX;
	size_t      lowY;
	size_t  rimCount;
	size_t     count;
	float    average;
	float     stddev;
	float rimAverage;
	float  rimStddev;
};

class Logger;
class MBRect;
class GDALLayer;
class GDALDataset;
class FloatBuffer : public GenericBuffer {
public:
	MULTIMAP_API ~FloatBuffer(void);
	MULTIMAP_API FloatBuffer(void);
	MULTIMAP_API FloatBuffer(float* buffer, unsigned int xSize, unsigned int ySize, double resolution);

	MULTIMAP_API void setBuffer(float* buffer, unsigned int xSize, unsigned int ySize);
	MULTIMAP_API float* getBuffer(unsigned int *xSize = NULL, unsigned int *ySize=NULL);

	MULTIMAP_API bool ReadBuffer(const char* inputPath, MBRect *cropWindow = NULL);
	MULTIMAP_API int  ReadBuffer(GDALDataset* gdalDS, MBRect *cropWindow = NULL);
	MULTIMAP_API bool SaveBuffer(const char* outputPath, MBRect *cropWindow = NULL);
	MULTIMAP_API int  SaveBuffer(GDALDataset* gdalDS, MBRect *cropWindow = NULL);
	MULTIMAP_API size_t CountNoData(void);
	MULTIMAP_API double NoDataRatio(void);

	MULTIMAP_API void Laplace(void);
	MULTIMAP_API float NeighborAverage(size_t x, size_t y, FloatNeighborStats & stats, int pixelDistance);
	MULTIMAP_API void setNeighbors(size_t x, size_t y, float value, int pixelDistance);
	MULTIMAP_API size_t FillPits(float maxDepth, int pixelDistance);
	MULTIMAP_API size_t NoDataPits(float maxDepth, int pixelDistance);
	MULTIMAP_API size_t SmoothInvDist(float maxDepth, int pixelDistance);

protected:
	inline void FloatBuffer::setBuffer(int x, int y, float value) {
		size_t offset = y*xSize + x;
		buffer[offset] = value;
	}
	inline float FloatBuffer::getBuffer(int x, int y) {
		size_t offset = y*xSize + x;
		return buffer[offset];
	}
	bool IsSet(size_t x, size_t y);
	bool IsNoData(size_t x, size_t y);
	unsigned int NeighborCount(size_t x, size_t y, bool use8 = true);

	float*                 buffer;
	float             maxPitDepth;
};