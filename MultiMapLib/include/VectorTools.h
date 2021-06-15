#pragma once
#include "MultiMap.h"
#include <vector>

class Logger;
class MBRect;
class OGRGeometry;
class OGRLayer;
class VectorTools {
public:
	~VectorTools(void);
	VectorTools(void);

	static int getOrdinates(OGRGeometry* pGeometry, double* xyArray, size_t maxArray);
	static int getOrdinates(OGRGeometry* pGeometry, std::vector<double> xyVector);
	static int getCoordinates(OGRGeometry* pGeometry, double* xArray, double* yArray, size_t maxArray);
	static int getCoordinates(OGRGeometry* pGeometry, std::vector<double> xVector, std::vector<double> yVector);

	static int douglasPeucker(OGRGeometry* pGeometry, double tolerance);
	static int douglasPeucker(OGRLayer* pGeometry, double tolerance);
	static int douglasPeucker(OGRGeometry* pOutGeometry, OGRGeometry* pInGeometry, double tolerance);
	static int douglasPeucker(OGRLayer* pOutGeometry, OGRLayer* pInGeometry, double tolerance);

protected:

	int errCode;
};