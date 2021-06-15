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
#include "SimpleStats.h"
#include <algorithm>

#undef min
#undef max

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

using namespace std;
///////////////////////////////////////////////////
MULTIMAP_API int compareUChar(const void* p1, const void* p2) {
	if ( *(unsigned char*)p1 < *(unsigned char*)p2 ) {
		return -1;
	} else if ( *(unsigned char*)p1 > *(unsigned char*)p2 ) {
		return 1;
	} else {
		return 0;
	}
}
MULTIMAP_API int compareChar(const void* p1, const void* p2) {
	if ( *(char*)p1 < *(char*)p2 ) {
		return -1;
	} else if ( *(char*)p1 > *(char*)p2 ) {
		return 1;
	} else {
		return 0;
	}
}
MULTIMAP_API int compareUShort(const void* p1, const void* p2) {
	if ( *(unsigned short*)p1 < *(unsigned short*)p2 ) {
		return -1;
	} else if ( *(unsigned short*)p1 > *(unsigned short*)p2 ) {
		return 1;
	} else {
		return 0;
	}
}
MULTIMAP_API int compareShort(const void* p1, const void* p2) {
	if ( *(short*)p1 < *(short*)p2 ) {
		return -1;
	} else if ( *(short*)p1 > *(short*)p2 ) {
		return 1;
	} else {
		return 0;
	}
}
MULTIMAP_API int compareUInt(const void* p1, const void* p2) {
	if ( *(unsigned int*)p1 < *(unsigned int*)p2 ) {
		return -1;
	} else if ( *(unsigned int*)p1 > *(unsigned int*)p2 ) {
		return 1;
	} else {
		return 0;
	}
}

MULTIMAP_API int compareInt(const void* p1, const void* p2) {
	if ( *(int*)p1 < *(int*)p2 ) {
		return -1;
	} else if ( *(int*)p1 > *(int*)p2 ) {
		return 1;
	} else {
		return 0;
	}
}
MULTIMAP_API int compareFloat(const void* p1, const void* p2) {
	if ( *(float*)p1 < *(float*)p2 ) {
		return -1;
	} else if ( *(float*)p1 > *(float*)p2 ) {
		return 1;
	} else {
		return 0;
	}
}
MULTIMAP_API int compareDouble(const void* p1, const void* p2) {
	if ( *(double*)p1 < *(double*)p2 ) {
		return -1;
	} else if ( *(double*)p1 > *(double*)p2 ) {
		return 1;
	} else {
		return 0;
	}
}
///////////////////////////////////////////////////
MULTIMAP_API SimpleStats::~SimpleStats(void) {
}

MULTIMAP_API SimpleStats::SimpleStats(void){
	defaults();
}
MULTIMAP_API SimpleStats::SimpleStats(std::string _name, bool _computeMedian, bool _computeStddev){
	defaults();
	computeMedian = _computeMedian;
	computeStddev = _computeStddev;
	name = _name;
}

PRIVATE void SimpleStats::defaults(void){
	noDataSet = false;
	dNoData = -DBL_MAX;
	fNoData = -FLT_MAX;
	memset(&iNoData,0xFFFFFFFF,sizeof(int));
	memset(&uiNoData,0xFFFFFFFF,sizeof(unsigned int));
	memset(&sNoData,0xFFFF,sizeof(short));
	memset(&usNoData,0xFFFF,sizeof(unsigned short));
	memset(&cNoData,0xFFFF,sizeof(char));
	memset(&ucNoData,0xFFFF,sizeof(unsigned char));

	minimum = DBL_MAX;
	maximum = -DBL_MAX;
	sum = 0.0;	
	count = 0;
	computeStddev = false;
	computeMedian = false;
	name = " ";
}

