#pragma once
#include "MultiMap.h"

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

	bool computeStddev{ false };
	bool computeMedian{ false };
	long count{ 0 };
	double minimum{ DBL_MAX };
	double maximum{ -DBL_MAX };
	double sum{ 0.0 };
	double average{ 0.0 };
	double median{ 0.0 };
	double stddev{ 0.0 };


private:
	void defaults(void);
	bool noDataSet{ false };
	double dNoData{ -DBL_MAX };
	float fNoData{ -FLT_MAX };
	int iNoData{ -1};
	unsigned int uiNoData{ 0xFFFFFFFF };
	short sNoData{ -1 };
	unsigned short usNoData{ 0xFFFF };
	char cNoData{ -1 };
	unsigned char ucNoData{ 0xFF };
};


