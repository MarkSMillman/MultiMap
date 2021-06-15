#pragma once
#include "GenericPoint.hpp"

template <class T>
class GenericVector {
public:
	GenericVector(void);
	GenericVector(GenericPoint<T> begin, GenericPoint<T> end);
	GenericVector(const GenericVector& other);

	void Set(GenericPoint<T> begin, GenericPoint<T> end);
	GenericVector& operator=(const GenericVector& other);
	GenericVector operator+(const GenericVector& other);
	GenericVector operator-(const GenericVector& other);
	GenericVector& operator+=(const GenericVector& other);
	GenericVector& operator-=(const GenericVector& other);
	bool operator==(const GenericVector& other);
	bool operator!=(const GenericVector& other);


	GenericVector Normal(void);
	T DotProduct2d(void);
	T DotProduct3d(void);
	T Length2d(void);
	T Length3d(void);

	GenericPoint<T>  begin;
	GenericPoint<T>    end;

};
template <class T> GenericVector<T>::GenericVector(void) {
}

template <class T> GenericVector<T>::GenericVector(GenericPoint<T> _begin, GenericPoint<T> _end) {
	begin = _begin;
	end = _end;
}

template <class T> void GenericVector<T>::Set(GenericPoint<T> _begin, GenericPoint<T> _end) {
	begin = _begin;
	end = _end;
}

template <class T> GenericVector<T>::GenericVector(const GenericVector<T>& other) {
	*this = other;
}

template <class T> GenericVector<T>& GenericVector<T>::operator=(const GenericVector<T>& other) {
	begin = other.begin;
	end = other.end;
	return *this;
}

template <class T> GenericVector<T> GenericVector<T>::operator+(const GenericVector<T>& other) {
	GenericVector sv = *this;
	sv.begin = begin + other.begin;
	sv.end   = end   + other.end;
	return sv;
}

template <class T> GenericVector<T> GenericVector<T>::operator-(const GenericVector<T>& other) {
	GenericVector sv = *this;
	sv.begin = begin - other.begin;
	sv.end   = end   - other.end;
	return sv;
}
template <class T> GenericVector<T>& GenericVector<T>::operator+=(const GenericVector<T>& other) {
	begin += other.begin;
	end   += other.end;
	return *this;
}

template <class T> GenericVector<T>& GenericVector<T>::operator-=(const GenericVector<T>& other) {
	begin -= other.begin;
	end   -= other.end;
	return *this;
}

template <class T> bool GenericVector<T>::operator==(const GenericVector<T>& other) {
	bool equal = false;
	if ( begin == other.begin ) {
		if ( end == other.end ) {
			equal = true;
		}
	}
	return equal;
}
template <class T> bool GenericVector<T>::operator!=(const GenericVector<T>& other) {
	bool equal = false;
	if ( begin == other.begin ) {
		if ( end == other.end ) {
			equal = true;
		}
	}
	return !equal;
}

template <class T> GenericVector<T> GenericVector<T>::Normal() {
	return begin - end;
}
template <class T> T GenericVector<T>::DotProduct2d() {
	T dot = begin.X * end.X + begin.Y * end.Y;
	return dot;
}
template <class T> T GenericVector<T>::DotProduct3d() {
	T dot = begin.X * end.X + begin.Y * end.Y + begin.Z * end.Z;
	return dot;
}
template <class T> T GenericVector<T>::Length2d() {
	return begin.Distance2d(end);
}
template <class T> T GenericVector<T>::Length3d() {
	return begin.Distance3d(end);
}