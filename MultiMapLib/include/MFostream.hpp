#pragma once
#include "MultiMap.h"
#include "MFistream.hpp"

class MappedFile;
template<class _Traits> 
class MFostream : public MFistream<_Traits> {
public:
	~MFostream<_Traits>(void);
	MFostream<_Traits>(void);

	MFostream<_Traits>* flush();
	MFostream<_Traits>* seekp(std::streampos pos);
	MFostream<_Traits>* seekp(std::streamoff off, std::ios_base::seekdir way);
	std::streampos tellp();
	MFostream<_Traits>& write(const char* s, std::streamsize n);
};
template<class _Traits> 
MFostream<_Traits>::~MFostream(void) {
}
template<class _Traits> 
MFostream<_Traits>::MFostream(void) {
	mf = NULL;
}
template<class _Traits> 
MFostream<_Traits>* MFostream<_Traits>::flush() {
	return this;
}
template<class _Traits> 
MFostream<_Traits>* MFostream<_Traits>::seekp(std::streampos pos) {
	if ( !mf || !mf->IsOpen() ) {
		iostate |= failbit;
	} else {
		char* target = mf->Data() + pos - 1;
		if ( target >= mf->Eof() ) {
			iostate |= eofbit;
		} else {
			currentPosition = target;
		}
	}
	return this;
}
template<class _Traits> 
MFostream<_Traits>* MFostream<_Traits>::seekp(std::streamoff off, std::ios_base::seekdir way) {
	if ( !mf || !mf->IsOpen() ) {
		iostate |= failbit;
	} else {
		char* target = 0;
		switch (way) {
		case ios_base::beg:
			target = mf->Data() + off;
			break;
		case ios_base::cur:
			target = currentPosition + off;
			break;
		case ios_base::end:
			target = mf->Eof() + off;
		}
		if ( target == 0 ) {
			iostate |= badbit;
		} else if ( target >= mf->Eof() ) {
			iostate |= eofbit;
		} else if ( target < mf->Data() ) {
			iostate |= failbit;
		} else {
			currentPosition = target;
		}
	}
	return this;
}
template<class _Traits> 
std::streampos MFostream<_Traits>::tellp() {
	std::istream::streampos currentPos = -1;
	if ( mf && mf->IsOpen() ) {
		currentPos = currentPosition - mf->Data();
	}
	return currentPos;
}
template<class _Traits> 
MFostream<_Traits>& MFostream<_Traits>::write(const char* s, std::streamsize n) {
	if ( !mf || !mf->IsOpen() ) {
		iostate = failbit;
	} else if ( currentPosition + n > mf->Eof() ) {
		iostate = eofbit;
	} else {
		memcpy(currentPosition,s,n);
		currentPosition += n;
	}
	return *this;
}