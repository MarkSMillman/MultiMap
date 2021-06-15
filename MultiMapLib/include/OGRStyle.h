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
#pragma once
#include "MultiMap.h"
#include "MFUtils.h"
#include "ogr_core.h"
#include "ogr_featurestyle.h"

class OGRStyle {
public:
	MULTIMAP_API ~OGRStyle(void);
	MULTIMAP_API static OGRStyle* Instance(const char* szStyleSpecification);

	int      priority;

protected:
	OGRStyle(void);

private:
	bool ParsePen(OGRStyle* pStyle);
	bool ParseBrush(OGRStyle* pStyle);
	bool ParseSymbol(OGRStyle* pStyle);
	bool ParseLabel(OGRStyle* pStyle);

	MFUtils         mfUtils;
	OGRSTClassId  styleType;
	OGRStyleMgr     manager;
	OGRSTUnitId    unitType;
	std::string styleString;

	friend class OGRPen;
	friend class OGRBrush;
	friend class OGRSymbol;
	friend class OGRLabel;
};

class OGRPen : public OGRStyle {
public:
	MULTIMAP_API ~OGRPen(void);
	double          width;
	double      perOffset;
	unsigned int    color;
	std::string        id;
	std::string    pattern;
	std::string       cap;
	std::string      join;

protected:
	OGRPen(void);

	friend class OGRStyle;
};
class OGRBrush : public OGRStyle {
public:
	MULTIMAP_API ~OGRBrush(void);
	unsigned int   fColor;
	unsigned int   bColor;
	double          angle;
	double           size;
	double             dx;
	double             dy;
	std::string        id;

protected:

	OGRBrush(void);

	friend class OGRStyle;
};
class OGRSymbol : public OGRStyle {
public:
	MULTIMAP_API ~OGRSymbol(void);
	unsigned int    color;
	unsigned int   oColor;
	double          angle;
	double           size;
	double             dx;
	double             dy;
	double           step;
	double           perp;
	double         offset;
	std::string  fontName;

protected:
	OGRSymbol(void);

	friend class OGRStyle;
};
class OGRLabel : public OGRStyle {
public:
	MULTIMAP_API ~OGRLabel(void);
	unsigned int   fColor;
	unsigned int   bColor;
	unsigned int   hColor;
	unsigned int   oColor;
	double          angle;
	double           size;
	double             dx;
	double             dy;
	double           perp;
	int              bold;
	int            italic;
	int         placement;
	int            anchor;
	int         underline;
	int         strikeout;
	int           stretch;
	int        adjustHorz;
	int        adjustVert;
	std::string  fontName;
	std::string      text;

protected:
	OGRLabel(void);

	friend class OGRStyle;
};