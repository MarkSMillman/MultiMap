#include "MultiMap.h"
#include "MapTextAPI.h"
#include "MBRect.h"
#include "ThreadTools.h"

MULTIMAP_API MapTextAPI::~MapTextAPI(void) {
}
MULTIMAP_API MapTextAPI::MapTextAPI(void) {
}

MULTIMAP_API int MapTextAPI::Run(std::string commandline) {
	int errCode = 0;
	ThreadTools launcher;
	errCode = launcher.RunAndWait(commandline);
	return errCode;
}
MULTIMAP_API int MapTextAPI::RunArcView(std::string configFilePath, std::string specFilePath, std::string dataFolder) {
	// Label-EZ /J ARCVIEW <Full path Spec File> <Full Path Config File> <Full Path Data Directory>
	std::string commandline;
	commandline.append("Label-EZ/J ARCVIEW ");
	commandline.append(specFilePath);
	commandline.append(" ");
	commandline.append(configFilePath);
	commandline.append(" ");
	commandline.append(dataFolder);
	return Run(commandline);
}
MULTIMAP_API int MapTextAPI::RunArcView(std::string metadataFolder, std::string configFileName, std::string specFileName, std::string dataFolder) {
	std::string commandline;
	commandline.append("Label-EZ/J ARCVIEW ");
	commandline.append(metadataFolder);
	commandline.append(FILE_SEP);
	commandline.append(specFileName);
	commandline.append(" ");
	commandline.append(metadataFolder);
	commandline.append(FILE_SEP);
	commandline.append(configFileName);
	commandline.append(" ");
	commandline.append(dataFolder);
	return Run(commandline);
}

MULTIMAP_API int MapTextAPI::PrepareShapes(std::string dataFolder, int dlmScale, std::string feature, MBRect & range) {
	int errCode = 0;

	return errCode;
}