#include "MultiMap.h"
#include "MFUtils.h"
#include "ByteBuffer.h"
#include "GDALOCIConnector.h"
#include "ParameterParser.h"
#include "DLMProcessor.h"
#include "LASProcessor.h"
#include "Logger.h"
#include <iostream>

int main(int argc, char* argv[] ) {
	bool status = true;
	MFUtils mfUtils;
	bool verbose = false;

	time_t startTime;
	time(&startTime);

	ParameterParser* cmdParser = NULL;
	Parameters* params = ParameterParser::Parse(argc,argv,&cmdParser);
	if ( params ) {
		verbose = true;//params->verbose;

		cmdParser->GetProperties();
		switch ( params->command ) {
		case CMD_LASDSM:
		{
			std::cout << "LASDSM" << std::endl;
			LASParams* lasParams = (LASParams*)params;
			LASProcessor* lasProcessor = new LASProcessor(lasParams);
			lasProcessor->m_verbose = params->verbose;
			lasProcessor->m_debug = params->debug;
			lasProcessor->createDSM();
			delete lasProcessor;
			delete lasParams;
		}
		break;
		case CMD_LASFILL:
		{
			std::cout << "LASFILL" << std::endl;
			LASParams* lasParams = (LASParams*)params;
			LASProcessor* lasProcessor = new LASProcessor(lasParams);
			lasProcessor->m_verbose = params->verbose;
			lasProcessor->m_debug = params->debug;
			lasProcessor->fill();
			delete lasProcessor;
			delete lasParams;
		}
		break;
		case CMD_DLM:
			{
				std::cout << "DLM" << std::endl;
				DLMParams* dlmParams = (DLMParams*) params;
				DLMProcessor* dlmProcessor = new DLMProcessor(dlmParams);
				dlmProcessor->Run();
				delete dlmProcessor;
				delete dlmParams;
			}
			break;
		case CMD_ERODE:
			{
				std::cout << "ERODE" << std::endl;
				ErodeParams* erodeParams = (ErodeParams*) params;
				ByteBuffer eroder;
				eroder.setVerbose(params->verbose);
				eroder.setDebug(params->debug);
				eroder.setInvert(erodeParams->invert);
				eroder.setBinary(erodeParams->binary);
				eroder.setMaxPasses(erodeParams->maxPasses);
				eroder.setCompress(false);
				if ( eroder.ReadBuffer(erodeParams->inputPath.c_str()) ) {
					if ( eroder.Erode(erodeParams->factor) ) {
						eroder.SaveBuffer(erodeParams->outputPath.c_str());
					}
				}
			}
			break;
		case CMD_MERGE:
			std::cout << "MERGE" << std::endl;
			cmdParser->WriteJSON();
			break;
		case CMD_INVALID:
			std::cout << "INVALID" << std::endl;
		default:
			break;
		}
		if (cmdParser) {
			delete cmdParser;
		} else {
			delete params;
		}
	} else {
		status = false;
	}

	if ( verbose ) {
		printf("Finished at %s\n",mfUtils.TimeIs().c_str());
		time_t stopTime;
		time(&stopTime);
		double minutes = static_cast<double>(stopTime - startTime) / 60.0;
		printf("Processing time = %.2f\n", minutes);
	}
	if ( !status ) {
		exit(1);
	}

}