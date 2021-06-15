#pragma once
#include <vector>
#include "MBRect.h"
#include "GenericPoint.hpp"
#include "Logger.h"


static const unsigned char LAS_CLASS_CREATED = 0;
static const unsigned char LAS_CLASS_UNCLASSIFIED = 1;
static const unsigned char LAS_CLASS_GROUND = 2;
static const unsigned char LAS_CLASS_LOW_VEGETATION = 3;
static const unsigned char LAS_CLASS_MED_VEGETATION = 4;
static const unsigned char LAS_CLASS_HI_VEGETATION = 5;
static const unsigned char LAS_CLASS_BUILDING = 6;
static const unsigned char LAS_CLASS_LOW_POINT = 7;
static const unsigned char LAS_CLASS_NOISE = 7;
static const unsigned char LAS_CLASS_MODEL_KEY_POINT = 8;
static const unsigned char LAS_CLASS_WATER = 9;
static const unsigned char LAS_CLASS_OVERLAP_POINTS = 12;

static const float NODATA = -32767.0;
static const unsigned char MASK_OMIT = 0xFF;
static const unsigned char MASK_UNDEFINED = 0;
static const unsigned char MASK_EDGE = 1;
static const unsigned char MASK_ORIGINAL = 2;
static const unsigned char MASK_LAPLACE = 3;

class MappedFile;
class LASParams;
class IOReaderLAS;
class LASProcessor {
public:
	MULTIMAP_API ~LASProcessor(void);
	MULTIMAP_API LASProcessor(LASParams* params=NULL);

	MULTIMAP_API int fill(void);
	MULTIMAP_API int createDSM(void);

	MULTIMAP_API std::string getProjectionWKT(int epsg = 0);

	MULTIMAP_API void setInput(std::string filePath);
	MULTIMAP_API std::string getInput(void);
	MULTIMAP_API void setOutput(std::string filePath);
	MULTIMAP_API std::string getOutput(void);
	MULTIMAP_API void setResolution(double resolution);
	MULTIMAP_API double getResolution(void);
	MULTIMAP_API void setSmoothingPixels(size_t pixels);
	MULTIMAP_API size_t getSmoothingPixels(void);
	MULTIMAP_API void setKeepOriginalPoints(bool keepOriginalPoints=true);
	MULTIMAP_API bool getKeepOriginalPoints(void);
	MULTIMAP_API void setFillClass(int fillClass);
	MULTIMAP_API int getFillClass(void);
	MULTIMAP_API void setIncludeClasses(std::vector<int> includeClasses);
	MULTIMAP_API void setIncludeClass(int includeClass);
	MULTIMAP_API std::vector<int> getIncludeClasses(void);
	MULTIMAP_API void setExcludeClasses(std::vector<int> excludeClasses);
	MULTIMAP_API void setExcludeClass(int excludeClass);
	MULTIMAP_API std::vector<int> getExcludeClasses(void);
	MULTIMAP_API void setRange(MBRect range);
	MULTIMAP_API void setRange(double minX, double minY, double maxX, double maxY);
	MULTIMAP_API void setRange(double minX, double minY, double minZ, double maxX, double maxY, double maxZ);
	MULTIMAP_API MBRect getRange(void);
	MULTIMAP_API void setBufferSize(size_t xSize, size_t ySize);
	MULTIMAP_API size_t getBufferXSize(void);
	MULTIMAP_API size_t getBufferYSize(void);
	MULTIMAP_API void setBufferOrigin(GenericPoint<double> origin);
	MULTIMAP_API void setBufferOrigin(double x, double y);
	MULTIMAP_API GenericPoint<double> getBufferOrigin(void);
	bool                        m_verbose;
	bool                          m_debug;
private:
	size_t countPoints(IOReaderLAS* reader);

	Logger*                      m_logger;
	MappedFile*                 m_inputMF;
	MappedFile*                m_outputMF;
	float*                       m_buffer;
	unsigned char*                 m_mask;
	
	std::vector<std::string> m_inputFiles;
	std::string               m_inputFile;
	std::string              m_outputFile;
	MBRect                        m_range;
	double                   m_resolution;
	double            m_minDistanceFilter;
	size_t              m_smoothingPixels;
	float                   m_maxPitDepth;
	bool                m_keepOriginalPts;
	int                       m_fillClass;
	std::vector<int>     m_includeClasses;
	std::vector<int>     m_excludeClasses;

	size_t                  m_bufferXSize;
	size_t                  m_bufferYSize;
	GenericPoint<double>   m_bufferOrigin;


};