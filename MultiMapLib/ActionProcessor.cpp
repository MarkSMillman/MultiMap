#pragma warning ( disable : 4251 )
#include "MultiMap.h"
#include "ActionProcessor.h"
#include "Logger.h"

#include "GDALOCIConnector.h"
#include "GDALImageTools.h"
#include "GDALVectorTools.h"

#include "gdal.h"
#include "gdal_priv.h"
#include "ogr_api.h"
#include "ogr_srs_api.h"
#include "ogr_spatialref.h"
#include "ogrsf_frmts.h"
#include "cpl_string.h"

#include <boost/property_tree/ptree_fwd.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

MULTIMAP_API ActionProcessor::~ActionProcessor() {
}
MULTIMAP_API ActionProcessor::ActionProcessor() {
	logger = Logger::Instance();
}
MULTIMAP_API void ActionProcessor::SetLogFile( std::string logFile ) {
	logger->SetLogFile(logFile);
}
MULTIMAP_API void ActionProcessor::SetLogScreen( bool toScreen ) {
	logger->SetToScreen(toScreen);
}
MULTIMAP_API bool ActionProcessor::Run ( std::string actionFile ) {
	bool status = false;
	std::vector<Action> actions = ReadJSON(actionFile);
	if ( actions.size() > 0 ) {
		status = Run(actions);
	}
	return status;
}
MULTIMAP_API bool ActionProcessor::Run ( std::vector<Action> actions ) {
	bool status = false;
	for ( Action action : actions ) {
		status = Run (action);
		if (  !status ) {
			break;
		}
	}
	return status;
}
MULTIMAP_API bool ActionProcessor::Run ( Action action ) {
	bool status = false;

	return status;
}

