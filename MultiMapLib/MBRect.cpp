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
#include <string>
#include <vector>
#include "MBRect.h"

#undef min
#undef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

MULTIMAP_API MBRect::~MBRect(void){
}
/**
* @see OGC_Operations.png
**/
MULTIMAP_API MBRect::MBRect(void) {
	Reset();
}
MULTIMAP_API MBRect::MBRect(std::string sMBR) { //// WKT format (minX minY,maxX maxY) or (minX minY minZ,maxX maxY maxZ)
	Reset();
	size_t start = sMBR.find_first_not_of("(");
	size_t end = sMBR.find_last_not_of(")");
	size_t comma1 = sMBR.find_first_of(",",start+1);
	if ( comma1 != std::string::npos ) {
		std::string minXYZ = sMBR.substr(start,comma1-start);
		std::string maxXYZ = sMBR.substr(comma1+1,end);
		DblPoint minPoint(minXYZ);
		DblPoint maxPoint(maxXYZ);
		minX   =  minPoint.X;
		minY   =  minPoint.Y;
		minZ   =  minPoint.Z;
		maxX   =  maxPoint.X;
		maxY   =  maxPoint.Y;
		maxZ   =  maxPoint.Z;
	}
}
MULTIMAP_API MBRect::MBRect(DblPoint minXY, DblPoint maxXY) {
	minX   =  minXY.X;
	minY   =  minXY.Y;
	minZ   =  minXY.Z;
	maxX   =  maxXY.X;
	maxY   =  maxXY.Y;
	maxZ   =  maxXY.Z;
}
MULTIMAP_API MBRect::MBRect(double _minX, double _minY, double _maxX, double _maxY) {
	Reset();
	minX = _minX;
	minY = _minY;
	maxX = _maxX;
	maxY = _maxY;
}
MULTIMAP_API MBRect::MBRect(double _minX, double _minY, double _minZ, double _maxX, double _maxY, double _maxZ) {
	minX = _minX;
	minY = _minY;
	minZ = _minZ;
	maxX = _maxX;
	maxY = _maxY;
	maxZ = _maxZ;
}

MULTIMAP_API MBRect::MBRect(float _minX, float _minY, float _maxX, float _maxY) {
	Reset();
	minX = _minX;
	minY = _minY;
	maxX = _maxX;
	maxY = _maxY;
}
MULTIMAP_API MBRect::MBRect(float _minX, float _minY, float _minZ, float _maxX, float _maxY, float _maxZ) {
	minX = _minX;
	minY = _minY;
	minZ = _minZ;
	maxX = _maxX;
	maxY = _maxY;
	maxZ = _maxZ;
}

MULTIMAP_API MBRect::MBRect(const MBRect& other) {
	*this = other;
}


MULTIMAP_API MBRect& MBRect::operator=(const MBRect& other) {
	minX = other.minX;
	minY = other.minY;
	minZ = other.minZ;
	maxX = other.maxX;
	maxY = other.maxY;
	maxZ = other.maxZ;

	return *this;
}

MULTIMAP_API bool MBRect::Valid(void) {
	MBRect universe;
	return this->Inside(&universe);
}

MULTIMAP_API bool MBRect::operator==(const MBRect& other) {
	bool Equal = false;
	if ( minX == other.minX ) {
		if ( maxX == other.maxX ) {
			if ( minY == other.minY ) {
				if ( maxY == other.maxY ) {
					Equal = true;
				}
			}
		}
	}
	return Equal;
}

