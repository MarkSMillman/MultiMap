#pragma once
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
#include <string>
#include <vector>
// These named colors should be replaced with a larger file based list.
// @see http://cloford.com/resources/colours/500col.htm
// @see http://cloford.com/resources/colours/namedcol.htm
// @see http://en.wikipedia.org/wiki/Web_colors
// @see http://www.standardista.com/css3/cssnamed-hsl-and-rgb-colors/
// @see http://en.wikipedia.org/wiki/List_of_colors:_A%E2%80%93F
// @see http://en.wikipedia.org/wiki/List_of_colors:_G%E2%80%93M
// @see http://en.wikipedia.org/wiki/List_of_colors:_N%E2%80%93Z

typedef struct fNamedColor {
	const char *name;
	float r, g, b;
} fNamedColor;

static const fNamedColor fNamedColors[] = {
	{ "white",  1.00, 1.00, 1.00 },
	{ "black",  0.00, 0.00, 0.00 },
	{ "red",    1.00, 0.00, 0.00 },
	{ "green",  0.00, 1.00, 0.00 },
	{ "blue",   0.00, 0.00, 1.00 },
	{ "yellow", 1.00, 1.00, 0.00 },
	{ "magenta",1.00, 0.00, 1.00 },
	{ "cyan",   0.00, 1.00, 1.00 },
	{ "aqua",   0.00, 0.75, 0.75 },
	{ "grey",   0.75, 0.75, 0.75 },
	{ "gray",   0.75, 0.75, 0.75 },
	{ "orange", 1.00, 0.50, 0.00 },
	{ "brown",  0.75, 0.50, 0.25 },
	{ "purple", 0.50, 0.00, 1.00 },
	{ "violet", 0.50, 0.00, 1.00 },
	{ "indigo", 0.00, 0.50, 1.00 },
};
typedef struct ucNamedColor {
	const char *name;
	unsigned char r, g, b;
} ucNamedColor;

static const ucNamedColor ucNamedColors[] = {
	{ "white",  255, 255, 255 },
	{ "black",  000, 000, 000 },
	{ "red",    255, 000, 000 },
	{ "green",  000, 255, 000 },
	{ "blue",   000, 000, 255 },
	{ "yellow", 255, 255, 000 },
	{ "magenta",255, 000, 255 },
	{ "cyan",   000, 255, 255 },
	{ "aqua",   000, 191, 191 },
	{ "grey",   191, 191, 191 },
	{ "gray",   191, 191, 191 },
	{ "orange", 255, 127, 000 },
	{ "brown",  191, 127, 064 },
	{ "purple", 127, 000, 255 },
	{ "violet", 127, 000, 255 },
	{ "indigo", 000, 127, 255 },
};

#ifndef RGBA_MAKE
#define RGBA_MAKE(r, g, b, a)   ((unsigned long) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b)))
#endif

typedef struct {
	unsigned char a;
	unsigned char r;
	unsigned char g;
	unsigned char b;
} COLOR_ARGB;

typedef union {
	COLOR_ARGB     argb;
	unsigned int word32;
} COLOR_ARGU;

class ColorRow 
{
public:
	MULTIMAP_API ColorRow(void);
	MULTIMAP_API ColorRow(unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a = 255 );

	float floor;
	float ceil;
	unsigned long color;
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
};

class ColorTable
{
public:
	MULTIMAP_API ~ColorTable(void);
	MULTIMAP_API ColorTable(unsigned int rinc = 1, unsigned int ginc = 1, unsigned int binc = 1, unsigned int mix = 0);
	MULTIMAP_API ColorTable(std::string colorTablePath);
	MULTIMAP_API void Save(std::string colorTablePath);
	MULTIMAP_API void SetRange(float _floor, float _ceil);
	MULTIMAP_API void SetRange(double _floor, double _ceil);
    MULTIMAP_API static void Scale(std::string outTablePath, std::string inTablePath, float factor) ;

	MULTIMAP_API void Dump(std::string outputFilePath, int mode = 0);

	MULTIMAP_API ColorRow* GetRow(float height, float floor, float ceil);
	MULTIMAP_API ColorRow* GetRow(float height);
	MULTIMAP_API ColorRow* GetRow(int row);
	MULTIMAP_API unsigned long GetCOLORREF(float height, unsigned int alpha = 0XFF);

	MULTIMAP_API unsigned long GetColor(float height, float floor, float ceil);
	MULTIMAP_API unsigned long GetColor(float height);
	MULTIMAP_API unsigned long GetColor(int row);

	bool absolute;
	unsigned int rinc;
	unsigned int ginc;
	unsigned int binc;
	unsigned int mix;
	float floor;
	float ceil;
	float range;
	std::string name;
	std::string path;
	std::vector<ColorRow> rows;

private:
	MULTIMAP_API void ColorTable::BuildColorTable();
};

