#pragma once
#include "MultiMap.h"
#include "Action.h"

class Logger;
class GDALOCIConnector;
class ActionProcessor {
public:
	MULTIMAP_API ~ActionProcessor();
	MULTIMAP_API ActionProcessor();

	MULTIMAP_API bool Run ( std::string actionFile );
	MULTIMAP_API bool Run ( std::vector<Action> actions );
	MULTIMAP_API bool Run ( Action action );

	MULTIMAP_API void SetLogFile( std::string logFile );
	MULTIMAP_API void SetLogScreen( bool toScreen = true );

private:
	Logger* logger;
	std::vector<Action> ReadJSON(std::string actionFile);

	// Defaults
	std::string inDriver;
	std::string outDriver;
	std::string inLayerSQL;
	std::string inLayerName;
	std::string outLayerSQL;
	std::string outLayerName;
	double rasterResolution;

	std::string connectorString;
	std::string username;
	std::string password;
	std::string tnsname;
	std::string tables;
	std::string inputFolder;
	std::string outputFolder;

	bool debug;
	bool verbose;
	std::vector<Action> actions;
};