MULTIMAP_API bool MBRect::operator!=(const MBRect& other) {
	bool Equal = false;
	if ( minX == other.minX ) {
		if ( maxX == other.maxX ) {
			if ( minY == other.minY ) {
				if ( maxY == other.maxY ) {
					Equal = true;
				}
			}
		}
	}
	return !Equal;
}
MULTIMAP_API bool MBRect::Compare(MBRect & other, double precision, double *maxDiff) {
	bool same = true;
	if ( maxDiff ) {
		*maxDiff = 0.0;
	}
	if ( abs(minX-other.minX) > precision ) {
		same = false;
		if ( maxDiff ) {
			*maxDiff = max(*maxDiff,abs(minX-other.minX));
		}
	}
	if ( abs(maxX-other.maxX) > precision ) {
		same = false;
		if ( maxDiff ) {
			*maxDiff = max(*maxDiff,abs(maxX-other.maxX));
		}
	}

	if ( abs(minY-other.minY) > precision ) {
		same = false;
		if ( maxDiff ) {
			*maxDiff = max(*maxDiff,abs(minY-other.minY));
		}
	}
	if ( abs(maxY-other.maxY) > precision ) {
		same = false;
		if ( maxDiff ) {
			*maxDiff = max(*maxDiff,abs(maxY-other.maxY));
		}
	}
	return same;
}
MULTIMAP_API void MBRect::Reset(void) {
	minX   =  DBL_MAX;
	minY   =  DBL_MAX;
	minZ   =  DBL_MAX;
	maxX   = -DBL_MAX;
	maxY   = -DBL_MAX;
	maxZ   = -DBL_MAX;
}
MULTIMAP_API double MBRect::XRange(void) {
	return maxX-minX;
}
MULTIMAP_API double MBRect::YRange(void) {
	return maxY-minY;
}
MULTIMAP_API double MBRect::ZRange(void) {
	return maxZ-minZ;
}
/**
* @param dim [2] to convert XY, anything else will convert XYZ
* @return a std::string formatted as "(minX,minY,maxX,maxY)" or "(minX,minY,maxZ,maxX,maxY,maxZ)"
* @see MBRect::Parse(std::string)
**/
MULTIMAP_API std::string MBRect::ToString(int dim) {
	char buffer[128];
	if ( dim==2 ) {
		sprintf(buffer,"(%.4f %.4f,%.4f %.4f)",minX,minY,maxX,maxY);
	} else {
		sprintf(buffer,"(%.4f %.4f %.4f,%.4f %.4f %.4f)",minX,minY,minZ,maxX,maxY,maxZ);
	}
	return std::string(buffer);
}

MULTIMAP_API void MBRect::ResetIfInvalid(void) {
	if ( minX >= maxX || minY >= maxY ) {
		Reset();
	}
}
MULTIMAP_API void MBRect::Align(unsigned long alignmentFactor) {
	double dFactor = static_cast<double>(alignmentFactor);
	minX = static_cast<double>(static_cast<long long>(minX/dFactor) * static_cast<long long>(alignmentFactor));
	minY = static_cast<double>(static_cast<long long>(minY/dFactor) * static_cast<long long>(alignmentFactor));

	double _maxX = maxX;
	maxX = static_cast<double>(static_cast<long long>(maxX/dFactor) * static_cast<long long>(alignmentFactor));
	if ( maxX < _maxX ) {
		maxX += dFactor;
	}
	double _maxY = maxY;
	maxY = static_cast<double>(static_cast<long long>(maxY/dFactor) * static_cast<long long>(alignmentFactor));
	if ( maxY < _maxY ) {
		maxY += dFactor;
	}
}
MULTIMAP_API void MBRect::Expand(double x, double y, double z) {
	if ( y == DBL_MAX ) {
		y = x;
	}
	minX -= x;
	minY -= y;
	minZ -= z;
	maxX += x;
	maxY += y;
	maxZ += z;
}
MULTIMAP_API void MBRect::Split(std::vector<MBRect> & parts, size_t xParts, size_t yParts, size_t roundTo) {
	parts.clear();
	parts.resize(0);
	double rangeX = maxX - minX;
	double rangeY = maxY - minY;
	double originX = minX;
	double originY = minY;
	double incrementX = rangeX / static_cast<double>(xParts);
	double incrementY = rangeY / static_cast<double>(yParts);
	if ( roundTo > 0 ) {
		size_t iMinX = static_cast<size_t>(minX);
		size_t iMinY = static_cast<size_t>(minY);
		if ( iMinX%roundTo != 0 ) {
			originX = static_cast<double>(iMinX + roundTo - iMinX%roundTo);
		}
		if ( iMinY%roundTo != 0 ) {
			originY = static_cast<double>(iMinY + roundTo - iMinY%roundTo);
		}
		size_t iIncrementX = static_cast<size_t>(rangeX) / xParts;
		size_t iIncrementY = static_cast<size_t>(rangeY) / yParts;
		if ( iIncrementX%roundTo != 0 ) {
			incrementX = static_cast<double>(iIncrementX - iIncrementX%roundTo);
		}
		if ( iIncrementY%roundTo != 0 ) {
			incrementY = static_cast<double>(iIncrementY - iIncrementY%roundTo);
		}
	}

	double offsetY = originY;
	for ( size_t y=0; y<yParts; y++ ) {
		double offsetX = originX;
		for ( size_t x=0; x<xParts; x++ ) {
			MBRect part(offsetX,offsetY,offsetX+incrementX,offsetY+incrementY);
			parts.push_back(part);
			offsetX += incrementX;
		}
		offsetY += incrementY;
	}
}
MULTIMAP_API double MBRect::Area(void) {
	return abs((maxX-minX) * (maxY-minY));
}