MULTIMAP_API void SimpleStats::SetNoData(double noData) {
	dNoData = noData;
	noDataSet = true;
}
MULTIMAP_API void SimpleStats::SetNoData(float noData) {
	fNoData = noData;
	noDataSet = true;
}
MULTIMAP_API void SimpleStats::SetNoData(int noData) {
	iNoData = noData;
	noDataSet = true;
}
MULTIMAP_API void SimpleStats::SetNoData(unsigned int noData) {
	uiNoData = noData;
	noDataSet = true;
}
MULTIMAP_API void SimpleStats::SetNoData(short noData) {
	sNoData = noData;
	noDataSet = true;
}
MULTIMAP_API void SimpleStats::SetNoData(unsigned short noData) {
	usNoData = noData;
	noDataSet = true;
}
MULTIMAP_API void SimpleStats::SetNoData(char noData) {
	cNoData = noData;
	noDataSet = true;
}
MULTIMAP_API void SimpleStats::SetNoData(unsigned char noData) {
	ucNoData = noData;
	noDataSet = true;
}
MULTIMAP_API void SimpleStats::Compute(string name, double* darray, size_t size){
	this->name = name;
	SimpleStats::Compute(darray,size);
}
MULTIMAP_API void SimpleStats::Compute(std::vector<double> dvec){
	if ( dvec.size() > 0 ) {
		Compute(&dvec[0],dvec.size());
	}
}
MULTIMAP_API void SimpleStats::Compute(string name, std::vector<double> dvec){
	this->name = name;
	SimpleStats::Compute(dvec);
}
MULTIMAP_API void SimpleStats::Compute(string _name, float* farray, size_t size){
	name = _name;
	SimpleStats::Compute(farray,size);
}
MULTIMAP_API void SimpleStats::Compute(string _name, int* iarray, size_t size){
	name = _name;
	SimpleStats::Compute(iarray,size);
}
MULTIMAP_API void SimpleStats::Compute(string _name, unsigned int* iarray, size_t size){
	name = _name;
	SimpleStats::Compute(iarray,size);
}
MULTIMAP_API void SimpleStats::Compute(string _name, short* iarray, size_t size){
	name = _name;
	SimpleStats::Compute(iarray,size);
}
MULTIMAP_API void SimpleStats::Compute(string _name, unsigned short* iarray, size_t size){
	name = _name;
	SimpleStats::Compute(iarray,size);
}
MULTIMAP_API void SimpleStats::Compute(string _name, char* iarray, size_t size){
	name = _name;
	SimpleStats::Compute(iarray,size);
}
MULTIMAP_API void SimpleStats::Compute(string _name, unsigned char* iarray, size_t size){
	name = _name;
	SimpleStats::Compute(iarray,size);
}

