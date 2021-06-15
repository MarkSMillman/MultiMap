#pragma once

#include "MultiMap.h"
#include "GenericPoint.hpp"

typedef enum {   // used by Adjacent
	RPO_NONE=0, 
	RPO_COVERED, RPO_EAST, RPO_WEST, RPO_NORTH, RPO_SOUTH, RPO_NORTHEAST, RPO_NORTHWEST, RPO_SOUTHEAST, RPO_SOUTHWEST,
	RPO_SIZEOF
} RPointOrientation;

typedef enum { MBRLL, MBRLR, MBRUL, MBRUR } MbrCorner;

/**
 * @see ..\OGC_Operations.png
**/
#define DblPoint GenericPoint<double>
class MBRect {
public:
	MULTIMAP_API ~MBRect(void);
	MULTIMAP_API MBRect(void);
	MULTIMAP_API MBRect(std::string sMBR); //// WKT format (minX minY,maxX maxY) or (minX minY minZ,maxX maxY maxZ)
	MULTIMAP_API MBRect(DblPoint minXY, DblPoint maxXY);
	MULTIMAP_API MBRect(double minX, double minY, double maxX, double maxY);
	MULTIMAP_API MBRect(double minX, double minY, double minZ, double maxX, double maxY, double maxZ);

	MULTIMAP_API MBRect(float minX, float minY, float maxX, float maxY);
	MULTIMAP_API MBRect(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);

	MULTIMAP_API MBRect(const MBRect& other);
	MULTIMAP_API MBRect& operator=(const MBRect& other);
	MULTIMAP_API bool operator==(const MBRect& other);
	MULTIMAP_API bool operator!=(const MBRect& other);

	MULTIMAP_API bool Compare(MBRect & other, double precision, double *maxDifference = NULL);
	MULTIMAP_API bool Valid(void);

	MULTIMAP_API void Reset(void);
	MULTIMAP_API void ResetIfInvalid(void);
	MULTIMAP_API void Align(unsigned long alignmentFactor);
	MULTIMAP_API void Expand(double x, double y = DBL_MAX, double z=0.0);
	MULTIMAP_API void Split(std::vector<MBRect> & parts, size_t xParts, size_t yParts, size_t roundTo = 0);
	MULTIMAP_API double Distance2d(DblPoint point);
	MULTIMAP_API double XRange(void);
	MULTIMAP_API double YRange(void);
	MULTIMAP_API double ZRange(void);

	// OGC Operators
	MULTIMAP_API bool Equal(MBRect & other);
	MULTIMAP_API bool Equal(MBRect *other);
	MULTIMAP_API bool AnyInteract(MBRect & other);
	MULTIMAP_API bool AnyInteract(MBRect *other);
	MULTIMAP_API bool AnyInteract(DblPoint & point);
	MULTIMAP_API bool AnyInteract(DblPoint *point);

	MULTIMAP_API bool Covers(MBRect & other); // other is inside of this includes touching
	MULTIMAP_API bool Covers(MBRect *other);
	MULTIMAP_API bool Covers(DblPoint & point);
	MULTIMAP_API bool Covers(DblPoint *point);
	MULTIMAP_API bool CoveredBy(MBRect & other); // inverse of Covers 
	MULTIMAP_API bool CoveredBy(MBRect *other);

	MULTIMAP_API bool Contains(DblPoint & point); // this is completely inside of other no touching
	MULTIMAP_API bool Contains(DblPoint *point);
	MULTIMAP_API bool Inside(MBRect & other); // this is completely inside of other no touching
	MULTIMAP_API bool Inside(MBRect *other);
	MULTIMAP_API bool Contains(MBRect & other); // inverse of Inside no touching other is completely inside of this no touching
	MULTIMAP_API bool Contains(MBRect *other);

	MULTIMAP_API bool Overlaps(MBRect & other); // edges of this intersect edges of other
	MULTIMAP_API bool Overlaps(MBRect *other);

	/* TO DO 
	MULTIMAP_API bool Touches(MBRect & other); // this touches other on an edge or corner but there is no overlap
	MULTIMAP_API bool Touches(MBRect *other);
	*/

	MULTIMAP_API double Area(void);
	MULTIMAP_API DblPoint Center(void);

	MULTIMAP_API RPointOrientation Adjacent(MBRect & other); // special form of touches used for grids
	MULTIMAP_API RPointOrientation Adjacent(MBRect *other);

	MULTIMAP_API RPointOrientation Adjacent(DblPoint & point);

	// Indexes is very similar to 
	MULTIMAP_API bool Indexes(DblPoint & point);
	MULTIMAP_API bool Indexes(DblPoint *point);

	MULTIMAP_API void Update(MBRect & other);
	MULTIMAP_API void Update(MBRect *other);
	MULTIMAP_API void Update(DblPoint & point);
	MULTIMAP_API void Update(DblPoint *point);
	MULTIMAP_API void Update(double x, double y, double z=0.0);
	MULTIMAP_API void Update(std::vector<DblPoint> & points);
	MULTIMAP_API void UpdateXY(std::vector<DblPoint> & points);
	MULTIMAP_API void UpdateZ(std::vector<DblPoint> & points);

	MULTIMAP_API MBRect Round(void);
	MULTIMAP_API MBRect RoundXY(void);

	MULTIMAP_API bool Intersect(MBRect & intersection, MBRect & other);
	MULTIMAP_API bool Intersect(MBRect & intersection, MBRect *other);
	MULTIMAP_API MBRect Union(MBRect & other);
	MULTIMAP_API MBRect Union(MBRect *other);
	MULTIMAP_API static MBRect Union(std::vector<std::string> files, bool fromHeaders = true);

	MULTIMAP_API std::string ToString(int dim=2);
	MULTIMAP_API DblPoint CornerPoint(MbrCorner corner);

	double               minX;
	double               minY;
	double               minZ;
	double               maxX;
	double               maxY;
	double               maxZ;

private:

};