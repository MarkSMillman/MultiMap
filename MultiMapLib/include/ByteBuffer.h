#pragma once
#include "MultiMap.h"
#include "GenericBuffer.h"

class Logger;
class MBRect;
class GDALLayer;
class GDALDataset;
class ByteBuffer : public GenericBuffer {
public:
	MULTIMAP_API ~ByteBuffer(void);
	MULTIMAP_API ByteBuffer(void);
	MULTIMAP_API ByteBuffer(unsigned char* buffer, unsigned int xSize, unsigned int ySize, double resolution);

	MULTIMAP_API bool ReadBuffer(const char* inputPath, MBRect *cropWindow = NULL);
	MULTIMAP_API int  ReadBuffer(GDALDataset* gdalDS, MBRect *cropWindow = NULL);
	MULTIMAP_API bool SaveBuffer(const char* outputPath, MBRect *cropWindow = NULL);
	MULTIMAP_API int  SaveBuffer(GDALDataset* gdalDS, MBRect *cropWindow = NULL);
	MULTIMAP_API void InvertBuffer(bool binary=false);
	MULTIMAP_API void BinaryBuffer(unsigned char minValue = 0x01);

	MULTIMAP_API bool Erode(unsigned int factor);
	MULTIMAP_API bool Fill(unsigned int factor = 8, unsigned int minFills = 0);

	MULTIMAP_API void setInvert(bool invert);
	MULTIMAP_API bool getInvert();
	MULTIMAP_API void setBinary(bool binary);
	MULTIMAP_API bool getBinary();
	MULTIMAP_API unsigned char* getBuffer();

	MULTIMAP_API void setNoData(unsigned char _nodata);
	MULTIMAP_API bool IsSet(size_t x, size_t y);
	MULTIMAP_API bool IsNoData(size_t x, size_t y);
	MULTIMAP_API unsigned int NeighborCount(size_t x, size_t y, bool use8 = true);
	MULTIMAP_API unsigned int NeighborCountMatch(size_t x, size_t y, bool use8 = true);
	MULTIMAP_API unsigned int NeighborCountDiffer(size_t x, size_t y, bool use8 = true);
	MULTIMAP_API unsigned int NeighborhoodCount(size_t x, size_t y, size_t & minCount, size_t & maxCount);
	MULTIMAP_API bool IsLonely(size_t x, size_t y, size_t pixelDistance);
	MULTIMAP_API void CreateNeighborMap();
	MULTIMAP_API void DecrementNeighbors(size_t x, size_t y);

protected:
	inline void ByteBuffer::setBuffer(int x, int y, unsigned char value) {
		int offset = y*xSize + x;
		buffer[offset] = value;
	}
	inline unsigned char ByteBuffer::getBuffer(int x, int y) {
		int offset = y*xSize + x;
		return buffer[offset];
	}

	// From http://lodev.org/cgtutor/floodfill.html
	void floodFill4(int x, int y, unsigned char newColor, unsigned char oldColor);
	int  floodCount4(int x, int y, unsigned char newColor, unsigned char oldColor, int & maxFillCount);
	void floodFillScanlineStack(int x, int y, unsigned char newColor, unsigned char oldColor, size_t maxFillCount = 10);

	unsigned char         cNoData;

	bool                   invert;
	bool                   binary;
	bool            foreignBuffer;
	unsigned char*         buffer;
	unsigned char*      neighbors;
};