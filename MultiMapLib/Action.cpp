#pragma warning ( disable : 4251 )
#include "MultiMap.h"
#include "Action.h"
#include "Logger.h"

DISABLE_WARNINGS
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string.hpp>
ENABLE_WARNINGS

MULTIMAP_API Action::~Action() {
}
MULTIMAP_API Action::Action() {
	type = AT_None;
	rasterResolution = 1.0;
	sideAction = false;
}

MULTIMAP_API std::string Action::toJSON(void) {
	std::string json;
	boost::property_tree::iptree ptree;

	switch (type) {
	case AT_Simplify:
		ptree.put("type","simplify");
		break;
	case AT_Copy:
		ptree.put("type","copy");
		break;
	case AT_Move:
		ptree.put("type","move");
		break;
	case AT_Delete:
		ptree.put("type","delete");
		break;
	case AT_Rasterize:
		ptree.put("type","rasterize");
		break;
	case AT_SieveFilter:
		ptree.put("type","sievefilter");
		break;
	case AT_Hillshade:
		ptree.put("type","hillshade");
		break;
	case AT_Merge:
		ptree.put("type","merge");
		break;
	case AT_Polygonize:
		ptree.put("type","polygonize");
		break;
	case AT_Contour:
		ptree.put("type","contour");
		break;
	default:
		break;
	}
	if ( sideAction ) {
		ptree.put("side_action","true");
	}
	if ( inDriver.length() > 0 ) {
		ptree.put("indriver",inDriver);
	}
	if ( outDriver.length() > 0 ) {
		ptree.put("outdriver",outDriver);
	}
	if ( rasterResolution > 0 ) {
		ptree.put("resolution",rasterResolution);
	}
	if ( inLayerSQL.length() > 0 ) {
		ptree.put("inlayer_sql",inLayerSQL);
	}
	if ( inLayerName.length() > 0 ) {
		ptree.put("inlayer_name",inLayerName);
	}
	if ( outLayerSQL.length() > 0 ) {
		ptree.put("outlayer_sql",outLayerSQL);
	}
	if ( outLayerName.length() > 0 ) {
		ptree.put("outlayer_name",outLayerName);
	}
	return json;
}
MULTIMAP_API void Action::toJSON(std::string filePath) {
	boost::property_tree::iptree ptree;

	switch (type) {
	case AT_Simplify:
		ptree.put("type","simplify");
		break;
	case AT_Copy:
		ptree.put("type","copy");
		break;
	case AT_Move:
		ptree.put("type","move");
		break;
	case AT_Delete:
		ptree.put("type","delete");
		break;
	case AT_Rasterize:
		ptree.put("type","rasterize");
		break;
	case AT_SieveFilter:
		ptree.put("type","sievefilter");
		break;
	case AT_Hillshade:
		ptree.put("type","hillshade");
		break;
	case AT_Merge:
		ptree.put("type","merge");
		break;
	case AT_Polygonize:
		ptree.put("type","polygonize");
		break;
	case AT_Contour:
		ptree.put("type","contour");
		break;
	default:
		break;
	}
	if ( sideAction ) {
		ptree.put("side_action","true");
	}
	if ( inDriver.length() > 0 ) {
		ptree.put("indriver",inDriver);
	}
	if ( outDriver.length() > 0 ) {
		ptree.put("outdriver",outDriver);
	}
	if ( rasterResolution > 0 ) {
		ptree.put("resolution",rasterResolution);
	}
	if ( inLayerSQL.length() > 0 ) {
		ptree.put("inlayer_sql",inLayerSQL);
	}
	if ( inLayerName.length() > 0 ) {
		ptree.put("inlayer_name",inLayerName);
	}
	if ( outLayerSQL.length() > 0 ) {
		ptree.put("outlayer_sql",outLayerSQL);
	}
	if ( outLayerName.length() > 0 ) {
		ptree.put("outlayer_name",outLayerName);
	}
	try {
		write_json(filePath,ptree);
	} catch ( ... ) {
		Logger* logger = Logger::Instance();
		logger->Log(ERR,"write_json could not write %s",filePath.c_str());
	}
}

MULTIMAP_API void Action::setInputLayerSQL(std::string sql) {
	inLayerSQL = sql;
}
MULTIMAP_API std::string Action::getInputLayerSQL() {
	return inLayerSQL;
}
MULTIMAP_API void Action::setInputLayerName(std::string name) {
	inLayerName = name;
}
MULTIMAP_API std::string Action::getInputLayerName() {
	return inLayerName;
}
MULTIMAP_API void Action::setOutputLayerSQL(std::string sql) {
	outLayerSQL = sql;
}
MULTIMAP_API std::string Action::getOutputLayerSQL() {
	return outLayerSQL;
}
MULTIMAP_API void Action::setOutputLayerName(std::string name) {
	outLayerName = name;
}
MULTIMAP_API std::string Action::getOutputLayerName() {
	return outLayerName;
}
MULTIMAP_API void Action::setResolution(double resolution) {
	rasterResolution = resolution;
}
MULTIMAP_API double Action::getResolution() {
	return rasterResolution;
}