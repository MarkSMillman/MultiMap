#pragma once
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
#include <vector>
#include "GenericPoint.hpp"
#define DblPoint GenericPoint<double>
#define FloatPoint  GenericPoint<float>
class DouglasPeucker {
public:
	static void Reduce(std::vector<DblPoint> & returnPoints, std::vector<DblPoint> & points, double tolerance);
	static void Reduce(std::vector<DblPoint> & returnPoints, std::vector<double> & easting, std::vector<double> & northing, double tolerance);

	static void Reduce(std::vector<FloatPoint> & returnPoints, std::vector<FloatPoint> & points, float tolerance);
	static void Reduce(std::vector<FloatPoint> & returnPoints, std::vector<double> & easting, std::vector<double> & northing, float tolerance);

protected:
	~DouglasPeucker(void);
	DouglasPeucker(void);

private:
    static void Reduce(std::vector<DblPoint> & points, size_t firstPoint, size_t lastPoint, double tolerance, std::vector<size_t> & pointIndexsToKeep);
	static double PerpendicularDistance(DblPoint point1, DblPoint point2, DblPoint point);

    static void Reduce(std::vector<FloatPoint> & points, size_t firstPoint, size_t lastPoint, float tolerance, std::vector<size_t> & pointIndexsToKeep);
	static float PerpendicularDistance(FloatPoint point1, FloatPoint point2, FloatPoint point);
};