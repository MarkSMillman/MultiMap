#pragma warning ( disable : 4251 )
#include "MultiMap.h"
#include "ParameterParser.h"
#include "Parameters.h"

DISABLE_WARNINGS
#include <iostream>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
ENABLE_WARNINGS


ParameterParser::~ParameterParser(void) {
	if (cmdParser) {
		delete cmdParser;
		cmdParser = NULL;
	}

	if (pPropertyTree) {
		delete pPropertyTree;
		pPropertyTree = NULL;
	}
}
ParameterParser::ParameterParser(void) {
	cmdParser = NULL;
	params = NULL;
	pPropertyTree = new boost::property_tree::iptree();
}
void ParameterParser::Parse(void) {
	namespace po = boost::program_options;

	desc.add_options()
		("verbose,v", po::value<bool>()->implicit_value(true), "Provide feed back")
		("debug", po::value<bool>()->implicit_value(true), "Provide lots of feed back")
		;
}
VIRTUAL bool ParameterParser::AssignOptions(void) {
	bool success = true;
	try {
		Assign<bool>(params->verbose, "verbose", vm);
		Assign<bool>(params->debug, "debug", vm, params->verbose);
	}
	catch (const std::exception& e) {
		std::cout << __FUNCTION__ << " failed " << e.what() << std::endl;
		success = false;
	}
	return success;
}
VIRTUAL bool ParameterParser::ReadJSON(std::string filename) {
	bool success = true;
	try {

	}
	catch (const std::exception& e) {
		std::cout << __FUNCTION__ << " failed " << e.what() << std::endl;
		success = false;
	}
	return success;
}
VIRTUAL bool ParameterParser::WriteJSON(std::string filename) {
	bool success = true;
	try {
		bool success = true;
		try {
			if (filename.empty() || mfUtils.ToLower(filename).compare("stdout") == 0) {
				write_json(std::cout, (*pPropertyTree));
			}
			else {
				write_json(filename, (*pPropertyTree));
			}
		}
		catch (const std::exception& e) {
			std::cout << __FUNCTION__ << " failed " << e.what() << std::endl;
			success = false;
		}
		return success;
	}
	catch (const std::exception& e) {
		std::cout << __FUNCTION__ << " failed " << e.what() << std::endl;
		success = false;
	}
	return success;
}
PRIVATE bool ParameterParser::ParseOptions(int argc, char* argv[]) {
	bool status = true;
	try {
		namespace po = boost::program_options;
		namespace po_style = boost::program_options::command_line_style;
		po::store(po::command_line_parser(argc, argv).options(desc).style(po_style::unix_style | po_style::case_insensitive).run(), vm);
		po::notify(vm);
	}
	catch (const std::exception& e) {
		status = false;
		std::cout << "MultiMap parsing exception" << std::endl;
		std::cout << e.what() << std::endl;
	}
	return status;
}
/**
* This method is a little more convoluted than I would like because it recursively calls itself to get the HELP messages.
* With hindsight this could have been handled more elegantly but it works now so I'll leave it be. Millman
**/
STATIC Parameters* ParameterParser::Parse(int argc, char* argv[], ParameterParser** _cmdParser, MULTIMAP_CMD *_about) {
	Parameters* params = NULL;
	ParameterParser* cmdParser = NULL;
	MFUtils mfUtils;

	MULTIMAP_CMD command;
	MULTIMAP_CMD about;
	std::string cmd = "help";
	if (argc > 1) {
		cmd = mfUtils.ToLower(std::string(argv[1]));
	}
	if (cmd.compare("help") == 0) {
		command = CMD_HELP;
		if (argc > 2) {
			Parse(argc - 1, &argv[1], &cmdParser, &about);
			if (cmdParser) {
				try {
					namespace po = boost::program_options;
					namespace po_style = boost::program_options::command_line_style;
					cmdParser->desc.add_options()
						("verbose,v", po::value<bool>()->implicit_value(true), "Provide feed back")
						("debug", po::value<bool>()->implicit_value(true), "Provide lots of feed back")
						;
					po::store(po::command_line_parser(argc, argv).options(cmdParser->desc).style(po_style::unix_style | po_style::case_insensitive).run(), cmdParser->vm);
					po::notify(cmdParser->vm);
					Parameters tmpParams;
					std::cout << "MultiMap " + tmpParams.commandNames[about] + " parameters (UPPER CASE indicated a required parameter)" << std::endl;
					std::cout << cmdParser->desc << std::endl;
				}
				catch (const std::exception& e) {
					std::cout << "MultiMap parsing exception" << std::endl;
					std::cout << e.what() << std::endl;
				}
				delete cmdParser;
				cmdParser = NULL;
			}
		}
		else {
			std::cout << "MultiMap Help parameters (UPPER CASE is minimum required)" << std::endl;
			std::cout << "  Erode" << std::endl;
			std::cout << "  Merge" << std::endl;
			std::cout << "try MultiMap HELP <command> for more information" << std::endl << std::endl;
		}
		// Try to keep these alphabetically organized
	}
	else if (cmd.substr(0, 1).compare("d") == 0) {
		command = CMD_DLM;
		if (!_about)
			params = new DLMParams();
		cmdParser = new DLMParser(params);
	}
	else if (cmd.substr(0, 1).compare("e") == 0) {
		command = CMD_ERODE;
		if (!_about)
			params = new ErodeParams();
		cmdParser = new ErodeParser(params);
	}
	else if (cmd.substr(0, 1).compare("m") == 0) {
		command = CMD_MERGE;
		if (!_about)
			params = new MergeParams();
		cmdParser = new MergeParser(params);
	}
	else if (cmd.substr(0, 4).compare("lasd") == 0) {
		command = CMD_LASDSM;
		if (!_about)
			params = new LASParams();
		cmdParser = new LASParser(params);
	}
	else if (cmd.substr(0, 4).compare("lasf") == 0) {
		command = CMD_LASFILL;
		if (!_about)
			params = new LASParams();
		cmdParser = new LASParser(params);
	}

	if (_about) {
		*_about = command;
	}
	if (_cmdParser) {
		*_cmdParser = cmdParser;
	}
	if (params && cmdParser) {
		params->command = command;
		cmdParser->params = params;
		try {
			namespace po = boost::program_options;
			namespace po_style = boost::program_options::command_line_style;
			cmdParser->desc.add_options()
				("verbose,v", po::value<bool>()->implicit_value(true), "Provide feed back")
				("debug", po::value<bool>()->implicit_value(true), "Provide lots of feed back")
				;
			po::store(po::command_line_parser(argc, argv).options(cmdParser->desc).style(po_style::unix_style | po_style::case_insensitive).run(), cmdParser->vm);
			po::notify(cmdParser->vm);
			if (!cmdParser->AssignOptions()) {
				delete params;
				params = NULL;
			}
			if (params && _cmdParser) {
				*_cmdParser = cmdParser;
			}
			else {
				delete cmdParser;
				cmdParser = NULL;
				*_cmdParser = NULL;
			}
		}
		catch (const std::exception& e) {
			std::cout << __FUNCTION__ << " failed " << e.what() << std::endl;
			if (params) {
				delete params;
				params = NULL;
			}
			if (cmdParser) {
				delete cmdParser;
				cmdParser = NULL;
			}
		}
	}
	return params;
}