MULTIMAP_API void SimpleStats::Compute(unsigned char* iarray, size_t size){
	sum = 0.0;
	count = 0;
	if ( size > 0 ){
		for ( size_t i = 0; i < size; ++i){
			if ( !noDataSet || iarray[i] != ucNoData ) {
				double d = static_cast<double>(iarray[i]);
				count++;
				sum += d;
				minimum = min(minimum,d); 
				maximum = max(maximum,d);
			}
		}
		average = sum/count;

		if ( computeStddev ){
			double sumDevSquared = 0.0;
			for ( size_t i = 0; i < size; ++i){
				if ( !noDataSet || iarray[i] != ucNoData ) {
					double diff = (static_cast<double>(iarray[i])-average);
					sumDevSquared +=  diff*diff;
				}
			}
			stddev = sqrt(sumDevSquared/count);
		}

		if ( computeMedian && !noDataSet ){
			qsort(iarray,size,sizeof(unsigned char),compareUChar);
			size_t mid = size/2;
			median = size % 2 ? (static_cast<double>(iarray[mid]) + static_cast<double>(iarray[mid+1]))/2.0 : static_cast<double>(iarray[mid]);
		}
	}
}
MULTIMAP_API void SimpleStats::Compute(char* iarray, size_t size){
	sum = 0.0;
	count = 0;
	if ( size > 0 ){
		for ( size_t i = 0; i < size; ++i){
			if ( !noDataSet || iarray[i] != cNoData ) {
				double d = static_cast<double>(iarray[i]);
				count++;
				sum += d;
				minimum = min(minimum,d); 
				maximum = max(maximum,d);
			}
		}
		average = sum/count;

		if ( computeStddev ){
			double sumDevSquared = 0.0;
			for ( size_t i = 0; i < size; ++i){
				if ( !noDataSet || iarray[i] != cNoData ) {
					double diff = (static_cast<double>(iarray[i])-average);
					sumDevSquared +=  diff*diff;
				}
			}
			stddev = sqrt(sumDevSquared/count);
		}

		if ( computeMedian && !noDataSet ){
			qsort(iarray,size,sizeof(char),compareChar);
			size_t mid = size/2;
			median = size % 2 ? (static_cast<double>(iarray[mid]) + static_cast<double>(iarray[mid+1]))/2.0 : static_cast<double>(iarray[mid]);
		}
	}
}
MULTIMAP_API void SimpleStats::Compute(unsigned short* iarray, size_t size){
	sum = 0.0;
	count = 0;
	if ( size > 0 ){
		for ( size_t i = 0; i < size; ++i){
			if ( !noDataSet || iarray[i] != usNoData ) {
				double d = static_cast<double>(iarray[i]);
				count++;
				sum += d;
				minimum = min(minimum,d); 
				maximum = max(maximum,d);
			}
		}
		average = sum/count;

		if ( computeStddev ){
			double sumDevSquared = 0.0;
			for ( size_t i = 0; i < size; ++i){
				if ( !noDataSet || iarray[i] != usNoData ) {
					double diff = (static_cast<double>(iarray[i])-average);
					sumDevSquared +=  diff*diff;
				}
			}
			stddev = sqrt(sumDevSquared/count);
		}

		if ( computeMedian && !noDataSet ){
			qsort(iarray,size,sizeof(unsigned short),compareUShort);
			size_t mid = size/2;
			median = size % 2 ? (static_cast<double>(iarray[mid]) + static_cast<double>(iarray[mid+1]))/2.0 : static_cast<double>(iarray[mid]);
		}
	}
}
MULTIMAP_API void SimpleStats::Compute(short* iarray, size_t size){
	sum = 0.0;
	count = 0;
	if ( size > 0 ){
		for ( size_t i = 0; i < size; ++i){
			if ( !noDataSet || iarray[i] != sNoData ) {
				double d = static_cast<double>(iarray[i]);
				count++;
				sum += d;
				minimum = min(minimum,d); 
				maximum = max(maximum,d);
			}
		}
		average = sum/count;

		if ( computeStddev ){
			double sumDevSquared = 0.0;
			for ( size_t i = 0; i < size; ++i){
				if ( !noDataSet || iarray[i] != sNoData ) {
					double diff = (static_cast<double>(iarray[i])-average);
					sumDevSquared +=  diff*diff;
				}
			}
			stddev = sqrt(sumDevSquared/count);
		}

		if ( computeMedian && !noDataSet ){
			qsort(iarray,size,sizeof(short),compareShort);
			size_t mid = size/2;
			median = size % 2 ? (static_cast<double>(iarray[mid]) + static_cast<double>(iarray[mid+1]))/2.0 : static_cast<double>(iarray[mid]);
		}
	}
}
MULTIMAP_API void SimpleStats::Compute(unsigned int* iarray, size_t size){
	sum = 0.0;
	count = 0;
	if ( size > 0 ){
		for ( size_t i = 0; i < size; ++i){
			if ( !noDataSet || iarray[i] != uiNoData ) {
				double d = static_cast<double>(iarray[i]);
				count++;
				sum += d;
				minimum = min(minimum,d); 
				maximum = max(maximum,d);
			}
		}
		average = sum/count;

		if ( computeStddev ){
			double sumDevSquared = 0.0;
			for ( size_t i = 0; i < size; ++i){
				if ( !noDataSet || iarray[i] != uiNoData ) {
					double diff = (static_cast<double>(iarray[i])-average);
					sumDevSquared +=  diff*diff;
				}
			}
			stddev = sqrt(sumDevSquared/count);
		}

		if ( computeMedian && !noDataSet ){
			qsort(iarray,size,sizeof(unsigned int),compareUInt);
			size_t mid = size/2;
			median = size % 2 ? (static_cast<double>(iarray[mid]) + static_cast<double>(iarray[mid+1]))/2.0 : static_cast<double>(iarray[mid]);
		}
	}
}
MULTIMAP_API void SimpleStats::Compute(int* iarray, size_t size){
	sum = 0.0;
	count = 0;
	if ( size > 0 ){
		for ( size_t i = 0; i < size; ++i){
			if ( !noDataSet || iarray[i] != iNoData ) {
				double d = static_cast<double>(iarray[i]);
				count++;
				sum += d;
				minimum = min(minimum,d); 
				maximum = max(maximum,d);
			}
		}
		average = sum/count;

		if ( computeStddev ){
			double sumDevSquared = 0.0;
			for ( size_t i = 0; i < size; ++i){
				if ( !noDataSet || iarray[i] != iNoData ) {
					double diff = (static_cast<double>(iarray[i])-average);
					sumDevSquared +=  diff*diff;
				}
			}
			stddev = sqrt(sumDevSquared/count);
		}

		if ( computeMedian && !noDataSet ){
			qsort(iarray,size,sizeof(int),compareInt);
			size_t mid = size/2;
			median = size % 2 ? (static_cast<double>(iarray[mid]) + static_cast<double>(iarray[mid+1]))/2.0 : static_cast<double>(iarray[mid]);
		}
	}
}

