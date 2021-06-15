#pragma once
#include "MultiMap.h"

DISABLE_WARNINGS
#include <limits>
#include <assert.h>
ENABLE_WARNINGS

template <class T>
class QuickHistogram {
public:
	QuickHistogram::~QuickHistogram(void) {
		if ( bins ) {
			delete[] bins;
			bins = NULL;
			binCount = 0;
		}
		if ( histOutput ) {
			delete[] histOutput;
		}
	}
	QuickHistogram::QuickHistogram(size_t _binCount = 64, std::string _name = "") {
		binCount = _binCount;
		name = _name;
		bins = new size_t[binCount];
		memset(bins,0,sizeof(size_t)*binCount);
		histOutput= NULL;
		hSize = 0;

		minValue = static_cast<T>(ULLONG_MAX); // 10+e19 which should be big enough
		if ( std::numeric_limits<T>::is_signed ) {
			maxValue = -minValue;
		} else {
			maxValue = 0;
		}
		meanValue = 0;
		noData = -32767;
		edgeData = 32767;
		maxBinValue = 0;
		noDataCount = 0;
		edgeCount = 0;

		pc50 = 0.f;
		pc80 = 0.f;
		pc90 = 0.f;
		pc95 = 0.f;
		pc98 = 0.f;
		pc99 = 0.f;
		ranked = false;
	}

	QuickHistogram(const QuickHistogram<T> & other);
	QuickHistogram& QuickHistogram::operator=(const QuickHistogram & other);

	void QuickHistogram::FindEdgeNoDataArray(T* data, int xSize, int ySize) {
		int y=0;
		for ( int x=0; x<xSize; x++ ) {
			int c = y*xSize + x;
			if ( data[c] == noData ) {
				data[c] = static_cast<T>(edgeData);
				edgeCount++;
			} else {
				break;
			}
		}

		for ( int x=xSize-1; x>=0; x-- ) {
			int c = y*xSize + x;
			if ( data[c] == noData ) {
				data[c] = static_cast<T>(edgeData);
				edgeCount++;
			} else if (data[c] != edgeData) {
				break;
			}
		}

		y=ySize-1;
		for ( int x=0; x<xSize; x++ ) {
			int c = y*xSize + x;
			if ( data[c] == noData ) {
				data[c] = static_cast<T>(edgeData);
				edgeCount++;
			} else if (data[c] != edgeData) {
				break;
			}
		}

		for ( int x=xSize-1; x>=0; x-- ) {
			int c = y*xSize + x;
			if ( data[c] == noData ) {
				data[c] = static_cast<T>(edgeData);
				edgeCount++;
			} else if (data[c] != edgeData) {
				break;
			}
		}

		int x=0;
		for ( int y=0; y<ySize; y++ ) {
			int c = y*ySize + y;
			if ( data[c] == noData ) {
				data[c] = static_cast<T>(edgeData);
				edgeCount++;
			} else if (data[c] != edgeData) {
				break;
			}
		}

		for ( int y=ySize-1; y>=0; y-- ) {
			int c = y*ySize + y;
			if ( data[c] == noData ) {
				data[c] = static_cast<T>(edgeData);
				edgeCount++;
			} else if (data[c] != edgeData) {
				break;
			}
		}

		x=xSize-1;
		for ( int y=0; y<ySize; y++ ) {
			int c = y*ySize + y;
			if ( data[c] == noData ) {
				data[c] = static_cast<T>(edgeData);
				edgeCount++;
			} else if (data[c] != edgeData) {
				break;
			}
		}

		for ( int y=ySize-1; y>=0; y-- ) {
			int c = y*ySize + y;
			if ( data[c] == noData ) {
				data[c] = static_cast<T>(edgeData);
				edgeCount++;
			} else if (data[c] != edgeData) {
				break;
			}
		}	
	}

	void QuickHistogram::ResetEdgeNoDataArray(T* data, size_t dataCount) { 
		for ( size_t d=0; d<dataCount; d++ ) {
			if ( data[d] == edgeData ) {
				data[d] = noData;
			}
		}
	}

