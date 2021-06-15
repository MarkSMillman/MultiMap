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
#pragma warning ( disable : 4251 )
#include "MultiMap.h"
#include "MappedFile.h"

MULTIMAP_API MappedFile::~MappedFile(void) {
	if ( !copy && mf != NULL ){
		if (mf->is_open() ) {
			mf->close();
		}
		if ( mf )
			delete mf;
		if ( mfp )
			delete mfp;
		mf = NULL;
		mfp = NULL;
	}
}

MULTIMAP_API MappedFile::MappedFile(void) {
	copy = false;
	data = NULL;
	highestLocation = NULL;
	mf = NULL;
	mfp = NULL;
	sizeAllocated = 0;
	sizeUsed = 0;
	eof = 0;
	ResetErrorMessage();
}

MULTIMAP_API MappedFile::MappedFile(const MappedFile& other) {
	*this = other;
}

/**
* Comparison operator
**/
MULTIMAP_API bool MappedFile::operator==(const MappedFile &other) const {
	bool equal = false;
	if ( copy == other.copy ) {
		if ( mf == other.mf ) {
			if ( mfp == other.mfp ) {
				if ( data == other.data ) {
					if (filePath.compare(other.filePath) == 0 ) {
						if ( sizeAllocated == other.sizeAllocated ) {
							equal = true;
						}
					}
				}
			}
		}
	}
	return equal;
}
MULTIMAP_API bool MappedFile::operator!=(const MappedFile &other) const {
	return !(*this == other);
}

/**
* Assignment operator
**/
MULTIMAP_API MappedFile&MappedFile::operator=(const MappedFile& other) {
	if ( this != &other ) {
		copy = true;
		mf = other.mf;
		mfp = other.mfp;
		data = other.data;
		filePath = other.filePath;
		sizeAllocated = other.sizeAllocated;
		highestLocation = other.highestLocation;
	}
	return *this;
}
MULTIMAP_API char* MappedFile::OpenReadOnly(std::string _filePath) {
	assert(!copy);

	ResetErrorMessage();
	const char * _data = NULL;
	if ( mf != NULL ){
		if ( mf->is_open() ) {
			mf->close();
		}
		if ( mf )
			delete mf;
		if ( mfp )
			delete mfp;
		mf = NULL;
		mfp = NULL;
	}
	if ( mf == NULL  ) 
	{
		data = NULL;
		filePath = _filePath;
		mfp = new boost::iostreams::mapped_file_params(filePath);
		mfp->mode = boost::iostreams::mapped_file::readonly;
		mfp->offset = 0;
		try {
			mf = new boost::iostreams::mapped_file(*mfp);
			if ( mf->is_open() ) 
			{
				_data = mf->const_data();
				data = (char*)_data;
				sizeAllocated = mf->size();
				eof = data + sizeAllocated - 1;
			}
		} catch ( std::exception const& ex  ) {
			std::string exx = ex.what() + _filePath;
			strcpy(errorMessage,exx.c_str());
		}
	} else {
		std::string message = std::string("MappedFile::OpenReadOnly failed to open ").append(_filePath);
		strcpy(errorMessage,message.c_str());
	}

	return data;
}

MULTIMAP_API char* MappedFile::OpenReadWrite(std::string _filePath, long long size, bool deleteIfExists){
	assert(!copy);

	ResetErrorMessage();
	if ( mf != NULL ) {
		if ( mf->is_open() ) {
			mf->close();
		}
		if ( mf ) 
			delete mf;
		if ( mfp )
			delete mfp;
		mfp = NULL;
		mf = NULL;
	}
	data = NULL;

	long long available = mfUtils.AvailableSpace(_filePath);
	if (size < available ) {
		char * _data = NULL;
		try {
			boost::filesystem::path __filePath(_filePath);
			if ( deleteIfExists && boost::filesystem::exists(__filePath) ) {
				if ( boost::filesystem::is_regular_file(__filePath) ) {
					boost::filesystem::remove(__filePath);
				}
			} else if ( !deleteIfExists && boost::filesystem::exists(__filePath) ) {
				//throw boost::enable_error_info(std::runtime_error("MappedFile::OpenReadWrite file already exists "+_filePath));
			}

			if ( mf == NULL  ) {
				data = NULL;
				filePath = _filePath;
				if ( size > 0 ) {
					mfUtils.FolderExists(filePath,true);
				}
				mfp = new boost::iostreams::mapped_file_params(filePath);
				mfp->mode = boost::iostreams::mapped_file::readwrite;
				if ( size > 0 ) { // MILLMAN 20120921 for reopen use see ::Extend(long long size)
					mfp->new_file_size = size;
				}
				mfp->offset = 0;
				try {
					mf = new boost::iostreams::mapped_file(*mfp);
					if ( mf->is_open() ) 
					{
						_data = mf->data();
						data = (char*)_data;
						sizeAllocated = mf->size();
						eof = data + sizeAllocated - 1;
					}
				} catch ( std::exception const& ex  ) {
					std::string exx = std::string("MappedFile::OpenReadWrite ").append(std::string(ex.what()).append(_filePath));
					strcpy(errorMessage,exx.c_str());
				}
			} else {
				std::string message = "MappedFile::OpenReadWrite failed to open "+_filePath;
				strcpy(errorMessage,message.c_str());
			}
		} catch ( std::exception const& ex ) {
			strcpy(errorMessage,ex.what());
		}
	} else {
		std::string message = "MappedFile::Not enough available space on device to create "+_filePath;
		strcpy(errorMessage,message.c_str());
	}
	return data;
}