PRIVATE std::vector<Action> ActionProcessor::ReadJSON(std::string actionFile) {

	MFUtils mfUtils;
	if ( mfUtils.FileExists(actionFile) == 0 ) {
		using boost::property_tree::iptree;
		boost::property_tree::iptree propertyTree;
		try {
			read_json(actionFile,propertyTree);
			if ( propertyTree.count("processes") ) {    
				iptree processes = propertyTree.get_child("processes");

				BOOST_FOREACH(iptree::value_type &child1, processes) {
					iptree process = static_cast<iptree>(child1.second);

					if (process.count("dataset_connector") > 0 ) {
						connectorString = process.get<std::string>("dataset_connector");
					}
					if (process.count("username") > 0 ) {
						username = process.get<std::string>("username");
					}
					if (process.count("password") > 0 ) {
						password = process.get<std::string>("password");
					}
					if (process.count("tnsname") > 0 ) {
						tnsname = process.get<std::string>("tnsname");
					}
					if (process.count("tables") > 0 ) {
						tables = process.get<std::string>("tables");
					}
					if (process.count("input_folder") > 0 ) {
						inputFolder = process.get<std::string>("input_folder");
					}
					if (process.count("output_folder") > 0 ) {
						outputFolder = process.get<std::string>("output_folder");
					}
					if (process.count("resolution") > 0 ) {
						rasterResolution=process.get<double>("resolution");
					}
					if (process.count("indriver") > 0 ) {
						inDriver = process.get<std::string>("indriver");
					}
					if (process.count("outdriver") > 0 ) {
						outDriver = process.get<std::string>("outdriver");
					}
					if (process.count("inlayer_sql") > 0 ) {
						inLayerSQL = process.get<std::string>("inlayer_sql");
					}
					if (process.count("inlayer_name") > 0 ) {
						inLayerName = process.get<std::string>("inlayer_name");
					}
					if (process.count("outlayer_sql") > 0 ) {
						outLayerSQL = process.get<std::string>("outlayer_sql");
					}
					if (process.count("outlayer_name") > 0 ) {
						outLayerName = process.get<std::string>("outlayer_name");
					}
					if (process.count("verbose") > 0 ) {
						verbose=process.get<bool>("verbose");
					}
					if (process.count("debug") > 0 ) {
						debug=process.get<bool>("debug");
					}
					if (process.count("actions") > 0 ) {
						actions.clear();
						actions.resize(0);

						BOOST_FOREACH(iptree::value_type &child2, process.get_child("actions")) {
							iptree aTree = static_cast<iptree>(child2.second);
							Action action;
							// Default values
							action.rasterResolution = rasterResolution;
							if ( actions.empty() ) {
								action.type = AT_None;
								action.inLayerName = inLayerName;
								action.inLayerSQL = inLayerSQL;
								action.outLayerName = outLayerName;
								action.outLayerSQL = outLayerSQL;
							} else {
								// by default subsequent action input is the previous action output
								// will be over-written by explicit declaration in the JSON file
								Action* previousAction = &actions[actions.size()-1];
								if ( !previousAction->sideAction ) {
									action.inLayerName = previousAction->outLayerName;
									action.inLayerSQL = previousAction->outLayerSQL;
								}
							}
							if (aTree.count("type") > 0 ) {
								std::string atype =aTree.get<std::string>("type");
								atype = mfUtils.ToLower(atype);
								if ( mfUtils.BeginsWith(atype,"simpl",true) ) {
									action.type = AT_Simplify;
								} else if ( mfUtils.BeginsWith(atype,"copy",true) ) {
									action.type = AT_Copy;
								} else if ( mfUtils.BeginsWith(atype,"move",true) ) {
									action.type = AT_Move;
								} else if ( mfUtils.BeginsWith(atype,"del",true) ) {
									action.type = AT_Delete;
								} else if ( mfUtils.BeginsWith(atype,"raster",true) ) {
									action.type = AT_Rasterize;
								} else if ( mfUtils.BeginsWith(atype,"sieve",true) ) {
									action.type = AT_SieveFilter;
								} else if ( mfUtils.BeginsWith(atype,"hill",true) ) {
									action.type = AT_Hillshade;
								} else if ( mfUtils.BeginsWith(atype,"merg",true) ) {
									action.type = AT_Merge;
								} else if ( mfUtils.BeginsWith(atype,"poly",true) ) {
									action.type = AT_Polygonize;
								} else if ( mfUtils.BeginsWith(atype,"cont",true) ) {
									action.type = AT_Contour;
								} else {
									action.type = AT_None;
								}
							}
							if (aTree.count("sideAction") > 0 ) {
								std::string isSideAction =aTree.get<std::string>("sideAction");
								if ( mfUtils.BeginsWith(isSideAction,"true",true) ) {
									action.sideAction = true;
								} else if ( mfUtils.BeginsWith(isSideAction,"yes",true) ) {
									action.sideAction = true;
								} else if ( mfUtils.BeginsWith(isSideAction,"1",true) ) {
									action.sideAction = true;
								} else if ( mfUtils.BeginsWith(isSideAction,"false",true) ) {
									action.sideAction = false;
								} else if ( mfUtils.BeginsWith(isSideAction,"no",true) ) {
									action.sideAction = false;
								} else if ( mfUtils.BeginsWith(isSideAction,"0",true) ) {
									action.sideAction = false;
								}
							}
							if (aTree.count("indriver") > 0 ) {
								action.inDriver = aTree.get<std::string>("indriver");
							}
							if (aTree.count("outdriver") > 0 ) {
								action.outDriver = aTree.get<std::string>("outdriver");
							}
							if (aTree.count("resolution") > 0 ) {
								action.rasterResolution=aTree.get<double>("resolution");
							}
							if (aTree.count("inlayer_sql") > 0 ) {
								action.inLayerSQL = aTree.get<std::string>("inlayer_sql");
							}
							if (aTree.count("inlayer_name") > 0 ) {
								action.inLayerName = aTree.get<std::string>("inlayer_name");
							}
							if (aTree.count("outlayer_sql") > 0 ) {
								action.outLayerSQL = aTree.get<std::string>("outlayer_sql");
							}
							if (aTree.count("outlayer_name") > 0 ) {
								action.outLayerName = aTree.get<std::string>("outlayer_name");
							}
							actions.push_back(action);
						}
					}
				}
			}
		} catch ( const std::exception& e) {
			Logger::Instance()->Log(ERR,"Parsing %s - %s",actionFile.c_str(),e.what());
		}
	}
	return actions;
}