bool ParameterParser::SetRange(MBRect & range, std::string input) {
	bool fourOrSize = false;
	MFUtils mfUtils;
	std::vector<std::string> values;
	if (mfUtils.BeginsWith(input, "(")) {
		input = input.substr(1);
	}
	if (mfUtils.EndsWith(input, ")")){
		input = input.substr(0, input.size() - 1);
	}
	mfUtils.ToStrings(values, input, ",");
	if (values.size() == 4) {
		range.minX = atof(values[0].c_str());
		range.minY = atof(values[1].c_str());
		range.maxX = atof(values[2].c_str());
		range.maxY = atof(values[3].c_str());
		fourOrSize = true;
	}
	else if (values.size() == 6) {
		range.minX = atof(values[0].c_str());
		range.minY = atof(values[1].c_str());
		range.minZ = atof(values[2].c_str());
		range.maxX = atof(values[3].c_str());
		range.maxY = atof(values[4].c_str());
		range.maxZ = atof(values[5].c_str());
		fourOrSize = true;
	}
	return fourOrSize;
}
void ParameterParser::SetInts(std::vector<int> & ints, std::string input) {
	MFUtils mfUtils;
	std::vector<std::string> values;
	if (mfUtils.BeginsWith(input, "(")) {
		input = input.substr(1);
	}
	if (mfUtils.EndsWith(input, ")")){
		input = input.substr(0, input.size() - 1);
	}
	mfUtils.ToStrings(values, input, ",");
	for (size_t i = 0; i < values.size(); i++){
		int classification = atoi(values[i].c_str());
		ints.push_back(classification);
	}
}
std::string ParameterParser::ToString(std::vector<int> ints) {
	std::string stringOfInts;
	stringOfInts.append("(");
	std::string comma(",");
	char ibuf[32];
	for (size_t i = 0; i < ints.size(); i++) {
		if (i != 0) {
			stringOfInts.append(comma);
		}
		_itoa(ints[i], ibuf, 10);
		stringOfInts.append(std::string(ibuf));
	}
	stringOfInts.append(")");
	return stringOfInts;
}
DLMParser::DLMParser(Parameters* _params) {
	namespace po = boost::program_options;
	desc.add_options()
		("INPUT,I", po::value<std::string>(), "Input table name")
		("OUTPUT,O", po::value<std::string>(), "Output table name")
		("GEOM,G", po::value<std::string>(), "Geometry column name")
		("ATTR,A", po::value<std::string>(), "Attribute columns")
		("CLASS,C", po::value<std::string>(), "Class column name")
		("RANGE,N", po::value<std::string>(), "Range=minX,minY,maxX,maxY")
		("OUTCLASSNAME,X", po::value<std::string>(), "Output class name")
		("OUTCLASSNUM,Y", po::value<int>(), "Output class number")
		("INCLASSNUM,Z", po::value<std::string>(), "inclassno=#,#,#...")
		("PRODUCT,P", po::value<int>(), "Product key")
		("RESOLUTION,E", po::value<double>(), "Resolution")
		("FILL,F", po::value<bool>(), "Fill")
		("ERODE,E", po::value<bool>(), "Erode")
		("KEEP,K", po::value<bool>(), "Keep original")
		("THIN,H", po::value<bool>(), "Thin only (no VRV)")
		;
	pParams = (DLMParams*)_params;

}
bool DLMParser::AssignOptions(void) {
	bool success = true;

	DLMParams* pParams = (DLMParams*)params;
	pParams->geomColumn = "POLY_GEOM_VALUE";
	pParams->attrColumns = "ID,PRODUCT,SCALE,FORM_ID,FUNC_ID,STATUS,PRIMARY_NAME,SECONDARY_NAME";
	pParams->resolution = 1.0;
	pParams->fill = true;
	pParams->erode = false;
	pParams->keepOrig = false;
	pParams->thinOnly = false;

	try {
		Assign<bool>(params->verbose, "verbose", vm);
		Assign<bool>(params->debug, "debug", vm, params->verbose);
		if (Assign<std::string>(pParams->inTableName, "INPUT", vm, params->verbose)) {
			if (Assign<std::string>(pParams->outTableName, "OUTPUT", vm, params->verbose)) {
				if (Assign<std::string>(pParams->classField, "CLASS", vm, params->verbose)) {
					if (Assign<int>(pParams->productId, "PRODUCT", vm, params->verbose)) {
						if (Assign<int>(pParams->outClassNum, "OUTCLASSNUM", vm, params->verbose)) {
							if (Assign<std::string>(pParams->rangeString, "RANGE", vm, params->verbose)) {
								std::vector<double>doubles;
								mfUtils.ParseList(doubles, pParams->rangeString);
								pParams->range.minX = doubles[0];
								pParams->range.minY = doubles[1];
								pParams->range.maxX = doubles[2];
								pParams->range.maxY = doubles[3];

								if (Assign<std::string>(pParams->inClassNumString, "INCLASSNUM", vm, params->verbose)) {
									mfUtils.ParseList(pParams->inClassNum, pParams->inClassNumString);
								}
							}
							else {
								std::cout << __FUNCTION__ << " failed required --range parameter missing." << std::endl;
								std::cout << "Try MultiMap HELP DLM for more information." << std::endl;
								success = false;
							}
						}
						else {
							std::cout << __FUNCTION__ << " failed required --product parameter missing." << std::endl;
							std::cout << "Try MultiMap HELP DLM for more information." << std::endl;
							success = false;
						}
					}
					else {
						std::cout << __FUNCTION__ << " failed required --product parameter missing." << std::endl;
						std::cout << "Try MultiMap HELP DLM for more information." << std::endl;
						success = false;
					}
				}
				else {
					std::cout << __FUNCTION__ << " failed required --class parameter missing." << std::endl;
					std::cout << "Try MultiMap HELP DLM for more information." << std::endl;
					success = false;
				}
			}
			else {
				std::cout << __FUNCTION__ << " failed required --output parameter missing." << std::endl;
				std::cout << "Try MultiMap HELP DLM for more information." << std::endl;
				success = false;
			}
		}
		else {
			std::cout << __FUNCTION__ << " failed required --input parameter missing." << std::endl;
			std::cout << "Try MultiMap HELP ERODE for more information." << std::endl;
			success = false;
		}
		Assign<std::string>(pParams->geomColumn, "GEOM", vm, params->verbose);
		Assign<std::string>(pParams->attrColumns, "ATTR", vm, params->verbose);
		Assign<std::string>(pParams->outClassName, "OUTCLASSNAME", vm, params->verbose);
		Assign<double>(pParams->resolution, "RESOLUTION", vm, params->verbose);
		Assign<bool>(pParams->fill, "FILL", vm, params->verbose);
		Assign<bool>(pParams->erode, "ERODE", vm, params->verbose);
		Assign<bool>(pParams->keepOrig, "KEEP", vm, params->verbose);
		Assign<bool>(pParams->thinOnly, "THIN", vm, params->verbose);

	}
	catch (const std::exception& e) {
		std::cout << __FUNCTION__ << " failed " << e.what() << std::endl;
		success = false;
	}
	return success;
}
bool DLMParser::GetProperties(void) {
	bool success = true;
	try {
		Assign("verbose", pParams->verbose, pPropertyTree);
		Assign("debug", pParams->debug, pPropertyTree);
		Assign("input", pParams->inTableName, pPropertyTree);
		Assign("output", pParams->outTableName, pPropertyTree);
		Assign("geom", pParams->geomColumn, pPropertyTree);
		Assign("attr", pParams->attrColumns, pPropertyTree);
		Assign("class", pParams->classField, pPropertyTree);
		Assign("outclassname", pParams->outClassName, pPropertyTree);
		Assign("outclassnum", pParams->outClassNum, pPropertyTree);
		Assign("inclassnum", pParams->inClassNumString, pPropertyTree);
		Assign("range", pParams->rangeString, pPropertyTree);
		Assign("product", pParams->productId, pPropertyTree);
		Assign("resolution", pParams->resolution, pPropertyTree);
		Assign("fill", pParams->fill, pPropertyTree);
		Assign("erode", pParams->erode, pPropertyTree);
		Assign("keep", pParams->keepOrig, pPropertyTree);
		Assign("thin", pParams->thinOnly, pPropertyTree);
	}
	catch (const std::exception& e) {
		std::cout << __FUNCTION__ << " failed " << e.what() << std::endl;
		success = false;
	}
	return success;
}
bool DLMParser::ReadJSON(std::string filename) {
	bool success = true;
	try {

	}
	catch (const std::exception& e) {
		std::cout << __FUNCTION__ << " failed " << e.what() << std::endl;
		success = false;
	}
	return success;
}
ErodeParser::ErodeParser(Parameters* _params) {
	namespace po = boost::program_options;
	desc.add_options()
		("INPUT,I", po::value<std::string>(), "Input file(s) may use wild-cards")
		("OUTPUT,O", po::value<std::string>(), "Output file or Folder if input wild-carded")
		("FACTOR", po::value<unsigned int>()->implicit_value(6), "Erosion factor")
		("PASSES", po::value<unsigned int>()->implicit_value(300), "Maximum number of passes")
		("INVERT", po::value<bool>()->implicit_value(false), "Invert image")
		("BINARY", po::value<bool>()->implicit_value(false), "Force image to binary")
		;
	pParams = (ErodeParams*)_params;
}
bool ErodeParser::AssignOptions(void) {
	bool success = true;
	ErodeParams* pParams = (ErodeParams*)params;
	try {
		Assign<bool>(params->verbose, "verbose", vm);
		Assign<bool>(params->debug, "debug", vm, params->verbose);
		if (Assign<std::string>(pParams->inputPath, "INPUT", vm, params->verbose)) {
			if (Assign<std::string>(pParams->outputPath, "OUTPUT", vm, params->verbose)) {
			}
			else {
				std::cout << __FUNCTION__ << " failed required --input parameter missing." << std::endl;
				std::cout << "Try MultiMap HELP ERODE for more information." << std::endl;
				success = false;
			}
		}
		else {
			std::cout << __FUNCTION__ << " failed required --input parameter missing." << std::endl;
			std::cout << "Try MultiMap HELP ERODE for more information." << std::endl;
			success = false;
		}

		Assign<unsigned int>(pParams->factor, "FACTOR", vm, params->verbose);
		Assign<unsigned int>(pParams->maxPasses, "PASSES", vm, params->verbose);
		Assign<bool>(pParams->invert, "INVERT", vm, params->verbose);
		Assign<bool>(pParams->binary, "BINARY", vm, params->verbose);

	}
	catch (const std::exception& e) {
		std::cout << __FUNCTION__ << " failed " << e.what() << std::endl;
		success = false;
	}
	return success;
}
bool ErodeParser::GetProperties(void) {
	bool success = true;
	try {
		Assign("verbose", pParams->verbose, pPropertyTree);
		Assign("debug", pParams->debug, pPropertyTree);
		Assign("input", pParams->inputPath, pPropertyTree);
		Assign("output", pParams->outputPath, pPropertyTree);
		Assign("factor", pParams->factor, pPropertyTree);
		Assign("passes", pParams->maxPasses, pPropertyTree);
		Assign("invert", pParams->invert, pPropertyTree);
		Assign("binary", pParams->binary, pPropertyTree);
	}
	catch (const std::exception& e) {
		std::cout << __FUNCTION__ << " failed " << e.what() << std::endl;
		success = false;
	}
	return success;
}
bool ErodeParser::ReadJSON(std::string filename) {
	bool success = true;
	try {

	}
	catch (const std::exception& e) {
		std::cout << __FUNCTION__ << " failed " << e.what() << std::endl;
		success = false;
	}
	return success;
}

