#pragma once
#include "MultiMap.h"

class Logger;
class MBRect;
class GDALLayer;
class GDALDataset;
class GenericBuffer {
public:
	virtual ~GenericBuffer(void);
	GenericBuffer(void);

	virtual bool IsSet(size_t x, size_t y) = 0;
	virtual bool IsNoData(size_t x, size_t y) = 0;
	virtual bool ReadBuffer(const char* inputPath, MBRect *cropWindow = NULL) = 0;
	virtual int  ReadBuffer(GDALDataset* gdalDS, MBRect *cropWindow = NULL) = 0;
	virtual bool SaveBuffer(const char* outputPath, MBRect *cropWindow = NULL) = 0;
	virtual int  SaveBuffer(GDALDataset* gdalDS, MBRect *cropWindow = NULL) = 0;

	MULTIMAP_API void setProjectionRef(char* szProjectionRef);
	MULTIMAP_API const char* getProjectionRef(void);
	MULTIMAP_API void setGeotransform(double xOrigin, double yOrigin, double resolution);
	MULTIMAP_API void setGeotransform(double geotransform[6]);
	MULTIMAP_API const double* getGeotransform(void);

	MULTIMAP_API bool GetNeighbor(size_t direction, size_t x, size_t y, size_t & ix, size_t & iy);
	MULTIMAP_API void setMinPasses(unsigned int passes);
	MULTIMAP_API unsigned int getMinPasses();
	MULTIMAP_API void setMaxPasses(unsigned int passes);
	MULTIMAP_API unsigned int getMaxPasses();
	MULTIMAP_API void setCompress(bool compress);
	MULTIMAP_API bool getCompress();
	MULTIMAP_API void setVerbose(bool verbose);
	MULTIMAP_API bool getVerbose();
	MULTIMAP_API void setDebug(bool debug);
	MULTIMAP_API bool getDebug();
	MULTIMAP_API int getErrorCode();
	MULTIMAP_API int getRasterX();
	MULTIMAP_API int getRasterY();

	MULTIMAP_API bool InGrid(size_t x, size_t y);
	MULTIMAP_API bool OnEdge(size_t x, size_t y);

protected:
	static const int	DeltaX[8]; // = { 0, 1, 1, 1, 0,-1,-1,-1 };
	static const int	DeltaY[8]; // = { 1, 1, 0,-1,-1,-1, 0, 1 };

	Logger                *logger;
	bool            foreignBuffer;

	bool        pop(int &x, int &y);
	bool         push(int x, int y);
	std::vector<size_t>       stack;
	size_t                 maxStack;
	size_t                  changed;

	unsigned int              xSize;
	unsigned int              ySize;

	int                    dataType;
	char      szProjectionRef[4096];
	char         szDriverDescr[128];
	double          geotransform[6];
	double               resolution;
	double                  dNoData;
	unsigned int          maxPasses;
	unsigned int          minPasses;

	int                     errCode;
	
	bool                   compress;
	bool                    verbose;
	bool                      debug;
};