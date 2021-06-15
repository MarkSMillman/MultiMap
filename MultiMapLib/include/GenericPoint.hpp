#pragma once

#include "MultiMap.h"

template <class T>
class GenericPoint {
public:
	GenericPoint(void);
	GenericPoint(std::string sPoint); // WKT format (x y) or (x y z)
	GenericPoint(T x, T y, T z = 0);
	GenericPoint(const GenericPoint& other);

	void Set(T x, T y, T z=0.0);
	std::string ToString(void);
	bool NonZero(void);

	GenericPoint& operator=(const GenericPoint& other);
	GenericPoint operator+(const GenericPoint& other);
	GenericPoint operator-(const GenericPoint& other);
	GenericPoint operator/(const size_t n);
	GenericPoint operator*(const size_t n);
	GenericPoint& operator*=(const GenericPoint& other);
	GenericPoint& operator+=(const GenericPoint& other);
	GenericPoint& operator-=(const GenericPoint& other);
	bool operator==(const GenericPoint& other);
	bool operator!=(const GenericPoint& other);

	T Distance2d(GenericPoint<T> & point);
	T Distance3d(GenericPoint<T> & point);

	static void Decompose(std::vector<GenericPoint<T>> & XYZ, std::vector<T> & X, std::vector<T> & Y, std::vector<T> & Z);
	static void Decompose(std::vector<GenericPoint<T>> & XYZ, std::vector<T> & X, std::vector<T> & Y);

	T                             X;
	T                             Y;
	T                             Z;

private:
	//friend class LidarPoint;
};

template <class T> GenericPoint<T>::GenericPoint(void) {
	X = 0;
	Y = 0;
	Z = 0;
}
template <class T> GenericPoint<T>::GenericPoint(std::string sPoint) {
	size_t start = sPoint.find_first_not_of("(");
	size_t end = sPoint.find_last_not_of(")");
	size_t space1 = sPoint.find_first_of(" ",start+1);
	if ( space1 != std::string::npos ) {
		std::string strVal;
		size_t space2 = sPoint.find_first_of(",",space1+1);			
		if ( space2 != std::string::npos ) {
			strVal = sPoint.substr(start,space1-start);
			X = std::stod(strVal);
			strVal = sPoint.substr(space1+1,space2-space1-1);
			Y = std::stod(strVal);
			strVal = sPoint.substr(space2+1,end-space2);
			Z = std::stod(strVal);
		} else {
			strVal = sPoint.substr(start,space1-start);
			X = std::stod(strVal);
			strVal = sPoint.substr(space1+1,space2-space1-1);
			Y = std::stod(strVal);
			Z = 0.0;
		}		
	} 
}
template <class T> GenericPoint<T>::GenericPoint(T x, T y, T z) {
	X = x;
	Y = y;
	Z = z;
}

template <class T> void GenericPoint<T>::Set(T x, T y, T z) {
	X = x;
	Y = y;
	Z = z;
}
template <class T> std::string GenericPoint<T>::ToString(void) {
	char buffer[128];
	sprintf(buffer,"(%.4f %.4f %.4f)",static_cast<double>(X),static_cast<double>(Y),static_cast<double>(Z));
	return std::string(buffer);
}
template <class T> bool GenericPoint<T>::NonZero(void) {
	bool nonZero = false;
	if ( static_cast<double>(X) != 0.0 ) {
		nonZero = true;
	} else if ( static_cast<double>(Y) != 0.0 ) {
		nonZero = true;
	} else if ( static_cast<double>(Z) != 0.0 ) {
		nonZero = true;
	}
	return nonZero;
}
template <class T> GenericPoint<T>::GenericPoint(const GenericPoint<T>& other) {
	*this = other;
}

template <class T> GenericPoint<T>& GenericPoint<T>::operator=(const GenericPoint<T>& other) {

	X = other.X;
	Y = other.Y;
	Z = other.Z;

	return *this;
}

template <class T> GenericPoint<T> GenericPoint<T>::operator+(const GenericPoint<T>& other) {
	GenericPoint dp = *this;
	dp.X = X + other.X;
	dp.Y = Y + other.Y;
	dp.Z = Z + other.Z;
	return dp;
}

