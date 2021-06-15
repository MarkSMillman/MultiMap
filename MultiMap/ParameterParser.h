#pragma once
#include "Parameters.h"

#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

class ParameterParser {
public:
	~ParameterParser(void);
	ParameterParser(void);
	static Parameters* Parse(int argc, char* argv[], ParameterParser** _cmdParser = NULL, MULTIMAP_CMD *about = NULL);
	bool WriteJSON(std::string filename="");

	virtual bool ReadJSON(std::string filename) = 0;
	virtual bool GetProperties(void) = 0;

protected:
	virtual bool AssignOptions(void);

	template <typename T>
	bool Assign(T & value,std::string key,boost::program_options::variables_map & vm, bool verbose=false) {
		bool assigned = false;
		if (vm.count(key)) {	
			value = vm[key].as<T>(); 
			assigned = true; 
		}
		return assigned;
	}
	template <typename T>
	void Assign(std::string key,T value,boost::property_tree::iptree *ptree) { 
		ptree->put(key,value);
	}
	template <typename T>
	void Assign(T & value, std::string key,boost::property_tree::iptree & ptree) {
		bool assigned = false;
		if ( ptree.count(key) ) {
			value = ptree.get<T>(key);
			assigned = true;
		}
		return assigned;
	}

	MFUtils mfUtils;
	boost::program_options::variables_map vm;
	boost::program_options::options_description desc;
	boost::property_tree::iptree* pPropertyTree;

	ParameterParser* cmdParser;
	Parameters* params;

protected:
	bool SetRange(MBRect & range, std::string input);
	void SetInts(std::vector<int> & ints, std::string input);
	std::string ToString(std::vector<int> ints);
private:
	void Parse(void);
	bool ParseOptions(int argc, char* argv[]);
	MULTIMAP_CMD about;
	std::vector<std::string> commandNames;
};

// Try to keep these alphabetically organized
class DLMParser : public ParameterParser {
public:
	DLMParser(Parameters* _params = NULL);
	bool AssignOptions();
	bool GetProperties();
	bool ReadJSON(std::string filename);
	DLMParams* pParams;
};

class ErodeParser : public ParameterParser {
public:
	ErodeParser(Parameters* _params = NULL);
	bool AssignOptions();
	bool GetProperties();
	bool ReadJSON(std::string filename);
	ErodeParams* pParams;
};

class MergeParser : public ParameterParser {
public:
	MergeParser(Parameters* _params = NULL);
	bool AssignOptions(void);
	bool GetProperties();
	bool ReadJSON(std::string filename);
	MergeParams* pParams;
};

class LASParser : public ParameterParser {
public:
	LASParser(Parameters* _params = NULL);
	bool AssignOptions();
	bool GetProperties();
	bool ReadJSON(std::string filename);
	LASParams* pParams;
};