MULTIMAP_API double MBRect::Distance2d(DblPoint point) {
	double distance = DBL_MAX;

	if ( Covers(point) ) {
		distance = 0;
	} else {
		DblPoint ll(minX,minY);
		DblPoint lr(maxX,minY);
		DblPoint ul(minX,maxY);
		DblPoint ur(maxX,maxY);

		distance = min(distance,point.Distance2d(ll));
		distance = min(distance,point.Distance2d(lr));
		distance = min(distance,point.Distance2d(ur));
		distance = min(distance,point.Distance2d(ul));
	}
	return distance;
}

MULTIMAP_API DblPoint MBRect::Center(void) {
	DblPoint point;
	point.X = minX + ((maxX-minX)/2.0);
	point.Y = minY + ((maxY-minY)/2.0);
	point.Z = minZ + ((maxZ-minZ)/2.0);
	return point;
}

MULTIMAP_API bool MBRect::Equal(MBRect & other) { 
	return Equal(&other);
}

MULTIMAP_API bool MBRect::Equal(MBRect *other) { 
	bool Equal = false;
	if ( minX == other->minX ) {
		if ( maxX == other->maxX ) {
			if ( minY == other->minY ) {
				if ( maxY == other->maxY ) {
					Equal = true;
				}
			}
		}
	}
	return Equal;
}
MULTIMAP_API RPointOrientation MBRect::Adjacent(MBRect & other) { 
	return Adjacent(&other);
}
MULTIMAP_API RPointOrientation MBRect::Adjacent(MBRect *other)  { 
	RPointOrientation adjacent = RPO_NONE;
	if ( Covers(other) ) {
		adjacent = RPO_COVERED;
	} else if ( minX == other->maxX && minY == other->minY && maxY == other->maxY ) {
		adjacent = RPO_WEST;
	} else if ( maxX == other->minX && minY == other->minY && maxY == other->maxY ) {
		adjacent = RPO_EAST;
	} else if ( minY == other->maxY && minX == other->minX && maxX == other->maxX ) {
		adjacent = RPO_SOUTH;
	} else if ( maxY == other->minY && minX == other->minX && maxX == other->maxX ) {
		adjacent = RPO_NORTH;
	} else if ( minX == other->maxX && minY == other->maxY ) {
		adjacent = RPO_SOUTHWEST;
	} else if ( maxX == other->minX && minY == other->maxY ) {
		adjacent = RPO_SOUTHEAST;
	} else if ( maxY == other->minY && minX == other->maxX ) {
		adjacent = RPO_NORTHWEST;
	} else if ( maxY == other->minY && maxX == other->minX ) {
		adjacent = RPO_NORTHEAST;
	}

	return adjacent;
}

MULTIMAP_API RPointOrientation MBRect::Adjacent(DblPoint & point) {
	RPointOrientation adjacent = RPO_NONE;
	if ( Covers(point) ) {
		adjacent = RPO_COVERED;
	} else if ( minX > point.X && minY <= point.Y && maxY >= point.Y ) {
		adjacent = RPO_WEST;
	} else if ( maxX < point.X && minY <= point.Y && maxY >= point.Y ) {
		adjacent = RPO_EAST;
	} else if ( minY > point.Y && minX <= point.X && maxX >= point.X ) {
		adjacent = RPO_SOUTH;
	} else if ( maxY < point.Y && minX <= point.X && maxX >= point.X ) {
		adjacent = RPO_NORTH;
	} else if ( minX > point.X && minY > point.Y ) {
		adjacent = RPO_SOUTHWEST;
	} else if ( maxX < point.X && maxY > point.Y ) {
		adjacent = RPO_SOUTHEAST;
	} else if ( minX > point.X && maxY < point.Y ) {
		adjacent = RPO_NORTHWEST;
	} else if ( maxX < point.X && maxY < point.Y ) {
		adjacent = RPO_NORTHEAST;
	}

	return adjacent;
}