template <class T> GenericPoint<T> GenericPoint<T>::operator-(const GenericPoint<T>& other) {
	GenericPoint<T> dp = *this;
	dp.X = X - other.X;
	dp.Y = Y - other.Y;
	dp.Z = Z - other.Z;
	return dp;
}
template <class T> GenericPoint<T> GenericPoint<T>::operator/(const size_t n) {
	GenericPoint<T> dp = *this;
	dp.X = X / n;
	dp.Y = Y / n;
	dp.Z = Z / n;
	return dp;
}
template <class T> GenericPoint<T> GenericPoint<T>::operator*(const size_t n) {
	GenericPoint<T> dp = *this;
	dp.X = X * n;
	dp.Y = Y * n;
	dp.Z = Z * n;
	return dp;
}
template <class T> GenericPoint<T>& GenericPoint<T>::operator+=(const GenericPoint<T>& other) {
	X += other.X;
	Y += other.Y;
	Z += other.Z;
	return *this;
}

template <class T> GenericPoint<T>& GenericPoint<T>::operator-=(const GenericPoint<T>& other) {
	X -= other.X;
	Y -= other.Y;
	Z -= other.Z;
	return *this;
}

template <class T> bool GenericPoint<T>::operator==(const GenericPoint<T>& other) {
	bool equal = false;
	if ( X == other.X ) {
		if ( Y == other.Y ) {
			if ( Z == other.Z ) {
				equal = true;
			}
		}
	}
	return equal;
}
template <class T> bool GenericPoint<T>::operator!=(const GenericPoint<T>& other) {
	bool equal = false;
	if ( X == other.X ) {
		if ( Y == other.Y ) {
			if ( Z == other.Z ) {
				equal = true;
			}
		}
	}
	return !equal;
}

template <class T> GenericPoint<T>& GenericPoint<T>::operator*=(const GenericPoint<T>& other) {
	this->X *= other.X;
	this->Y *= other.Y;
	this->Z *= other.Z;
	return *this;
}
template <class T> GenericPoint<T> operator*(const GenericPoint<T>& point1, const GenericPoint<T>& point2) {
	GenericPoint<T> outpoint;
	outpoint.X = point1.X * point2.X;
	outpoint.Y = point1.Y * point2.Y;
	outpoint.Z = point1.Z * point2.Z;
	return outpoint;
}

template <class T> T GenericPoint<T>::Distance2d(GenericPoint<T> & point) {
	T dX = this->X - point.X;
	T dY = this->Y - point.Y;
	return static_cast<T>(sqrt( dX*dX + dY*dY ));
}
template <class T> T GenericPoint<T>::Distance3d(GenericPoint<T> & point) {
	T dX = this->X - point.X;
	T dY = this->Y - point.Y;
	T dZ = this->Z - point.Z;
	return static_cast<T>(sqrt( dX*dX + dY*dY + dZ*dZ ));
}

template <class T> STATIC void GenericPoint<T>::Decompose(std::vector<GenericPoint<T>> & XYZ, std::vector<T> & X, std::vector<T> & Y, std::vector<T> & Z) {
	X.clear();
	Y.clear();
	Z.clear();
	X.resize(XYZ.size());
	Y.resize(XYZ.size());
	Z.resize(XYZ.size());
	for ( size_t p=0; p<XYZ.size(); p++ ) {
		X[p] = XYZ[p].X;
		Y[p] = XYZ[p].Y;
		Z[p] = XYZ[p].Z;
	}
}

template <class T> STATIC void GenericPoint<T>::Decompose(std::vector<GenericPoint<T>> & XYZ, std::vector<T> & X, std::vector<T> & Y) { 
	X.clear();
	Y.clear();
	X.resize(XYZ.size());
	Y.resize(XYZ.size());
	for ( size_t p=0; p<XYZ.size(); p++ ) {
		X[p] = XYZ[p].X;
		Y[p] = XYZ[p].Y;
	}
}