	void QuickHistogram::Array(T* data, size_t _xSize, size_t _ySize) { 
		xSize = static_cast<int>(_xSize);
		ySize = static_cast<int>(_ySize);
		size_t dataCount = xSize * ySize;
		validCount = 0;
		edgeCount = 0;
		FindEdgeNoDataArray(data, xSize, ySize);
		for ( size_t d=0; d<dataCount; d++ ) {
			if ( data[d] != noData  && data[d] != edgeData ) {
				minValue = min (minValue,data[d]);
				maxValue = max (maxValue,data[d]);
				meanValue = (static_cast<double>(meanValue*validCount) + static_cast<double>(data[d])) / static_cast<double>(validCount+1);
				validCount++;
			}
		}
		interval = ( maxValue - minValue ) / binCount;
		maxBinValue = 0;
		noDataCount = 0;
		edgeCount = 0;
		double sumDiffSqrd = 0;
		for ( size_t d=0; d<dataCount; d++ ) {
			if ( data[d] != noData && data[d] != edgeData ) {
				double diff = data[d] - meanValue;
				sumDiffSqrd += diff*diff;
				size_t bin = size_t( floor((data[d]-minValue) / interval ));
				if ( bin == binCount ) 
					bin--;
				if ( bin >= binCount ) {
					assert (bin<binCount);
				}
				bins[bin]++;
				if ( bins[bin] > maxBinValue ) {
					maxBinValue = bins[bin];
					maxBin = bin;
				}
			} else if ( data[d] == noData ) {
				noDataCount++;
			} else {
				edgeCount++;
			}
		}
		rowCount = dataCount;
		// Population Standard Deviation
		stdDev = sqrt(sumDiffSqrd / validCount);
	}

	void QuickHistogram::Rank(void) {

		if ( !ranked && bins && validCount > 0 ) {
			double dValidCount = static_cast<double>(validCount);
			double dTotal = 0.0;
			double dInterval = static_cast<double>(interval);
			double dDifference = 0.0;
			for ( size_t b=0; b<binCount; b++ ) {
				dTotal += static_cast<double>(bins[b]);
				dDifference += dInterval;
				double ratio = dTotal / dValidCount;
				if ( pc50 == 0.f && ratio >= 0.50 ) {
					pc50 = static_cast<float>(dDifference);
				} else if ( pc80 == 0.f && ratio >= 0.80 ) {
					pc80 = static_cast<float>(dDifference);
				} else if ( pc90 == 0.f && ratio >= 0.90 ) {
					pc90 = static_cast<float>(dDifference);
				} else if ( pc95 == 0.f && ratio >= 0.95 ) {
					pc95 = static_cast<float>(dDifference);
				} else if ( pc98 == 0.f && ratio >= 0.98 ) {
					pc98 = static_cast<float>(dDifference);
				} else if ( pc99 == 0.f && ratio >= 0.99 ) {
					pc99 = static_cast<float>(dDifference);
					break;
				}
			}
		}
		ranked = true;
	}

