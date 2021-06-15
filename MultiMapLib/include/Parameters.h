#pragma once
#include "MultiMap.h"
#include "MFUtils.h"
#include "GenericPoint.hpp"
#include "MBRect.h"

typedef enum {CMD_INVALID=0,CMD_HELP,
	// Try to keep these alphabetically organized
	CMD_DLM, CMD_ERODE, CMD_LASDSM, CMD_LASFILL, CMD_MERGE,
	CMD_MAX} MULTIMAP_CMD;

class Parameters {
public:
	MULTIMAP_API ~Parameters(void);
	MULTIMAP_API Parameters(void);

	MULTIMAP_CMD command;
	std::vector<std::string> commandNames;
	bool verbose;
	bool debug;

protected:
	MFUtils mfUtils;

};
// Try to keep these alphabetically organized
class DLMParams : public Parameters {
public:
	MULTIMAP_API DLMParams::DLMParams(void);
	MULTIMAP_API DLMParams(const DLMParams& other);
	MULTIMAP_API DLMParams& operator=(const DLMParams& other);

	std::string outTableName;
	std::string inTableName;
	std::string geomColumn;
	std::string attrColumns;
	std::string classField;
	std::string outClassName;
	std::vector<int> inClassNum;
	int outClassNum;
	int productId;
	double resolution;
	bool  fill;
	bool erode;
	bool keepOrig;
	bool thinOnly;

	MBRect range;

	std::string inClassNumString;
	std::string rangeString;

	//	MULTIMAP_API void GDALOCIConnector::CreateDLM(std::string outTableName, int outClassNum, double resolution, std::string inTableName, MBRect & range, std::string geomColumn, std::string attrColumns, std::string classField, int classNum, std::string className, bool db, bool local, bool keepOrig)
};
class ErodeParams : public Parameters {
public:
	MULTIMAP_API ErodeParams::ErodeParams(void) { factor = 6; invert=false; binary=false;};
	std::vector<std::string> inputFiles;
	std::vector<std::string> outputFiles;
	std::string inputPath;
	std::string outputPath;
	unsigned int factor;
	unsigned int maxPasses;
	bool invert;
	bool binary;
};

class MergeParams : public Parameters {
public:
	MULTIMAP_API MergeParams::MergeParams(void) {};
	std::vector<std::string> inputFiles;
	std::string inputPath;
	std::string outputPath;
	GenericPoint<double> origin;
	bool clean;
};

class LASParams : public Parameters {
public:
	MULTIMAP_API LASParams::LASParams(void) { resolution = 1.0; smoothingPixels = 4; keepOriginalPts = true; fillClass = 0; };
	std::vector<std::string> inputFiles;
	std::string               inputPath;
	std::string              outputPath;
	MBRect                           range;
	std::string             rangeString;
	double                   resolution;
	double            minDistanceFilter;
	float                   maxPitDepth;
	size_t              smoothingPixels;
	bool                keepOriginalPts;
	int                       fillClass;
	std::vector<int>     includeClasses;
	std::vector<int>     excludeClasses;
	std::string           classesString;
};
