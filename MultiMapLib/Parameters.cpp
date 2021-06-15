#include "MultiMap.h"
#include "Parameters.h"

MULTIMAP_API Parameters::~Parameters(void) {;}
MULTIMAP_API Parameters::Parameters(void) {
	command = CMD_HELP;
	verbose = false;
	debug = false;
	commandNames.resize(CMD_MAX);
	commandNames[CMD_HELP].assign("Help");
	commandNames[CMD_ERODE].assign("Erode");
	commandNames[CMD_MERGE].assign("Merge");
	commandNames[CMD_INVALID].assign("Invalid");
}

MULTIMAP_API DLMParams::DLMParams(void) {
	outClassNum = 0;
	resolution = 1.0;
	fill = true;
	erode = false;
	keepOrig = false;
	thinOnly = true;
}
MULTIMAP_API DLMParams::DLMParams(const DLMParams& other) {
	*this = other;
}
MULTIMAP_API DLMParams& DLMParams::operator=(const DLMParams& other) {
	outTableName = other.outTableName;
	inTableName = other.inTableName;
	geomColumn = other.geomColumn;
	attrColumns = other.attrColumns;
	classField = other.classField;
	outClassName = other.outClassName;
	outClassNum = other.outClassNum;
	inClassNum = other.inClassNum;
	productId = other.productId;
	resolution = other.resolution;
	fill = other.fill;
	erode = other.erode;
	keepOrig = other.keepOrig;
	thinOnly = other.thinOnly;
	range = other.range;
	
	return *this;
}