MULTIMAP_API bool MBRect::AnyInteract(MBRect & other) { 
	return AnyInteract( &other );
}
MULTIMAP_API bool MBRect::AnyInteract(MBRect *other) {
	bool anyinteract = false;
	DblPoint d(other->minX,other->minY);
	if ( AnyInteract(&d) ) {
		anyinteract = true;
	} else {
		d.Set(other->maxX,other->minY);
		if ( AnyInteract(&d) ) {
			anyinteract = true;
		} else {
			d.Set(other->minX,other->maxY);
			if ( AnyInteract(&d) ) {
				anyinteract = true;
			} else {
				d.Set(other->maxX,other->maxY);
				if ( AnyInteract(&d) ) {
					anyinteract = true;
				} else {
					d.Set(minX,minY);
					if ( other->AnyInteract(&d) ) {
						anyinteract = true;
					} else {
						d.Set(maxX,minY);
						if ( other->AnyInteract(&d) ) {
							anyinteract = true;
						} else {
							d.Set(minX,maxY);
							if ( other->AnyInteract(&d) ) {
								anyinteract = true;
							} else {
								d.Set(maxX,maxY);
								if ( other->AnyInteract(&d) ) {
									anyinteract = true;
								}
							}
						}
					}
				}
			}
		}
	}
	if ( !anyinteract ) {
		if ( minX <= other->minX && maxX >= other->maxX && other->minY <= minY && other->maxY >= maxY ) {
			anyinteract = true;	
		} else 	if ( other->minX <= minX && other->maxX >= maxX && minY <= other->minY && maxY >= other->maxY ) {
			anyinteract = true;	
		}
	}
	return anyinteract;
}

MULTIMAP_API bool MBRect::AnyInteract(DblPoint & point){
	return AnyInteract( &point );
}
MULTIMAP_API bool MBRect::AnyInteract(DblPoint *point){
	bool anyinteract = false;
	if ( point->X >= minX ) {
		if ( point->Y >= minY ) {
			if ( point->X <= maxX ) {
				if ( point->Y <= maxY ) {
					anyinteract = true;
				}
			}
		}
	}
	return anyinteract;
}

MULTIMAP_API bool MBRect::Covers(MBRect & other) { 
	return Covers( &other );
}
MULTIMAP_API bool MBRect::Covers(MBRect *other) { 
	bool covers = false;
	DblPoint ll(other->minX,other->minY);
	DblPoint lr(other->maxX,other->minY);
	DblPoint ul(other->minX,other->maxY);
	DblPoint ur(other->maxX,other->maxY);
	if (AnyInteract(ll) && AnyInteract(lr) && AnyInteract(ul) && AnyInteract(ur) ) {
		int insides = 0;
		if ( Contains(ll) )
			insides++;
		if ( Contains(lr) )
			insides++;
		if ( Contains(ul) )
			insides++;
		if ( Contains(ur) )
			insides++;
		if ( insides > 0 )
			covers = true;
		else if ( Equal(other) ) 
			covers = true;
	}
	return covers;
}

MULTIMAP_API bool MBRect::Covers(DblPoint & point) { 
	return Covers( &point );
}
MULTIMAP_API bool MBRect::Covers(DblPoint *point) { 
	bool covers = false;
	if ( point->X >= minX ) {
		if ( point->Y >= minY ) {
			if ( point->X <= maxX ) {
				if ( point->Y <= maxY ) {
					covers = true;
				}
			}
		}
	}
	return covers;
}
MULTIMAP_API bool MBRect::CoveredBy(MBRect & other) { 
	return CoveredBy( &other );
}
MULTIMAP_API bool MBRect::CoveredBy(MBRect *other) { 
	bool covers = false;
	DblPoint ll(minX,minY);
	DblPoint lr(maxX,minY);
	DblPoint ul(minX,maxY);
	DblPoint ur(maxX,maxY);
	if (other->AnyInteract(ll) && other->AnyInteract(lr) && other->AnyInteract(ul) && other->AnyInteract(ur) ) {
		covers = true;
	}
	return covers;
}

MULTIMAP_API bool MBRect::Contains(DblPoint & point) {
	return Contains(&point);
}

