#pragma once
#include "MultiMap.h"

class MBRect;
class MapTextAPI {
public:
	MULTIMAP_API ~MapTextAPI(void);
	MULTIMAP_API MapTextAPI(void);

	MULTIMAP_API int PrepareShapes(std::string dataFolder, int dlmScale, std::string feature, MBRect & range);
	MULTIMAP_API int Run(std::string commandline);
	MULTIMAP_API int RunArcView(std::string configFilePath, std::string specFilePath, std::string dataFolder);
	MULTIMAP_API int RunArcView(std::string metadataFolder, std::string configFileName, std::string specFileName, std::string dataFolder);
};