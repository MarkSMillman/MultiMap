#pragma once
#include "MultiMap.h"


class ThreadTools {
public:
	MULTIMAP_API ~ThreadTools(void);
	MULTIMAP_API ThreadTools(void);

	//MULTIMAP_API int Run(std::string commandline);
	MULTIMAP_API int RunAndWait(std::string commandline);
	MULTIMAP_API std::vector<std::string> GetResponse(void);
	MULTIMAP_API void ClearResponse(void);

private:
	std::string lastCommandLine;
	std::vector<std::string> response;
};