MULTIMAP_API bool MBRect::Contains(DblPoint *point) {
	bool contained = false;
	if ( point->X > minX ) {
		if ( point->Y > minY ) {
			if ( point->X < maxX ) {
				if ( point->Y < maxY ) {
					contained = true;
				}
			}
		}
	}
	return contained;
}
MULTIMAP_API bool MBRect::Contains(MBRect & other) {
	return Contains(&other);
}

MULTIMAP_API bool MBRect::Contains(MBRect *other) {
	bool contained = false;
	DblPoint ll(other->minX,other->minY);
	DblPoint lr(other->maxX,other->minY);
	DblPoint ul(other->minX,other->maxY);
	DblPoint ur(other->maxX,other->maxY);
	if (Contains(ll) && Contains(lr) && Contains(ul) && Contains(ur) ) {
		contained = true;
	}
	return contained;
}
MULTIMAP_API bool MBRect::Inside(MBRect & other) {
	return Inside(&other);
}

MULTIMAP_API bool MBRect::Inside(MBRect *other) {
	bool inside = false;
	DblPoint ll(minX,minY);
	DblPoint lr(maxX,minY);
	DblPoint ul(minX,maxY);
	DblPoint ur(maxX,maxY);
	if (other->Contains(ll) && other->Contains(lr) && other->Contains(ul) && other->Contains(ur) ) {
		inside = true;
	}
	return inside;
}

MULTIMAP_API bool MBRect::Overlaps(MBRect & other) {
	return Overlaps(&other);
}

MULTIMAP_API bool MBRect::Overlaps(MBRect *other) {
	bool overlaps = false;
	if ( AnyInteract(other) ) {
		if ( Equal(other) ) {
			overlaps = true;
		} else if ( Covers(other) ) {
			overlaps = true;
		} else if ( other->Covers(this) ) {
			overlaps = true;
		}else if ( minX <= other->minX && maxX >= other->maxX && other->minY <= minY && other->maxY >= maxY ) {
			overlaps = true;	
		} else if ( other->minX <= minX && other->maxX >= maxX && minY <= other->minY && maxY >= other->maxY ) {
			overlaps = true;	
		} else {
			DblPoint ll(other->minX,other->minY);
			DblPoint lr(other->maxX,other->minY);
			DblPoint ul(other->minX,other->maxY);
			DblPoint ur(other->maxX,other->maxY);
			if ( Contains(ll) || Contains(lr) || Contains(ul) || Contains(ur) ) {
				overlaps = true;
			} else {
				DblPoint ll(minX,minY);
				DblPoint lr(maxX,minY);
				DblPoint ul(minX,maxY);
				DblPoint ur(maxX,maxY);
				if ( other->Contains(ll) || other->Contains(lr) || other->Contains(ul) || other->Contains(ur) ) {
					overlaps = true;
				}
			}
		}
	}
	return overlaps;
}

//////////////////////// end of OGC

MULTIMAP_API bool MBRect::Indexes(DblPoint & point) { 
	return Indexes( &point );
}
MULTIMAP_API bool MBRect::Indexes(DblPoint *point) { 
	bool indexes = false;
	if ( point->X >= minX ) {
		if ( point->Y >= minY ) {
			if ( point->X < maxX ) {
				if ( point->Y < maxY ) {
					indexes = true;
				}
			}
		}
	}
	return indexes;
}
MULTIMAP_API void MBRect::Update(MBRect & other) {
	Update(&other);
}

MULTIMAP_API void MBRect::Update(MBRect *other) {
	minX = min(minX,other->minX);
	minY = min(minY,other->minY);
	minZ = min(minZ,other->minZ);
	maxX = max(maxX,other->maxX);
	maxY = max(maxY,other->maxY);
	maxZ = max(maxZ,other->maxZ);
}

MULTIMAP_API void MBRect::Update(DblPoint & point) { 
	Update( &point );
}

