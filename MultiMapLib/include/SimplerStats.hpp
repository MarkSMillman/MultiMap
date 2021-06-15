#pragma once
#include "MultiMap.h"

template <class T>
class SimplerStats {
public:
	~SimplerStats(void) {
	}

	SimplerStats(std::string _name = "") {
		name = _name;
		noData = -INT_MAX;
		minimum = static_cast<T>(ULLONG_MAX);
		if ( std::numeric_limits<T>::is_signed ) {
			maximum = -minimum;
		} else {
			maximum = 0;
		}
		sum =   0.0;	
		count = 0;
	}

	void ComputeVector(std::vector<T> data) {
		if ( data.size() > 0 ) {
			Compute(&data[0],data.size());
		}
	}
	void DataInstance(T data) {
		if ( noData == -INT_MAX || data != static_cast<T>(noData) ) {
			count++;
			sum += data;
			minimum = min(minimum,data); 
			maximum = max(maximum,data);
		}
	}
	void ComputeMean() {
		mean = sum/static_cast<double>(count);
	}
	void Compute(T* data, size_t size) {
		sum   = 0;
		count = 0;
		if ( size > 0 ){
			for ( size_t i = 0; i < size; ++i){
				if ( noData == -INT_MAX || data[i] != static_cast<T>(noData) ) {
					count++;
					sum += data[i];
					minimum = min(minimum,data[i]); 
					maximum = max(maximum,data[i]);
				}
			}
			mean = sum/static_cast<double>(count);
		}
	}

	void SetNoData(int _noData) {
		noData = _noData;
	}

	std::string toString(){
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

			sprintf(szString,"%s:%smin=%.3lf\tavg=%.3lf\tmax=%.3lf",name.c_str(),tabs,minimum,mean,maximum);

			info = std::string(szString);
		}
		return info;
	}

	std::string name;
	size_t count;
	T minimum;
	T maximum;
	double sum;
	double mean;

private:
	int noData;
};