MergeParser::MergeParser(Parameters* _params) {
	namespace po = boost::program_options;
	desc.add_options()
		("INPUT,I", po::value<std::string>(), "Input file(s) may use wild-cards")
		("OUTPUT,O", po::value<std::string>(), "Output file")
		("origin", po::value<std::string>(), "Origin (x,y,z) or (x,y) where z=0")
		("clean", "Remove input files after successful creation of output")
		;
	pParams = (MergeParams*)_params;
}
bool MergeParser::AssignOptions(void) {
	bool success = true;
	try {
		Assign<bool>(params->verbose, "verbose", vm);
		Assign<bool>(params->debug, "debug", vm, params->verbose);
		if (Assign<std::string>(pParams->inputPath, "INPUT", vm, params->verbose)) {
			if (Assign<std::string>(pParams->outputPath, "OUTPUT", vm, params->verbose)) {
				std::string tmp;
				if (Assign<std::string>(tmp, "origin", vm, params->verbose)) {
					pParams->origin = GenericPoint<double>(tmp);
				}
				if (vm.count("clean") > 0) {
					pParams->clean = true;
				}
			}
			else {
				std::cout << __FUNCTION__ << " failed required --output parameter missing." << std::endl;
				std::cout << "Try MultiMap HELP DSM for more information." << std::endl;
				success = false;
			}
		}
		else {
			std::cout << __FUNCTION__ << " failed required --input parameter missing." << std::endl;
			std::cout << "Try MultiMap HELP DSM for more information." << std::endl;
			success = false;
		}
	}
	catch (const std::exception& e) {
		std::cout << __FUNCTION__ << " failed " << e.what() << std::endl;
		success = false;
	}
	return success;
}
bool MergeParser::GetProperties(void) {
	bool success = true;
	try {
		Assign("verbose", pParams->verbose, pPropertyTree);
		Assign("debug", pParams->debug, pPropertyTree);
		Assign("input", pParams->inputPath, pPropertyTree);
		Assign("output", pParams->outputPath, pPropertyTree);
		if (pParams->origin.NonZero())
			Assign("origin", pParams->origin.ToString(), pPropertyTree);
		if (pParams->clean)
			Assign("clean", pParams->clean, pPropertyTree);
	}
	catch (const std::exception& e) {
		std::cout << __FUNCTION__ << " failed " << e.what() << std::endl;
		success = false;
	}
	return success;
}
bool MergeParser::ReadJSON(std::string filename) {
	bool success = true;
	try {

	}
	catch (const std::exception& e) {
		std::cout << __FUNCTION__ << " failed " << e.what() << std::endl;
		success = false;
	}
	return success;
}

