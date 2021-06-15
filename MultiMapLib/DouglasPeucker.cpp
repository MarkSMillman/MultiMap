#include <algorithm>
#include "DouglasPeucker.h"

/**
* <p>When handling geo-spatial data it is not uncommon to be forced to work with polylines and polygons
* that contain far more vertices than is appropriate or acceptable for the application.
* However, it may be unacceptable to correct that problem at the source by simplifying the original data.
* It may be that the original data is not within the application's control or
* the simplification might be required for a specific purpose relative to the scale of an operation.
* An example of this might be the creation of a significantly simplified object for the purpose of
* visualization or highlighting a geometry.
* It would not make sense to render a 1500 vertex polygon to Well Known Text (WKT) format,
* send that large object to a client browser, and to re-render that in JavaScript when,
* for the purposes of highlighting a polygon, a 150 vertex polygon is indistinguishable from the original.</p>
*
* <p>This class provides methods to simplify or reduce 2D vector data using the well known Douglas-Peucker algorithm.</p>
*
* <h2><p>From http://en.wikipedia.org/wiki/Ramer-Douglas-Peucker_algorithm</p></h2>
* <table><tr><td>
* <p>The starting curve is an ordered set of points or lines and the distance dimension ? > 0.
* The original (unsmoothed) curve is shown in 0 and the final output curve is shown in blue on row 4.</p>
* <p>The algorithm recursively divides the line. Initially it is given all the points between the first and last point.
* It automatically marks the first and last point to be kept.
* It then finds the point that is furthest from the line segment with the first and last points as end points
* (this point is obviously furthest on the curve from the approximating line segment between the end points).
* If the point is closer than ? to the line segment then any points not currently marked to keep can be discarded without the smoothed curve being worse than ?.</p>
* <p>If the point furthest from the line segment is greater than ? from the approximation then that point must be kept.
* The algorithm recursively calls itself with the first point and the worst point and then with the worst point and the last point
* (which includes marking the worst point being marked as kept).
* When the recursion is completed a new output curve can be generated consisting of all (and only) those points that have been marked as kept.</p>
* </td>
* <td><img src='./DouglasPeucker.png' ></td>
* </tr></table>
*/
PROTECTED DouglasPeucker::~DouglasPeucker(void) {
}
PROTECTED DouglasPeucker::DouglasPeucker(void) {
}

