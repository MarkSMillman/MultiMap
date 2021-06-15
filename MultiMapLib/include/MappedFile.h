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
#include "MultiMap.h"
#include "MFUtils.h"
#include "boost/iostreams/device/mapped_file.hpp"
#include "boost/exception/exception.hpp"

class MappedFile {
public:
	MULTIMAP_API ~MappedFile(void);
	MULTIMAP_API MappedFile(void);

	MULTIMAP_API MappedFile(const MappedFile& other);
	MULTIMAP_API MappedFile& operator=(const MappedFile& other);
    MULTIMAP_API bool operator==(const MappedFile &other) const;
    MULTIMAP_API bool operator!=(const MappedFile &other) const;

	MULTIMAP_API char* OpenReadOnly(std::string fileName);
	MULTIMAP_API char* OpenReadWrite(std::string fileName, long long size = 0, bool deleteIfExists = false);

	MULTIMAP_API char* Data();
	MULTIMAP_API char* Eof();
	MULTIMAP_API std::string Path();

	MULTIMAP_API void Close(char * & data, long long _size = -1);
	MULTIMAP_API void Close();

	MULTIMAP_API bool IsOpen();
	MULTIMAP_API bool IsClosed();
	MULTIMAP_API bool IsReadWrite();
	MULTIMAP_API bool IsReadOnly();
	MULTIMAP_API bool IsCopy();

	MULTIMAP_API long long Size(bool useHighestLocation = false);
	MULTIMAP_API void Truncate();
	MULTIMAP_API char* Extend(long long biggerSize);

	MULTIMAP_API std::string GetLastError(void);

private:
	void ResetErrorMessage(void);
	bool                                  copy;
	boost::iostreams::mapped_file         * mf;
	boost::iostreams::mapped_file_params * mfp;
	char*                                 data;
	long long                         sizeUsed;
	char*                      highestLocation;
	char*                                  eof;
	long long                    sizeAllocated;
	std::string                       filePath;
	char                    errorMessage[1024];
	MFUtils                            mfUtils;

	friend class _IOBase;
};