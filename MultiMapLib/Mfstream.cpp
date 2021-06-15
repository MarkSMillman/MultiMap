#pragma warning ( disable : 4251 )
#include "MultiMap.h"
#include "Mfstream.h"
#include "MappedFile.h"
using namespace FLIDAR;

MULTIMAP_API Mfstream::~Mfstream(void) {
}

MULTIMAP_API Mfstream::Mfstream(void) {
	mf = NULL;
	currentPosition = NULL;
	last_read_count = 0;
	iostate = 0;
}

MULTIMAP_API void Mfstream::open(const char* filename, std::ios_base::openmode mode) {
	open(std::string(filename),mode);
}

MULTIMAP_API void Mfstream::open(const std::string& filename, std::ios_base::openmode mode) {
	if ( is_open() ) {
		close();
	}
	mf = new FLIDAR::MappedFile();
	if ( mode & std::ios_base::out ) {
		currentPosition = mf->OpenForUpdate(filename);
	} else {
		currentPosition = mf->OpenReadOnly(filename);
	}
	last_read_count = 0;
}

MULTIMAP_API void Mfstream::close() {
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

MULTIMAP_API bool Mfstream::is_open() const {
	if ( mf && mf->IsOpen() ) {
		return true;
	} else {
		return false;
	}
}

MULTIMAP_API std::streamsize Mfstream::gcount() const {
	return last_read_count;
}

MULTIMAP_API int Mfstream::peek(void) {
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

MULTIMAP_API Mfstream& Mfstream::read(char_type* s, std::streamsize n) {
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

MULTIMAP_API Mfstream* Mfstream::seekg(std::streampos pos) {
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

MULTIMAP_API Mfstream* Mfstream::seekg(std::streamoff off, std::ios_base::seekdir way) {
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

MULTIMAP_API int Mfstream::sync(void) {
	int status = 0;
	if ( !mf || !mf->IsOpen() ) {
		status = -1;
	}
	return status;
}

MULTIMAP_API std::istream::streampos Mfstream::tellg() {
	std::istream::streampos currentPos = -1;
	if ( mf && mf->IsOpen() ) {
		currentPos = currentPosition - mf->Data();
	}
	return currentPos;
}

MULTIMAP_API Mfstream& Mfstream::unget() {
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
MULTIMAP_API Mfstream* Mfstream::flush() {
	return this;
}

MULTIMAP_API Mfstream* Mfstream::seekp(std::streampos pos) {
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

MULTIMAP_API Mfstream* Mfstream::seekp(std::streamoff off, std::ios_base::seekdir way) {
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

MULTIMAP_API std::streampos Mfstream::tellp() {
	std::istream::streampos currentPos = -1;
	if ( mf && mf->IsOpen() ) {
		currentPos = currentPosition - mf->Data();
	}
	return currentPos;
}

MULTIMAP_API Mfstream& Mfstream::write(const char* s, std::streamsize n) {
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

void Mfstream::_Add_vtordisp1() {
	// required to meet virtual requirements of fstream
}
void Mfstream::_Add_vtordisp2() {
	// required to meet virtual requirements of fstream
}