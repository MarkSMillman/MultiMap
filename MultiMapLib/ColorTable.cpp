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
#include "ColorTable.h"

#pragma warning (disable : 4100)

MULTIMAP_API ColorRow::ColorRow(void) {
}

MULTIMAP_API ColorRow::ColorRow(unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a ) 
{ 
	r=_r; 
	g=_g; 
	b=_b; 
	a=_a; 
	color = RGBA_MAKE(r,g,b,a);
};

MULTIMAP_API ColorTable::~ColorTable(void)
{
}
MULTIMAP_API void ColorTable::BuildColorTable(){
	absolute = false;
	rows.clear();

	rows.reserve(256+3*255);
	unsigned char r = 0;
	unsigned char g = 0;
	unsigned char b = 255;
	for ( unsigned int i=0; i<256; i+=ginc ) {
		g = i;
		rows.push_back(ColorRow(r+mix,g,b));
	}

	for ( unsigned int i=1; i<256; i+=binc ) {
		b = 255 - i;
		rows.push_back(ColorRow(r+mix,g,b));
	}

	for ( unsigned int i=1; i<256; i+=rinc ) {
		r = i;
		rows.push_back(ColorRow(r,g,b+mix));
	}

	for ( unsigned int i=1; i<256; i+=ginc ) {
		g = 255 - i;
		rows.push_back(ColorRow(r,g,b+mix));
	}
}

MULTIMAP_API ColorTable::ColorTable(unsigned int _rinc, unsigned int _ginc, unsigned int _binc, unsigned int _mix) {
	floor = 0.f;
	ceil = 0.f;
	range = 0.f;
	rinc = _rinc;
	ginc = _ginc;
	binc = _binc;
	mix  = _mix;
	BuildColorTable();
}

MULTIMAP_API ColorTable::ColorTable(std::string _path) {
	FILE* file = fopen(path.c_str(),"r");
	if ( file != NULL ) {
		rows.clear();
		ColorRow row;
		int rtn = fscanf(file,"%u %u %u",&row.r,&row.g,&row.b);
		rows.push_back(row);

	}
}
MULTIMAP_API void ColorTable::Scale(std::string outTablePath, std::string inTablePath, float factor) {
	FILE* infile = fopen(inTablePath.c_str(),"rt");
	if ( infile != NULL ) {
		FILE* outfile = fopen(outTablePath.c_str(),"wt");
		if ( outfile != NULL ) {
			int elev;
			unsigned int red,green,blue;
			int nc = fscanf(infile,"%d %u %u %u",&elev,&red,&green,&blue);
			while ( nc > 0 ) {
				elev = static_cast<int>(static_cast<float>(elev)*factor);
				fprintf(outfile,"%d %u %u %u\n",elev,red,green,blue);
				nc = fscanf(infile,"%u %u %u %u",&elev,&red,&green,&blue);
			}
			fprintf(outfile,"nv 255 255 255 255");
			fclose(outfile);
		}
		fclose(infile);
	}
}
MULTIMAP_API void ColorTable::Save(std::string colorTablePath) {
	FILE* file = fopen(colorTablePath.c_str(),"wt");
	if ( file != NULL ) {
		double dSize = static_cast<double>(rows.size());
		for ( size_t r=0; r<rows.size(); r++ ) {
			double dfPct = static_cast<double>(r)/dSize * 100;
			if ( r < rows.size()-1 ) {
				fprintf(file,"%5.2f%% %u %u %u\n",dfPct,rows[r].r,rows[r].g,rows[r].b);
			} else {
				fprintf(file,"100.00%% %u %u %u\n",rows[r].r,rows[r].g,rows[r].b);
			}
		}
		fprintf(file,"nv 0 0 0 0");
		fclose(file);
	}
}
MULTIMAP_API void ColorTable::SetRange(double _floor, double _ceil) {
	SetRange(static_cast<float>(_floor), static_cast<float>(_ceil));
}

MULTIMAP_API void ColorTable::SetRange(float _floor, float _ceil) {
	floor = _floor;
	ceil = _ceil;
	range = ceil-floor;
}

/**
* TODO: optimize this.
* this is awful code but it will do for a quickie.
**/
MULTIMAP_API ColorRow* ColorTable::GetRow(float height) {
	ColorRow* crow = NULL;
	if ( !absolute && range > 0 ) {
		int row = static_cast<int>(((height - floor)/range) * rows.size());
		if ( row >= rows.size() ) {
			row--;
		}
		crow = &rows[row];
	} else if ( absolute ) {
		for (size_t i=0; i<rows.size(); i++ ) {
			if ( height >= rows[i].floor && height <= rows[i].ceil ) {
				crow = &rows[i];
				break;
			}
		}
	} 
	return crow;
}

MULTIMAP_API unsigned long ColorTable::GetCOLORREF(float height, unsigned int _alpha) {
	unsigned long color = 0x00000000;
	ColorRow* row = GetRow(height);
	unsigned int blue = row->b;
	unsigned int green = row->g << 8;
	unsigned int red = row->r << 16;
	unsigned int alpha = _alpha << 24;
	color = alpha | red | green | blue;
	return color;
}

MULTIMAP_API ColorRow* ColorTable::GetRow(float height, float floot, float ceil) {
	ColorRow* crow = NULL;
	if ( !absolute && range > 0 ) {
		int row = static_cast<int>(((height - floor)/range) * rows.size());
		crow = &rows[row];
	} 
	else if ( absolute ) {
		for (size_t i=0; i<rows.size(); i++ ) {
			if ( height >= rows[i].floor && height <= rows[i].ceil ) {
				crow = &rows[i];
				break;
			}
		}
	} 
	return crow;
}

MULTIMAP_API ColorRow* ColorTable::GetRow(int crow) {
	return &rows[crow];
}

MULTIMAP_API unsigned long ColorTable::GetColor(float height, float floot, float ceil) {
	int crow = static_cast<int>((height - floor)/range) * static_cast<int>(rows.size());
	return GetColor(crow);
}

MULTIMAP_API unsigned long ColorTable::GetColor(float height) {
	int crow = static_cast<int>((height - floor)/range) * static_cast<int>(rows.size());
	return GetColor(crow);
}

MULTIMAP_API unsigned long ColorTable::GetColor(int crow) {
	if ( crow < 0 || crow > rows.size() ) {
		return NULL;
	} else {
		return rows[crow].color;
	}
}

MULTIMAP_API void ColorTable::Dump(std::string outputFilePath, int mode) {
	FILE* f = fopen(outputFilePath.c_str(),"wt");
	if ( f ) {
		float frows = static_cast<float>(rows.size());
		for ( size_t r=0;r<rows.size();r++) {
			if ( mode != 0 && range > 0.f) {
				float frow = static_cast<float>(r);
				float elev = frow/frows*range + floor;
				fprintf(f,"%.0f ",elev);
			}
			fprintf(f,"%3u %3u %3u\n",rows[r].r,rows[r].g,rows[r].b);
		}
		fclose(f);
	}
}