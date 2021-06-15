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

#if defined (_WIN32)  || defined(_WIN64)
#ifdef _USRDLL
#define MULTIMAP_API __declspec(dllexport)
#else
#define MULTIMAP_API __declspec(dllimport)
#endif
#else if defined (__linux__) 
#define MULTIMAP_API
#endif

#ifndef DISABLE_WARNINGS
#define DISABLE_WARNINGS \
__pragma (warning (push)) \
__pragma (warning (disable : 26439 26444 26450 26451 26495 26498 26812 28182 28251 4003 4005 4100 4101 4127 4189 4201 4251 4267 4290 4503 4575 4710 4996 6001 6011 6031 6201 6248 6269 6282 6285 6286 6294 6308 6385 6386 6387 6388))
#endif
#ifndef ENABLE_WARNINGS
#define ENABLE_WARNINGS __pragma (warning(pop))
#endif

#if defined (_WIN32)  || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define __windows__
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#endif

DISABLE_WARNINGS
#include <vector>
#include <string>
ENABLE_WARNINGS

// These are used for annotative purposes only so that the CPP file has a indication of that is defined in the H file.
// These may not be used consistently and PUBLIC will seldom be used and can be considered a default
#define PRIVATE
#define PROTECTED
#define PUBLIC
#define STATIC
#define VIRTUAL

#if defined (_WIN32)  || defined(_WIN64)
#define FILE_SEP "\\"
#else if defined (__linux__) 
#define FILE_SEP "/"
#endif

static const double MMM_PI =3.14159265358979323846264338327950288419716939937510;
static const double MMM_PI_OVER_180 = (MMM_PI / 180.0);
static const double MMM_DEGREES_TO_RADIANS = MMM_PI_OVER_180;
static const double MMM_PI_UNDER_180 = (180.0 / MMM_PI);
static const double MMM_RADIANS_TO_DEGREES = MMM_PI_UNDER_180;
static const double MMM_TWOPI = MMM_PI*2.0;
static const double MMM_TWOTHIRDSPI = MMM_TWOPI / 3.0;
static const double MMM_FOURTHIRDSPI = MMM_TWOPI*2.0 / 3.0;

static const unsigned long MMM_MEGABYTE = 1048576;
static const unsigned long MMM_GIGABYTE = 1073741824;
static const unsigned long MMM_HALFGIG = 536870912;
static const unsigned long long MMM_MEGABYTE_ULL = 1048576ULL;
static const unsigned long long GMMM_IGABYTE_ULL = 1073741824ULL;

#define MMM_MIN_MAPPED_MEMORY_REQUIREMENT (8 * GIGABYTE_ULL) // make this larger once we get File IO tested

typedef enum {IOT_UNKNOWN=-1, IOT_FOLDER=0, IOT_BIN=1, IOT_LAS=2, IOT_LAZ=3, IOT_ASCII=4, IOT_TIF=5, IOT_JPG=6, IOT_SHP=7, IOT_DBF=8, IOT_LDR=9} MMM_IOBTYPE;