#pragma once
#include "MultiMap.h"

typedef enum ActionType { 
	AT_None = 0, AT_Copy, AT_Move, AT_Delete, AT_Validate, // generic
	AT_Rasterize, AT_SieveFilter, AT_Hillshade, AT_Merge, // raster
	AT_Simplify, AT_Smooth, AT_Polygonize, AT_Contour, // vector
	AT_MAX } ActionType; 

class Action {
public:
	MULTIMAP_API ~Action();
	MULTIMAP_API Action();

	MULTIMAP_API std::string toJSON();
	MULTIMAP_API void toJSON(std::string filePath);

	MULTIMAP_API void setInputLayerSQL(std::string sql);
	MULTIMAP_API std::string getInputLayerSQL();
	MULTIMAP_API void setInputLayerName(std::string name);
	MULTIMAP_API std::string getInputLayerName();
	MULTIMAP_API void setOutputLayerSQL(std::string sql);
	MULTIMAP_API std::string getOutputLayerSQL();
	MULTIMAP_API void setOutputLayerName(std::string name);
	MULTIMAP_API std::string getOutputLayerName();
	MULTIMAP_API void setResolution(double resolution);
	MULTIMAP_API double getResolution();

private:
	ActionType type;
	bool sideAction;
	std::string inDriver;
	std::string outDriver;
	std::string inLayerSQL;
	std::string inLayerName;
	std::string outLayerSQL;
	std::string outLayerName;

	double rasterResolution;

	friend class ActionProcessor;
};