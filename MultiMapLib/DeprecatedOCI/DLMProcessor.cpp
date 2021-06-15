#include "MultiMap.h"
#include "DLMProcessor.h"
#include "Parameters.h"
#include "MBRect.h"
#include "Logger.h"

MULTIMAP_API DLMProcessor::~DLMProcessor(void) {
	if ( connector ) {
		delete connector;
		connector = NULL;
	}
	if ( params ) {
		delete params;
		params = NULL;
	}
}

MULTIMAP_API DLMProcessor::DLMProcessor(DLMParams* _params) {

	if ( _params ) {
		params = new DLMParams(*_params);
	}
}

MULTIMAP_API void DLMProcessor::Run() {
	char buffer[1024];
	connector = new GDALOCIConnector();
	//if ( verbose ) {
		connector->setLoggerLevel(INFO);
	//}
	connector->setDim(2);
	connector->setSRID(2157);
	connector->setSequenceName(params->outTableName+"_SEQ");
	sprintf(buffer,"C:\\MultiMap\\WorkFolder\\%d\\",params->productId);
	connector->setWorkFolder(std::string(buffer));
	connector->erode = params->erode;
	connector->setProductId(params->productId);
	connector->fill = params->fill;
	connector->setDim(2);
	connector->setGeomColumnName(params->geomColumn);
	connector->setIndex(false);
	connector->setThinOnly(params->thinOnly);
	if ( params->thinOnly ) {
		connector->CreateDLMThinOnly(params->outTableName, params->outClassNum, params->resolution, params->inTableName, params->range, params->geomColumn, params->attrColumns, params->classField, params->inClassNum);
	} else {
		connector->CreateDLM(params->outTableName, params->outClassNum, params->resolution, params->inTableName, params->range, params->geomColumn, params->attrColumns,  params->classField, (int)params->inClassNum.size(), &params->inClassNum[0], params->outClassName, true, true, params->keepOrig);
	}
}