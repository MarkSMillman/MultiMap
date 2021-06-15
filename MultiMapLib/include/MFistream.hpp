#pragma once
#include "MultiMap.h"
#include "MappedFile.h"

DISABLE_WARNINGS
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/stream.hpp>
ENABLE_WARNINGS

class MappedFile;
template<class _Traits> 
class MFistream : public boost::iostreams::stream<_Traits>{
public:
	~MFistream<_Traits>(void);
	MFistream<_Traits>();

	void close();
	bool is_open() const;
	void open(const char* filename, std::ios_base::openmode mode = std::ios_base::in);
	void open(const std::string& filename, std::ios_base::openmode mode = std::ios_base::in);

	std::streamsize gcount() const;
	int peek();
	MFistream<_Traits>& read (char_type* s, std::streamsize n);
	MFistream<_Traits>* seekg(std::streampos pos);
	MFistream<_Traits>* seekg(std::streamoff off, std::ios_base::seekdir way);
	int sync();
	std::istream::streampos tellg();
	MFistream<_Traits>& unget();



protected:
	MappedFile* mf;
	char* currentPosition;
	size_t last_read_count;
	std::ios::iostate iostate;
};
template<class _Traits> 
MFistream<_Traits>::~MFistream(void) {
}
template<class _Traits> 
MFistream<_Traits>::MFistream(void) {
	mf = NULL;
	currentPosition = NULL;
	last_read_count = 0;
	iostate = 0;
}
template<class _Traits> 
void MFistream<_Traits>::open(const char* filename, std::ios_base::openmode mode) {
	open(std::string(filename),mode);
}
template<class _Traits> 
void MFistream<_Traits>::open(const std::string& filename, std::ios_base::openmode mode) {
	if ( is_open() ) {
		close();
	}
	mf = new MappedFile();
	if ( mode & std::ios_base::out ) {
		currentPosition = mf->OpenReadWrite(filename);
	} else {
		currentPosition = mf->OpenReadOnly(filename);
	}
	last_read_count = 0;
}
template<class _Traits> 
void MFistream<_Traits>::close() {
	if ( mf ) {
		if ( mf->IsReadOnly() ) {
			mf->Close();
		} else {
			char* _data = mf->Data();
			mf->Close(_data);
		}
		mf = NULL;
	}
}
template<class _Traits> 
bool MFistream<_Traits>::is_open() const {
	if ( mf && mf->IsOpen() ) {
		return true;
	} else {
		return false;
	}
}
template<class _Traits> 
std::streamsize MFistream<_Traits>::gcount() const {
	return last_read_count;
}
template<class _Traits> 
int MFistream<_Traits>::peek(void) {
	int val = 0;
	if ( !mf || !mf->IsOpen() ) {
		val = EOF;
		iostate = failbit;
	} else if ( currentPosition > mf->Eof() ) {
		val = EOF;
		iostate = eofbit;
	} else {
		val = static_cast<int>(*currentPosition);
	}
	return val;
}
template<class _Traits> 
MFistream<_Traits>& MFistream<_Traits>::read(char_type* s, std::streamsize n) {
	if ( !mf || !mf->IsOpen() ) {
		iostate = failbit;
	} else if ( (currentPosition + n - 1) > mf->Eof() ) {
		iostate = eofbit;
	} else {
		memcpy(s,currentPosition,n);
		currentPosition += n;
		last_read_count = n;
	}
	return *this;
}
template<class _Traits> 
MFistream<_Traits>* MFistream<_Traits>::seekg(std::streampos pos) {
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
MFistream<_Traits>* MFistream<_Traits>::seekg(std::streamoff off, std::ios_base::seekdir way) {
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
int MFistream<_Traits>::sync(void) {
	int status = 0;
	if ( !mf || !mf->IsOpen() ) {
		status = -1;
	}
	return status;
}
template<class _Traits> 
std::istream::streampos MFistream<_Traits>::tellg() {
	std::istream::streampos currentPos = -1;
	if ( mf && mf->IsOpen() ) {
		currentPos = currentPosition - mf->Data();
	}
	return currentPos;
}
template<class _Traits> 
MFistream<_Traits>& MFistream<_Traits>::unget() {
	if ( !mf || !mf->IsOpen() ) {
		iostate |= failbit;
	} else if ( currentPosition - 1 < mf->Data() ) {
		iostate |= failbit;
	} else if ( currentPosition - 1 > mf->Eof() ) {
		iostate |= failbit;
	} else {
		currentPosition--;
	}
	return *this;
}