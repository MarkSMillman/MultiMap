#pragma once
#include "MultiMap.h"
#include <string>
#include <vector>
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
class SimpleStats
{
public:
	MULTIMAP_API ~SimpleStats(void);
	MULTIMAP_API SimpleStats(void);
	MULTIMAP_API SimpleStats(std::string name, bool computeMedian = false, bool computeStddev = false);

	MULTIMAP_API void SetNoData(double noData);
	MULTIMAP_API void SetNoData(float noData);
	MULTIMAP_API void SetNoData(int noData);
	MULTIMAP_API void SetNoData(unsigned int noData);
	MULTIMAP_API void SetNoData(short noData);
	MULTIMAP_API void SetNoData(unsigned short noData);
	MULTIMAP_API void SetNoData(char noData);
	MULTIMAP_API void SetNoData(unsigned char noData);

	MULTIMAP_API void Compute(std::vector<double> dvec);
	MULTIMAP_API void Compute(std::string name, std::vector<double> dvec);

	MULTIMAP_API void Compute(double* darray, size_t size);
	MULTIMAP_API void Compute(std::string name, double* darray, size_t size);

	MULTIMAP_API void Compute(float* farray, size_t size);
	MULTIMAP_API void Compute(std::string name, float* farray, size_t size);

	MULTIMAP_API void Compute(int* iarray, size_t size);
	MULTIMAP_API void Compute(std::string name, int* iarray, size_t size);
	
	MULTIMAP_API void Compute(unsigned int* iarray, size_t size);
	MULTIMAP_API void Compute(std::string name, unsigned int* iarray, size_t size);

	MULTIMAP_API void Compute(short* iarray, size_t size);
	MULTIMAP_API void Compute(std::string name, short* iarray, size_t size);
	
	MULTIMAP_API void Compute(unsigned short* iarray, size_t size);
	MULTIMAP_API void Compute(std::string name, unsigned short* iarray, size_t size);

	MULTIMAP_API void Compute(char* iarray, size_t size);
	MULTIMAP_API void Compute(std::string name, char* iarray, size_t size);

	MULTIMAP_API void Compute(unsigned char* iarray, size_t size);
	MULTIMAP_API void Compute(std::string name, unsigned char* iarray, size_t size);

	MULTIMAP_API bool Compare(SimpleStats & differences, SimpleStats & other, double averageEpsilon = 0.0);
	
	MULTIMAP_API std::string ToString();

	std::string name;
	std::string description;

	bool computeStddev;
	bool computeMedian;
	long count;
	double minimum;
	double maximum;
	double sum;
	double average;
	double median;
	double stddev;


private:
	void defaults(void);
	bool noDataSet;
	double dNoData;
	float fNoData;
	int iNoData;
	unsigned int uiNoData;
	short sNoData;
	unsigned short usNoData;
	char cNoData;
	unsigned char ucNoData;
};


