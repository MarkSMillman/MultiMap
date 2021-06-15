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