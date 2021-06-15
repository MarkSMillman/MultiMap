/**
* Copyright 2014 Mizar, LLC
* All Rights Reserved.
*
* This file is part of Mizar's MultiMap software library.
* MultiMap is licensed under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version a copy of which is available at http://www.gnu.org/licenses/
*
* MultiMap is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You may NOT remove this copyright notice; it must be retained in any modified 
* version of the software.
**/
#include "MultiMap.h"
#include "Parameters.h"

MULTIMAP_API Parameters::~Parameters(void) {;}
MULTIMAP_API Parameters::Parameters(void) {
	command = CMD_HELP;
	verbose = false;
	debug = false;
	commandNames.resize(CMD_MAX);
	commandNames[CMD_HELP].assign("Help");
	commandNames[CMD_ERODE].assign("Erode");
	commandNames[CMD_MERGE].assign("Merge");
	commandNames[CMD_INVALID].assign("Invalid");
}

MULTIMAP_API DLMParams::DLMParams(void) {
	outClassNum = 0;
	resolution = 1.0;
	fill = true;
	erode = false;
	keepOrig = false;
	thinOnly = true;
}
MULTIMAP_API DLMParams::DLMParams(const DLMParams& other) {
	*this = other;
}
MULTIMAP_API DLMParams& DLMParams::operator=(const DLMParams& other) {
	outTableName = other.outTableName;
	inTableName = other.inTableName;
	geomColumn = other.geomColumn;
	attrColumns = other.attrColumns;
	classField = other.classField;
	outClassName = other.outClassName;
	outClassNum = other.outClassNum;
	inClassNum = other.inClassNum;
	productId = other.productId;
	resolution = other.resolution;
	fill = other.fill;
	erode = other.erode;
	keepOrig = other.keepOrig;
	thinOnly = other.thinOnly;
	range = other.range;
	
	return *this;
}