LASParser::LASParser(Parameters* _params) {
	namespace po = boost::program_options;
	desc.add_options()
		("INPUT,I", po::value<std::string>(), "Input file(s) may use wild-cards")
		("OUTPUT,O", po::value<std::string>(), "Output file or Folder if input wild-carded")
		("crop_window", po::value<std::string>(), "Range filter (minX,minY,maxX,maxY)")
		("resolution", po::value<double>(), "Resolution of grid for DSM production or Laplace filling")
		("mindist", po::value<double>(), "Minimum distance between points filter")
		("smooth_pixels", po::value<size_t>(), "Pixels radius for inverse distance smoothing")
		("keep_original", po::value<bool>()->implicit_value(true), "Use original points rather than interpolated points where possible")
		("fill_class", po::value<int>(), "Classification for filled points in output LAS")
		("include_classes", po::value<std::string>(), "LAS Classification types to include (#,#,#) all others will be omitted")
		("exclude_classes", po::value<std::string>(), "LAS Classification types to exclude (#,#,#) all others will be included")
		;
	pParams = (LASParams*)_params;
}
bool LASParser::AssignOptions(void) {
	bool success = true;
	LASParams* pParams = (LASParams*)params;
	try {
		Assign<bool>(params->verbose, "verbose", vm);
		Assign<bool>(params->debug, "debug", vm, params->verbose);
		if (Assign<std::string>(pParams->inputPath, "INPUT", vm, params->verbose)) {
			if (!Assign<std::string>(pParams->outputPath, "OUTPUT", vm, params->verbose)) {
				std::cout << __FUNCTION__ << " failed required --input parameter missing." << std::endl;
				std::cout << "Try MultiMap HELP LAS for more information." << std::endl;
				success = false;
			}
		}
		else {
			std::cout << __FUNCTION__ << " failed required --input parameter missing." << std::endl;
			std::cout << "Try MultiMap HELP LAS for more information." << std::endl;
			success = false;
		}

		Assign<double>(pParams->resolution, "resolution", vm, params->verbose);
		Assign<double>(pParams->minDistanceFilter, "mindist", vm, params->verbose);
		Assign<size_t>(pParams->smoothingPixels, "smooth_pixels", vm, params->verbose);
		Assign<bool>(pParams->keepOriginalPts, "keep_original", vm, params->verbose);
		Assign<int>(pParams->fillClass, "fill_class", vm, params->verbose);
		if (Assign<std::string>(pParams->rangeString, "crop_window", vm, params->verbose)) {
			SetRange(pParams->range, pParams->rangeString);
		}
		pParams->classesString.clear();
		if (Assign<std::string>(pParams->classesString, "include_classes", vm, params->verbose)) {
			SetInts(pParams->includeClasses, pParams->classesString);
			pParams->classesString.clear();
		}
		if (Assign<std::string>(pParams->classesString, "exclude_classes", vm, params->verbose)) {
			SetInts(pParams->excludeClasses, pParams->classesString);
			pParams->classesString.clear();
		}
	}
	catch (const std::exception& e) {
		std::cout << __FUNCTION__ << " failed " << e.what() << std::endl;
		success = false;
	}
	return success;
}
bool LASParser::GetProperties(void) {
	bool success = true;
	try {
		Assign("verbose", pParams->verbose, pPropertyTree);
		Assign("debug", pParams->debug, pPropertyTree);
		Assign("INPUT", pParams->inputPath, pPropertyTree);
		Assign("OUTPUT", pParams->outputPath, pPropertyTree);
		Assign("resolution", pParams->resolution, pPropertyTree);
		Assign("mindist", pParams->minDistanceFilter, pPropertyTree);
		Assign("smooth_pixels", pParams->smoothingPixels, pPropertyTree);
		Assign("keep_original", pParams->keepOriginalPts, pPropertyTree);
		Assign("fill_class", pParams->fillClass, pPropertyTree);
		if (pParams->range.Valid())
			Assign("crop_window", pParams->range.ToString(), pPropertyTree);
		if (pParams->includeClasses.size() > 0)
			Assign("include_classes", ToString(pParams->includeClasses), pPropertyTree);
		if (pParams->excludeClasses.size() > 0)
			Assign("exclude_classes", ToString(pParams->excludeClasses), pPropertyTree);
	}
	catch (const std::exception& e) {
		std::cout << __FUNCTION__ << " failed " << e.what() << std::endl;
		success = false;
	}
	return success;
}
bool LASParser::ReadJSON(std::string filename) {
	bool success = true;
	try {

	}
	catch (const std::exception& e) {
		std::cout << __FUNCTION__ << " failed " << e.what() << std::endl;
		success = false;
	}
	return success;
}