MULTIMAP_API void SimpleStats::Compute(float* farray, size_t size){
	sum = 0.0;
	count = 0;
	if ( size > 0 ){
		for ( size_t i = 0; i < size; ++i){
			if ( !noDataSet || farray[i] != fNoData ) {
				count++;
				sum += farray[i];
				minimum = min(minimum,farray[i]); 
				maximum = max(maximum,farray[i]);
			}
		}
		average = sum/count;

		if ( computeStddev ){
			double sumDevSquared = 0.0;
			for ( size_t i = 0; i < size; ++i){
				if ( !noDataSet || farray[i] != fNoData ) {
					double diff = (static_cast<double>(farray[i])-average);
					sumDevSquared +=  diff*diff;
				}
			}
			stddev = sqrt(sumDevSquared/count);
		}

		if ( computeMedian && !noDataSet ){
			qsort(farray,size,sizeof(float),compareFloat);
			size_t mid = size/2;
			median = size % 2 ? static_cast<double>((farray[mid] + farray[mid+1])/2.0) : static_cast<double>(farray[mid]);
		}
	}
}

MULTIMAP_API void SimpleStats::Compute(double* darray, size_t size){
	sum = 0.0;
	count = 0;
	if ( size > 0 ){
		for ( size_t i = 0; i < size; ++i){
			if ( !noDataSet || darray[i] != dNoData ) {
				count++;
				sum += darray[i];
				minimum = min(minimum,darray[i]); 
				maximum = max(maximum,darray[i]);
			}
		}
		average = sum/count;

		if ( computeStddev ){
			double sumDevSquared = 0.0;
			for ( size_t i = 0; i < size; ++i){
				if ( !noDataSet || darray[i] != dNoData ) {
					double diff = (darray[i]-average);
					sumDevSquared +=  diff*diff;
				}
			}
			stddev = sqrt(sumDevSquared/count);
		}

		if ( computeMedian && !noDataSet ){
			qsort(darray,size,sizeof(double),compareDouble);
			size_t mid = size/2;
			median = size % 2 ? (darray[mid] + darray[mid+1])/2.0 : darray[mid];
		}
	}
}

MULTIMAP_API bool SimpleStats::Compare(SimpleStats & differences, SimpleStats & other, double averageEpsilon) {
	differences.count   = count - other.count;
	differences.minimum = minimum - other.minimum;
	differences.maximum = maximum - other.maximum;
    differences.sum     = sum - other.sum;
	differences.average = average - other.average;
    differences.median  = median - other.median;
	differences.stddev  = stddev - other.stddev;

	bool overEpsilon = false;
	if ( differences.average > averageEpsilon ) {
		overEpsilon = true;
	}
	return overEpsilon;
}

MULTIMAP_API std::string SimpleStats::ToString(){
	std::string info = "";

	if ( count > 0 ) {
		char szString[1024];
		szString[0] = '0';
		char* tabs = NULL;
		if (name.length() < 7 ) {
			tabs = "\t\t";;
		} else {
			tabs = "\t";
		}
		if ( computeStddev && computeMedian && !noDataSet ){
			sprintf(szString,"%s:%smin=%.3lf\tavg=%.3lf\tmax=%.3lf\tmedian=%.3lf\tstddev=%.3lf",name.c_str(),tabs,minimum,average,maximum,median,stddev);
		} else if ( computeStddev ){
			sprintf(szString,"%s:%smin=%.3lf\tavg=%.3lf\tmax=%.3lf\tstddev=%.3lf",name.c_str(),tabs,minimum,average,maximum,stddev);
		} else if ( computeMedian && !noDataSet ){
			sprintf(szString,"%s:%smin=%.3lf\tavg=%.3lf\tmax=%.3lf\tmedian=%.3lf",name.c_str(),tabs,minimum,average,maximum,median);
		} else {
			sprintf(szString,"%s:%smin=%.3lf\tavg=%.3lf\tmax=%.3lf",name.c_str(),tabs,minimum,average,maximum);
		}
		info = std::string(szString);
	}
	return info;
}