	void QuickHistogram::BuildOutput(unsigned int height=48) {
		if ( histOutput ) {
			delete histOutput;
		}
		hSize = binCount * height + 1;
		if ( bins && binCount > 0 ) {
			minBinValue = ULLONG_MAX;
			for ( size_t b=0; b<binCount; b++ ) {
				minBinValue = std::min(minBinValue,bins[b]);
			}
			size_t binRange = maxBinValue - minBinValue + 1;
			heightScale = static_cast<float>(binRange) / static_cast<float>(height);


			histOutput = new char[hSize];
			memset(histOutput,0,hSize);
			for ( size_t x=0; x<binCount; x++ ) {
				size_t ym = static_cast<size_t>((static_cast<float>(bins[x]-minBinValue+1)/heightScale));
				for ( size_t y=0; y<ym; y++ ) {
					size_t h = y*binCount + x;
					if ( h >= hSize ) {
						int x=1;
					}
					histOutput[h] = 1;
				}
			}
		}
	}
	void QuickHistogram::Report(bool histogram = false, bool rank = false, FILE* pFile=stdout) {
		if ( name.length() > 0 ) {
			fprintf(pFile,"Analysis of %s\n",name.c_str());
		}
		fprintf(pFile,"Minimum            %lf\n",static_cast<double>(minValue));
		fprintf(pFile,"Maximum            %lf\n",static_cast<double>(maxValue));
		fprintf(pFile,"Mean               %lf\n",meanValue);
		fprintf(pFile,"Standard Deviation %lf\n",stdDev);
		fprintf(pFile,"Total cell count   %lu\n",rowCount);
		fprintf(pFile,"Valid cell count   %lu\n",validCount);
		fprintf(pFile,"No data cells edge %lu\n",edgeCount);
		fprintf(pFile,"No data cells      %lu\n",noDataCount);
		if ( rank ) {
			Rank();
			fprintf(pFile,"Differences (max-min)\n");
			fprintf(pFile,"50%% within         %.4lf\n",pc50); 
			fprintf(pFile,"80%% within         %.4lf\n",pc80); 
			fprintf(pFile,"90%% within         %.4lf\n",pc90); 
			fprintf(pFile,"95%% within         %.4lf\n",pc95); 
			fprintf(pFile,"98%% within         %.4lf\n",pc98); 
			fprintf(pFile,"99%% within         %.4lf\n",pc99); 
		}
		if ( histogram ) {
			bool newway = true;
			if ( newway ) {
				unsigned int height = 32;
				BuildOutput(height);
				size_t labelWidthY = static_cast<size_t>(log10(maxBinValue)) + 2;
				size_t labelRounding = static_cast<size_t>(pow(10,(static_cast<size_t>(log10(maxBinValue)) - 2)));
				fprintf(pFile,"Histogram\n");
				for ( int y=height-1; y>=0; y-- ) {
					if ( y%5 == 0 ) {
						size_t val = static_cast<size_t>(y*heightScale + minBinValue-1);
						val = val/labelRounding * labelRounding;
						fprintf(pFile,"%*d | ",labelWidthY,val);
					} else {
						fprintf(pFile,"%*c | ",labelWidthY,' ');
					}
					for ( size_t x=0; x<binCount; x++ ) {
						size_t h = y*binCount + x;
						if ( histOutput[h] ) {
							fprintf(pFile,"*");
						} else {
							fprintf(pFile," ");
						}
					}
					fprintf(pFile,"\n");
				}
			} else {
				double scalar = 1.0;
				if ( maxBinValue > 80 ) {
					scalar = 80.0/static_cast<double>(maxBinValue);
				}
				fprintf(pFile,"Histogram\n");
				double base = minValue;
				for ( size_t b=0; b<binCount; b++ ) {
					size_t bc = static_cast<size_t>(static_cast<double>(bins[b])*scalar+0.5);
					base = minValue + b*interval;
					fprintf(pFile,"%02.4lf - %02.4lf %4u : ",base,base+interval,b);
					for ( size_t dot=0; dot<bc; dot++ ) {
						fprintf(pFile,"*");
					}
					fprintf(pFile,"\n");
				}
			}
		}
	}

	int            xSize;
	int            ySize;
	int           noData;
	int         edgeData;
	size_t   noDataCount;
	size_t     edgeCount;
	size_t    validCount;
	size_t      rowCount;
	size_t         *bins;
	size_t      binCount;
	T           minValue;
	T           maxValue;
	T           interval;
	double     meanValue;
	double        stdDev;

	float           pc50;
	float           pc80;
	float           pc90;
	float           pc95;
	float           pc98;
	float           pc99;
	bool          ranked;

private:
	std::string     name;
	size_t        maxBin;
	size_t   maxBinValue;
	size_t   minBinValue;
	float    heightScale;
	size_t         hSize;
	char*     histOutput;
};

template <class T> QuickHistogram<T>::QuickHistogram(const QuickHistogram<T> & other) {
	*this = other;
}

template <class T> QuickHistogram<T>& QuickHistogram<T>::operator=(const QuickHistogram<T>& other) {
	xSize = other.xSize;
	ySize = other.ySize;
	noData = other.noData;
	edgeData = other.edgeData;
	noDataCount = other.noDataCount;
	edgeCount = other.edgeCount;
	validCount = other.validCount;
	rowCount = other.rowCount;

	minValue = other.minValue;
	maxValue = other.maxValue;
	interval = other.interval;
	meanValue = other.meanValue;
	stdDev = other.stdDev;

	pc50 = other.pc50;
	pc80 = other.pc80;
	pc90 = other.pc90;
	pc95 = other.pc95;
	pc98 = other.pc98;
	pc99 = other.pc99;
	ranked = other.ranked;

	name = other.name;
	maxBin = other.maxBin;
	maxBinValue = other.maxBinValue;
	minBinValue = other.minBinValue;
	heightScale = other.heightScale;
	if ( other.histOutput ) {
		hSize = other.hSize;
		histOutput = new char[hSize];
		memcpy(histOutput,other.histOutput,hSize);
	} else {
		hSize = 0;
		histOutput = NULL;
	}
	if ( other.bins ) {
		binCount = other.binCount;
		bins = new size_t[binCount];
		memcpy(bins,other.bins,binCount*sizeof(size_t));
	} else {
		binCount = 0;
		bins = NULL;
	}
	return *this;
}