MULTIMAP_API void MBRect::Update(DblPoint *point) { 
	minX = min(minX,point->X);
	minY = min(minY,point->Y);
	minZ = min(minZ,point->Z);
	maxX = max(maxX,point->X);
	maxY = max(maxY,point->Y);
	maxZ = max(maxZ,point->Z);
}
MULTIMAP_API void MBRect::Update(double x, double y, double z) {
	minX = min(minX,x);
	minY = min(minY,y);
	minZ = min(minZ,z);
	maxX = max(maxX,x);
	maxY = max(maxY,y);
	maxZ = max(maxZ,z);
}
MULTIMAP_API void MBRect::Update(std::vector<DblPoint> & points) {
	Reset();
	for ( size_t p=0; p<points.size(); p++ ) {
		minX = min(minX,points[p].X);
		maxX = max(maxX,points[p].X);
		minY = min(minY,points[p].Y);
		maxY = max(maxY,points[p].Y);
		minZ = min(minZ,points[p].Z);
		maxZ = max(maxZ,points[p].Z);
	}
}
MULTIMAP_API void MBRect::UpdateXY(std::vector<DblPoint> & points) {
	minX   =  DBL_MAX;
	minY   =  DBL_MAX;
	maxX   = -DBL_MAX;
	maxY   = -DBL_MAX;
	for ( size_t p=0; p<points.size(); p++ ) {
		minX = min(minX,points[p].X);
		maxX = max(maxX,points[p].X);
		minY = min(minY,points[p].Y);
		maxY = max(maxY,points[p].Y);
	}
}
MULTIMAP_API void MBRect::UpdateZ(std::vector<DblPoint> & points) {
	minZ   =  DBL_MAX;
	maxZ   = -DBL_MAX;
	for ( size_t p=0; p<points.size(); p++ ) {
		minZ = min(minZ,points[p].Z);
		maxZ = max(maxZ,points[p].Z);
	}
}
MULTIMAP_API MBRect MBRect::Round(void) {
	MBRect rounded = *this;
	rounded.minX = static_cast<double>(static_cast<long long>(minX));
	rounded.minY = static_cast<double>(static_cast<long long>(minY));
	rounded.minZ = static_cast<double>(static_cast<long long>(minZ));
	rounded.maxX = static_cast<double>(static_cast<long long>(maxX+1.0-(DBL_EPSILON*1000.0)));
	rounded.maxY = static_cast<double>(static_cast<long long>(maxY+1.0-(DBL_EPSILON*1000.0)));
	rounded.maxZ = static_cast<double>(static_cast<long long>(maxZ+1.0-(DBL_EPSILON*1000.0)));
	return rounded;
}
MULTIMAP_API MBRect MBRect::RoundXY(void) {
	MBRect rounded = *this;
	rounded.minX = static_cast<double>(static_cast<long long>(minX));
	rounded.minY = static_cast<double>(static_cast<long long>(minY));
	rounded.maxX = static_cast<double>(static_cast<long long>(maxX+1.0-(DBL_EPSILON*1000.0)));
	rounded.maxY = static_cast<double>(static_cast<long long>(maxY+1.0-(DBL_EPSILON*1000.0)));
	return rounded;
}
MULTIMAP_API bool MBRect::Intersect(MBRect & intersection, MBRect & other) { 
	return Intersect ( intersection, &other );
}

MULTIMAP_API bool MBRect::Intersect(MBRect & intersection, MBRect *other) {
	bool intersected = false;
	if ( AnyInteract(other) ) {
		intersection.minX = max(minX,other->minX);
		intersection.minY = max(minY,other->minY);
		intersection.minZ = max(minZ,other->minZ);
		intersection.maxX = min(maxX,other->maxX);
		intersection.maxY = min(maxY,other->maxY);
		intersection.maxZ = min(maxZ,other->maxZ);
		intersected = true;
	}
	return intersected;
}

MULTIMAP_API MBRect MBRect::Union(MBRect & other) { 
	return Union ( &other );
}

MULTIMAP_API MBRect MBRect::Union(MBRect *other) {
	MBRect _union;
	_union.minX = min(minX,other->minX);
	_union.minY = min(minY,other->minY);
	_union.minZ = min(minZ,other->minZ);
	_union.maxX = max(maxX,other->maxX);
	_union.maxY = max(maxY,other->maxY);
	_union.maxZ = max(maxZ,other->maxZ);
	return _union;
}

MULTIMAP_API DblPoint MBRect::CornerPoint(MbrCorner corner) {
	DblPoint cornerPoint;
	switch (corner) {
	case MBRLL:
		cornerPoint.Set(minX,minY);
		break;
	case MBRLR:
		cornerPoint.Set(maxX,minY);
		break;
	case MBRUL:
		cornerPoint.Set(minX,maxY);
		break;
	case MBRUR:
		cornerPoint.Set(maxX,maxY);
		break;
	}
	return cornerPoint;
}