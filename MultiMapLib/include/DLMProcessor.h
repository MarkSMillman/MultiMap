#pragma once
#include "MultiMap.h"
#include "GDALOCIConnector.h"
#include <string>
#include <vector>
#include "MBRect.h"

class DLMParams;
class DLMProcessor : public GDALOCIConnector {
public:
	MULTIMAP_API ~DLMProcessor(void);
	MULTIMAP_API DLMProcessor(DLMParams* params=NULL);

	MULTIMAP_API void Run();

	GDALOCIConnector* connector;
	DLMParams* params;
};