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
#include <stdint.h>
#include "MultiMap.h"
#include "GenericType.h"


#pragma pack(push,1)

/**
EXTRA BYTES: (optional)

User ID: LASF_Spec
Record ID: 4
Record Length after Header: n records x 192 bytes

The Extra Bytes VLR provides a mechanism whereby additional information can be added to the
end of a standard Point Record. This VLR has been added to LAS 1.4 to formalize a process that
has been used in prior versions of LAS. It is envisioned that software that is not cognizant of the
meaning of the extra bytes will simply copy these bytes when manipulating files.

This record is only needed for LAS files where points contain user-defined “extra bytes”. This
happens when the point record size is set to a larger value than required by the point type. For
example, when a LAS file that contains point type 1 has a point record size of 32 instead of 28
there are 4 “extra bytes”. The Extra Bytes VLR contains a simple description of the type and the
meaning of these “extra bytes” so they can be accessed in a consistent manner across
applications. The 4 “extra bytes” could, for example, be of data_type 9 - a floating point value -
that specifies an "echo width" for each return. In this case there would be one EXTRA_BYTES
struct in the payload of this VLR. An example with two EXTRA_BYTES struct in the payload are
14 "extra bytes" that have data type 29 - a floating point vector of length 3 - followed by data type
3 - an unsigned short - that specify the "laser pulse direction" and a "normalized reflectivity".

The "extra bytes" are made accessible via a unique name. The "name" field distinguishes the
additional point attributes that a LIDAR software may add to the points in a LAS file so they can
be accessed later in a consistent manner by another software. Descriptive names such as
"normalized reflectivity", "echo width", or "shading normal" are encouraged. The use of generic
names such as "variable1" or "temp1" is discouraged.

The bit mask in the options field specifies whether the min and max range of the value have been
set (i.e. are meaningful), whether the scale and/or offset values are set with which the “extra
bytes” are to be multiplied and translated to compute their actual value, and whether there is a
special value that should be interpreted as NO_DATA. By default all bits are zero which means
that the values in the corresponding fields are to be disregarded.
If the selected data_type is less than 8 bytes, the no_data, min, and max field should be upcast
into 8-byte storage. For any float these 8 bytes would be upcast to a double, for any unsigned
char, unsigned short, or unsigned long they would be upcast to an unsigned long long and for any
char, short, or long, they would be upcast to a long long.

A LAS file contains “undocumented extra bytes” when there are “extra bytes” but when there is no
Extra Bytes VLR that describes them or when there are more “extra bytes” than described in an
existing Extra Bytes VLR.

When adding an “Extra Bytes” VLR to a LAS file that contains “undocumented extra bytes” they
must be “described” as data_type == 0 with the options bit field storing the number of
undocumented bytes.

A LAS file has an “extra bytes mismatch” if the Extra Bytes VLR describes more “extra bytes”
than each LAS point actually has. The occurrence of an “extra bytes mismatch” renders the Extra
Bytes VLR invalid.
**/

typedef enum {
	EB_UNDOCUMENTED=0,
	EB_UINT8=1,
	EB_INT8=2,
	EB_UINT16=3,
	EB_INT16=4,
	EB_UINT32=5,
	EB_INT32=6,
	EB_UINT64=7,
	EB_INT64=8,
	EB_FLOAT=9,
	EB_DOUBLE=10,
	EB_UINT8_2=11,
	EB_INT8_2=12,
	EB_UINT16_2=13,
	EB_INT16_2=14,
	EB_UINT32_2=15,
	EB_INT32_2=16,
	EB_UINT64_2=17,
	EB_INT64_2=18,
	EB_FLOAT_2=19,
	EB_DOUBLE_2=20,
	EB_UINT8_3=21,
	EB_INT8_3=22,
	EB_UINT16_3=23,
	EB_INT16_3=24,
	EB_UINT32_3=25,
	EB_INT32_3=26,
	EB_UINT64_3=27,
	EB_INT64_3=28,
	EB_FLOAT_3=29,
	EB_DOUBLE_3=30
} EB_DATA_TYPE;
#define UNSIGNED 1
#define SIGNED   2
#define REAL     3
static const unsigned char ebType[31] = {0,
	UNSIGNED,SIGNED,UNSIGNED,SIGNED,UNSIGNED,SIGNED,REAL,REAL,
	UNSIGNED,SIGNED,UNSIGNED,SIGNED,UNSIGNED,SIGNED,REAL,REAL,
	UNSIGNED,SIGNED,UNSIGNED,SIGNED,UNSIGNED,SIGNED,REAL,REAL
};
typedef enum {
	EB_NO_DATA_BIT=0,
	EB_MIN_BIT=1,
	EB_MAX_BIT=2,
	EB_SCALE_BIT=3,
	EB_OFFSET_BIT=4
} EB_OPTIONS;
#define EB_NOT_FOUND ULLONG_MAX
class ExtraBytes
{
public:
	MULTIMAP_API ExtraBytes(void);
	MULTIMAP_API ExtraBytes(EB_DATA_TYPE type, std::string name, std::string description);
	MULTIMAP_API static size_t GetByName(std::vector<ExtraBytes> extraBytes, std::string name, bool caseInsensitive = false);
	MULTIMAP_API static bool AddOrReplace(std::vector<ExtraBytes> extraBytes, EB_DATA_TYPE type, std::string name, std::string description );
	MULTIMAP_API static bool AddOrReplace(std::vector<ExtraBytes> extraBytes, EB_DATA_TYPE type, std::string name, std::string description, 
		size_t width, size_t* _no_data, size_t* _min, size_t* _max, double* _scale, double* _offset );

	MULTIMAP_API bool Validate(GenericType gType, size_t index=0);

	uint8_t   reserved[2]; // 2 bytes
	uint8_t     data_type; // 1 byte // See EB_DATA_TYPE below
	uint8_t       options; // 1 byte // See EB_OPTIONS below
	char         name[32]; // 32 bytes
	uint8_t     unused[4]; // 4 bytes
	uint64_t   no_data[3]; // 24 = 3*8 bytes
	uint64_t       min[3]; // 24 = 3*8 bytes
	uint64_t       max[3]; // 24 = 3*8 bytes
	double       scale[3]; // 24 = 3*8 bytes
	double      offset[3]; // 24 = 3*8 bytes
	char  description[32]; // 32 bytes
};

#pragma pack(pop)