MULTIMAP_API char* MappedFile::Data() {
	char* _data = NULL;
	if (mf != NULL && mf->is_open() ) {
		_data = data;
	}
	return _data;
}
MULTIMAP_API char* MappedFile::Eof() {
	char* _eof = NULL;
	if (mf != NULL && mf->is_open() ) {
		_eof = eof;
	}
	return _eof;
}
MULTIMAP_API std::string MappedFile::Path() {
	return filePath;
}

MULTIMAP_API void MappedFile::Close(void){
	if (!copy) {
		// don't close copies
		ResetErrorMessage();
		if ( mf != NULL && mf->is_open() ) {
			assert (IsReadOnly()); // this should only be called for readonly files.
			mf->close();
			if ( mf )
				delete mf;
			if ( mfp )
				delete mfp;
			mf = NULL;
			mfp = NULL;
		}
		data = NULL;
	}
}

MULTIMAP_API void MappedFile::Close(char* & _data, long long _size){
	assert(!copy);

	ResetErrorMessage();
	bool isreadwrite = false;
	long long actualSize = 0;
	if ( mf!=NULL && mf->is_open() ) {
		actualSize = mf->size();
		isreadwrite = IsReadWrite();
		mf->close();
		if ( mf )
			delete mf;
		if ( mfp )
			delete mfp;
		mf = NULL;
		mfp = NULL;
	}

	data = NULL;
	_data = NULL;
	eof = NULL;

	if ( isreadwrite && _size >= 0 && _size < actualSize ) {
		boost::filesystem::resize_file(filePath,_size);
	}
}

MULTIMAP_API void MappedFile::Truncate() {
	assert(!copy);

	ResetErrorMessage();
	if ( IsClosed() && sizeUsed < sizeAllocated && sizeUsed > 0 ) {
		boost::filesystem::resize_file(filePath,sizeUsed);
	}
}

MULTIMAP_API char* MappedFile::Extend(long long biggerSize) {
	assert(!copy);

	ResetErrorMessage();
	char * newAddress = NULL;
	bool wasOpen = false;
	bool wasReadWrite = false;
	long long size;
	if ( IsOpen() ) {
		newAddress = data;
		wasOpen = true;
		size = Size();
		if ( IsReadWrite() ) {
			wasReadWrite = true;
		}
		Close(data);
	} else {
		size = boost::filesystem::file_size(filePath);
	}
	if ( size < biggerSize ) {
		boost::filesystem::resize_file(filePath, biggerSize);
		if ( wasOpen ) {
			if ( wasReadWrite ) {
				newAddress = OpenReadWrite(filePath,0LL);
			} else {
				newAddress = OpenReadOnly(filePath);
			}
		}
	}
	return newAddress;
}

MULTIMAP_API bool MappedFile::IsOpen(){
	bool isopen = false;
	if ( mf!=NULL && mf->is_open() ) {
		isopen = true;
	}
	return isopen;
}

MULTIMAP_API bool MappedFile::IsCopy() {
	bool isCopy = false;
	if ( copy ) {
		isCopy = true;
	}
	return isCopy;
}

MULTIMAP_API bool MappedFile::IsClosed(){
	bool isclosed = true;
	if ( mf!=NULL && mf->is_open() ) {
		isclosed = false;
	}
	return isclosed;
}

MULTIMAP_API long long MappedFile::Size(bool useHighestLocation){
	long long _size = -1;
	if ( mf!=NULL && mf->is_open() ) {
		_size = mf->size();
		if ( useHighestLocation ) {
			_size = highestLocation - mf->data();
		}
	}
	return _size;
}

MULTIMAP_API bool MappedFile::IsReadWrite() {
	bool isreadwrite = false;
	if ( mf!=NULL && mf->is_open() ) {
		if (mf->flags() & boost::iostreams::mapped_file::readwrite){
			isreadwrite = true;
		}
	}
	return isreadwrite;
}

MULTIMAP_API bool MappedFile::IsReadOnly(){
	bool isreadonly = false;
	if ( mf!=NULL && mf->is_open() ) {
		if (mf->flags() & boost::iostreams::mapped_file::readonly){
			isreadonly = true;
		}
	}
	return isreadonly;
}

void MappedFile::ResetErrorMessage(void) {
	errorMessage[0] = '\0';
}
MULTIMAP_API std::string MappedFile::GetLastError(void) {
	return std::string(errorMessage);
}