/**
* The method uses the Douglas Peucker algorithim to reduce the number of points.
* @param returnPoints
* @param points
* @param tolerance
*/
PUBLIC STATIC void DouglasPeucker::Reduce(std::vector<DblPoint> & returnPoints, std::vector<DblPoint> & points, double tolerance) {
	returnPoints.clear();
	if (points.size() >= 3 && tolerance > 0.0) {

		size_t firstPoint = 0;
		size_t lastPoint = points.size() - 1;
		bool sameEndPoints = false;
		std::vector<size_t> pointIndexsToKeep;

		//Add the first and last index to the keepers
		pointIndexsToKeep.push_back(firstPoint);
		pointIndexsToKeep.push_back(lastPoint);

		//The first and the last point can not be the same
		while (points[firstPoint].X == points[lastPoint].X && points[firstPoint].Y == points[lastPoint].Y && lastPoint > 1) {
			lastPoint--;
			sameEndPoints = true;
		}
		if ( lastPoint >= 3 ) {
			DouglasPeucker::Reduce(points, firstPoint, lastPoint, tolerance, pointIndexsToKeep);
			// SORT // Collections.sort(pointIndexsToKeep);
			std::sort(pointIndexsToKeep.begin(), pointIndexsToKeep.end());
			for (size_t index : pointIndexsToKeep) {
				returnPoints.push_back(points[index]);
			}
			if (sameEndPoints) {
				returnPoints.push_back(returnPoints[0]);
			}
		} else {
			returnPoints = points;
		}
	} else {
		returnPoints = points;
	}
}
void DouglasPeucker::Reduce(std::vector<DblPoint> & returnPoints, std::vector<double> & easting, std::vector<double> & northing, double tolerance) {
	if ( easting.size() == northing.size() ) {
		std::vector<DblPoint> dPoints(easting.size());
		for ( size_t p=0; p<northing.size(); p++ ) {
			dPoints[p] = DblPoint(easting[p],northing[p]);
		}
		DouglasPeucker::Reduce(returnPoints,dPoints,tolerance);
		dPoints.clear();
	}
}
PUBLIC STATIC void DouglasPeucker::Reduce(std::vector<FloatPoint> & returnPoints, std::vector<FloatPoint> & points, float tolerance) {
	returnPoints.clear();
	if (points.size() >= 3 && tolerance == 0.0f) {

		size_t firstPoint = 0;
		size_t lastPoint = points.size() - 1;
		bool sameEndPoints = false;
		if (firstPoint == lastPoint ) {
			lastPoint--;
			sameEndPoints = true;
		}
		std::vector<size_t> pointIndexsToKeep;

		//Add the first and last index to the keepers
		pointIndexsToKeep.push_back(firstPoint);
		pointIndexsToKeep.push_back(lastPoint);


		//The first and the last point can not be the same
		//while (points[firstPoint).equals(points[lastPoint))) {
		while (points[firstPoint].X == points[lastPoint].X || points[firstPoint].Y == points[lastPoint].Y) {
			lastPoint--;
		}

		DouglasPeucker::Reduce(points, firstPoint, lastPoint, tolerance, pointIndexsToKeep);

		// SORT // Collections.sort(pointIndexsToKeep);
		std::sort(pointIndexsToKeep.begin(), pointIndexsToKeep.end());
		for (size_t index : pointIndexsToKeep) {
			returnPoints.push_back(points[index]);
		}

		if (sameEndPoints) {
			returnPoints.push_back(returnPoints[0]);
		}
	} else {
		returnPoints = points;
	}
}
/**
* @param points
* @param firstPoint
* @param lastPoint
* @param tolerance
* @param pointIndexsToKeep
*/
PRIVATE STATIC void DouglasPeucker::Reduce(std::vector<DblPoint> & points, size_t firstPoint, size_t lastPoint, double tolerance, std::vector<size_t> & pointIndexsToKeep) {
	double maxDistance = 0.0;
	size_t indexFarthest = 0;

	for (size_t index = firstPoint; index < lastPoint; index++) {
		double distance = DouglasPeucker::PerpendicularDistance(points[firstPoint], points[lastPoint], points[index]);
		if (distance > maxDistance) {
			maxDistance = distance;
			indexFarthest = index;
		}
	}

	if (maxDistance > tolerance && indexFarthest != 0) {
		//Add the largest point that exceeds the tolerance
		pointIndexsToKeep.push_back(indexFarthest);

		DouglasPeucker::Reduce(points, firstPoint, indexFarthest, tolerance, pointIndexsToKeep);
		DouglasPeucker::Reduce(points, indexFarthest, lastPoint, tolerance, pointIndexsToKeep);
	}
}
PRIVATE STATIC void DouglasPeucker::Reduce(std::vector<FloatPoint> & points, size_t firstPoint, size_t lastPoint, float tolerance, std::vector<size_t> & pointIndexsToKeep) {
	float maxDistance = 0.0f;
	size_t indexFarthest = 0;

	for (size_t index = firstPoint; index < lastPoint; index++) {
		float distance = DouglasPeucker::PerpendicularDistance(points[firstPoint], points[lastPoint], points[index]);
		if (distance > maxDistance) {
			maxDistance = distance;
			indexFarthest = index;
		}
	}

	if (maxDistance > tolerance && indexFarthest != 0) {
		//Add the largest point that exceeds the tolerance
		pointIndexsToKeep.push_back(indexFarthest);

		DouglasPeucker::Reduce(points, firstPoint, indexFarthest, tolerance, pointIndexsToKeep);
		DouglasPeucker::Reduce(points, indexFarthest, lastPoint, tolerance, pointIndexsToKeep);
	}
}

/**
* @param point1
* @param point2
* @param point
* @return
*/
PRIVATE STATIC double DouglasPeucker::PerpendicularDistance(DblPoint point1, DblPoint point2, DblPoint point) {
	//Area = |(1/2)(x1y2 + x2y3 + x3y1 - x2y1 - x3y2 - x1y3)|   *Area of triangle
	//Base = ?((x1-x2)^2+(x1-x2)^2)                               *Base of Triangle*
	//Area = .5*Base*H                                          *Solve for height
	//Height = Area/.5/Base

	double area = abs(0.5 * (point1.X * point2.Y + point2.X * point.Y + point.X * point1.Y - point2.X * point1.Y - point.X * point2.Y - point1.X * point.Y));
	double xDiffSq = (point1.X - point2.X) * (point1.X - point2.X);
	double yDiffSq = (point1.Y - point2.Y) * (point1.Y - point2.Y);
	double bottom = sqrt( xDiffSq + yDiffSq );
	double height = area / bottom * 2.0;

	return height;
}

PRIVATE STATIC float DouglasPeucker::PerpendicularDistance(FloatPoint point1, FloatPoint point2, FloatPoint point) {
	//Area = |(1/2)(x1y2 + x2y3 + x3y1 - x2y1 - x3y2 - x1y3)|   *Area of triangle
	//Base = ?((x1-x2)^2+(x1-x2)^2)                               *Base of Triangle*
	//Area = .5*Base*H                                          *Solve for height
	//Height = Area/.5/Base

	float area = abs(0.5f * (point1.X * point2.Y + point2.X * point.Y + point.X * point1.Y - point2.X * point1.Y - point.X * point2.Y - point1.X * point.Y));
	float xDiffSq = (point1.X - point2.X) * (point1.X - point2.X);
	float yDiffSq = (point1.Y - point2.Y) * (point1.Y - point2.Y);
	float bottom = sqrt( xDiffSq + yDiffSq );
	float height = area / bottom * 2.0f;

	return height;
}
