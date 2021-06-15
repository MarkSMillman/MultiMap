#pragma once
#include "MultiMap.h"
#include "VectorTools.h"
#include "MBRect.h"
#include "Logger.h"

DISABLE_WARNINGS
#include "ogr_api.h"
#include "ogr_core.h"
#include "ogrsf_frmts.h"
ENABLE_WARNINGS

VectorTools::~VectorTools(void) {
}
VectorTools::VectorTools(void) {
}


STATIC int getOrdinates(OGRGeometry* pGeometry, double* xyArray, size_t maxArray) {
	int errCode = CE_None;
	return errCode;
}
STATIC int getOrdinates(OGRGeometry* pGeometry, std::vector<double> xyVector) {
	int errCode = CE_None;
	return errCode;
}
STATIC int getCoordinates(OGRGeometry* pGeometry, double* xArray, double* yArray, size_t maxArray) {
	int errCode = CE_None;
	return errCode;
}
STATIC int getCoordinates(OGRGeometry* pGeometry, std::vector<double> xVector, std::vector<double> yVector) {
	int errCode = CE_None;
	return errCode;
}

STATIC int douglasPeucker(OGRGeometry* pGeometry, double tolerance) {
	int errCode = CE_None;
	return errCode;
}
STATIC int douglasPeucker(OGRLayer* pGeometry, double tolerance) {
	int errCode = CE_None;
	return errCode;
}
STATIC int douglasPeucker(OGRGeometry* pOutGeometry, OGRGeometry* pInGeometry, double tolerance) {
	int errCode = CE_None;
	return errCode;
}
STATIC int douglasPeucker(OGRLayer* pOutGeometry, OGRLayer* pInGeometry, double tolerance) {
	int errCode = CE_None;
	return errCode;
}