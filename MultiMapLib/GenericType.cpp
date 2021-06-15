#include "MultiMap.h"
#include "GenericType.h"
#include <sstream>

/**
* maximums is indexed by DTYPE
**/
STATIC const unsigned long long maximums[DT_MAX] = {0,0,SHRT_MAX,USHRT_MAX,LONG_MAX,ULONG_MAX,LLONG_MAX,ULLONG_MAX,ULLONG_MAX,ULLONG_MAX,0}; 

MULTIMAP_API GenericType::GenericType(void) {
	type = DT_UNDEF;
	memset(&data,0,sizeof(data));
}
MULTIMAP_API GenericType::GenericType(short _short) {
	type = DT_SHRT;
	data.short_ = _short;
}
MULTIMAP_API GenericType::GenericType(unsigned short _ushort) {
	type = DT_USHRT;
	data.ushort_ = _ushort;
}
MULTIMAP_API GenericType::GenericType(long _long) {
	type = DT_LONG;
	data.long_ = _long;
}
MULTIMAP_API GenericType::GenericType(unsigned long _ulong) {
	type = DT_ULONG;
	data.ulong_ = _ulong;
}
MULTIMAP_API GenericType::GenericType(long long _llong) {
	type = DT_LLONG;
	data.llong_ = _llong;
}
MULTIMAP_API GenericType::GenericType(unsigned long long _ullong) {
	type = DT_ULLONG;
	data.ullong_ = _ullong;
}
MULTIMAP_API GenericType::GenericType(float _float) {
	type = DT_FLT;
	data.float_ = _float;
}
MULTIMAP_API GenericType::GenericType(double _double) {
	type = DT_DBL;
	data.double_ = _double;
}

MULTIMAP_API GenericType::GenericType(const GenericType& other) {
	*this = other;
}
MULTIMAP_API GenericType::GenericType(DTYPE _type, void* _data) {
	type = type;
	switch (type) {
	case DT_UCHAR:
		data.uchar_ = *((unsigned char*)_data);
	case DT_CHAR:
		data.char_ = *((char*)_data);
		break;
	case DT_USHRT:
		data.ushort_ = *((unsigned short*)_data);
	case DT_SHRT:
		data.short_ = *((short*)_data);
		break;
	case DT_ULONG:
		data.ulong_ = *((unsigned long*)_data);
		break;
	case DT_LONG:
		data.long_ = *((long*)_data);
		break;
	case DT_ULLONG:
		data.ullong_ = *((unsigned long long*)_data);
		break;
	case DT_LLONG:
		data.llong_ = *((long long*)_data);
		break;
	case DT_FLT:
		data.float_ = *((float*)_data);
		break;
	case DT_DBL:
		data.double_ = *((double*)_data);
		break;
	}
}

MULTIMAP_API GenericType& GenericType::operator=(const GenericType& other) {
	type = other.type;
	data = other.data;
	return *this;
}
std::ostream& operator<< (std::ostream& stream, const GenericType& other) {
	stream << ((GenericType)other).ToString();
	return stream;
}
void MULTIMAP_API GenericType::Set(char _char) {
	type = DT_CHAR;
	data.char_ = _char;
}
void MULTIMAP_API GenericType::Set(unsigned char _uchar) {
	type = DT_UCHAR;
	data.ushort_ = _uchar;
}
void MULTIMAP_API GenericType::Set(short _short) {
	type = DT_SHRT;
	data.short_ = _short;
}
void MULTIMAP_API GenericType::Set(unsigned short _ushort) {
	type = DT_USHRT;
	data.ushort_ = _ushort;
}
void MULTIMAP_API GenericType::Set(long _long) {
	type = DT_LONG;
	data.long_ = _long;
}
void MULTIMAP_API GenericType::Set(unsigned long _ulong) {
	type = DT_ULONG;
	data.ulong_ = _ulong;
}
void MULTIMAP_API GenericType::Set(int _int) {
	type = DT_LONG;
	data.long_ = _int;
}
void MULTIMAP_API GenericType::Set(unsigned int _uint) {
	type = DT_ULONG;
	data.ulong_ = _uint;
}
void MULTIMAP_API GenericType::Set(long long _llong) {
	type = DT_LLONG;
	data.llong_ = _llong;
}
void MULTIMAP_API GenericType::Set(unsigned long long _ullong) {
	type = DT_ULLONG;
	data.ullong_ = _ullong;
}
void MULTIMAP_API GenericType::Set(float _float) {
	type = DT_FLT;
	data.float_ = _float;
}
void MULTIMAP_API GenericType::Set(double _double) {
	type = DT_DBL;
	data.double_ = _double;
}

bool MULTIMAP_API GenericType::Signed(void) {
	bool compares = false;
	switch (type) {
	case DT_UCHAR:
	case DT_USHRT:
	case DT_ULONG:
	case DT_ULLONG:
		compares = false;
		break;
	case DT_CHAR:
	case DT_SHRT:
	case DT_LONG:
	case DT_LLONG:
	case DT_FLT:
	case DT_DBL:
		compares = true;
		break;
	}
	return compares;
}

bool MULTIMAP_API GenericType::Integer(void) {
	bool compares = false;
	switch (type) {
	case DT_UCHAR:
	case DT_CHAR:
	case DT_SHRT:
	case DT_USHRT:
	case DT_LONG:
	case DT_ULONG:
	case DT_LLONG:
	case DT_ULLONG:
		compares = true;
		break;
	case DT_FLT:
	case DT_DBL:
		compares = false;
		break;
	}
	return compares;
}
bool MULTIMAP_API GenericType::FloatingPoint(void) {
	bool compares = false;
	switch (type) {
	case DT_UCHAR:
	case DT_CHAR:
	case DT_SHRT:
	case DT_USHRT:
	case DT_LONG:
	case DT_ULONG:
	case DT_LLONG:
	case DT_ULLONG:
		compares = false;
		break;
	case DT_FLT:
	case DT_DBL:
		compares = true;
		break;
	}
	return compares;
}
GenericType MULTIMAP_API GenericType::Elevate(bool forceSigned) {
	GenericType elevated;
	switch (type) {
	case DT_CHAR:
		elevated.Set((short)data.char_);
		break;
	case DT_UCHAR:
		if ( forceSigned ) {
			elevated.Set((short)data.uchar_);
		} else {
			elevated.Set((unsigned short)data.uchar_);
		}
		break;
	case DT_SHRT:
		elevated.Set((long)data.short_);
		break;
	case DT_USHRT:
		if ( forceSigned ) {
			elevated.Set((long)data.ushort_);
		} else {
			elevated.Set((unsigned long)data.ushort_);
		}
		break;
	case DT_LONG:
		elevated.Set((long long)data.long_);
		break;
	case DT_ULONG:
		if ( forceSigned ) {
			elevated.Set((long long)data.ulong_);
		} else {
			elevated.Set((unsigned long long)data.ulong_);
		}
		break;
	case DT_LLONG:
		elevated.Set(data.llong_);
		break;
	case DT_ULLONG:
		if ( forceSigned ) {
			elevated.Set((double)data.ullong_);
		} else {
			elevated.Set(data.ullong_);
		}
		break;
	case DT_FLT:
		elevated.Set((double)(data.float_));
		break;
	case DT_DBL:
		elevated.Set(data.double_);
		break;
	}
	return elevated;
}
unsigned long long MULTIMAP_API GenericType::Maximum() {
	if ( type < DT_MAX ) {
		return maximums[type];
	} else {
		return 0LL;
	}
}
unsigned long long MULTIMAP_API GenericType::Maximum(DTYPE _type) {
	if ( _type < DT_MAX ) {
		return maximums[_type];
	} else {
		return 0LL;
	}
}

bool MULTIMAP_API GenericType::operator==(const GenericType & other) {
	bool compares = false;
	if ( type == other.type ) {
		switch (type) {
		case DT_CHAR:
			compares = (data.char_ == other.data.char_);
			break;
		case DT_UCHAR:
			compares = (data.uchar_ == other.data.uchar_);
			break;
		case DT_SHRT:
			compares = (data.short_ == other.data.short_);
			break;
		case DT_USHRT:
			compares = (data.ushort_ == other.data.ushort_);
			break;
		case DT_LONG:
			compares = (data.long_ == other.data.long_);
			break;
		case DT_ULONG:
			compares = (data.ulong_ == other.data.ulong_);
			break;
		case DT_LLONG:
			compares = (data.llong_ == other.data.llong_);
			break;
		case DT_ULLONG:
			compares = (data.ullong_ == other.data.ullong_);
			break;
		case DT_FLT:
			compares = (data.float_ == other.data.float_);
			break;
		case DT_DBL:
			compares = (data.double_ == other.data.double_);
			break;
		}
	} else {
		if ( Maximum() < Maximum(other.type) ) {
			compares = (ToDouble() == ((GenericType)other).ToDouble());
		} else {
			if ( Signed() == ((GenericType)other).Signed() ) {
				compares = (*this == ((GenericType)other).ToString());
			} else if ( !Signed() && ((GenericType)other).Signed() ) {
				GenericType elevated = Elevate(true);
				compares = (elevated == ((GenericType)other).ToString());
			} else if ( Signed() && !((GenericType)other).Signed() ) {
				GenericType elevated = ((GenericType)other).Elevate(true);
				compares = (*this == elevated.ToString());
			}
		}
	}
	return compares;
}
bool MULTIMAP_API GenericType::operator!=(const GenericType & other) {
	bool compares = false;
	if ( type == other.type ) {
		switch (type) {
		case DT_CHAR:
			compares = (data.char_ != other.data.char_);
			break;
		case DT_UCHAR:
			compares = (data.uchar_ != other.data.uchar_);
			break;
		case DT_SHRT:
			compares = (data.short_ != other.data.short_);
			break;
		case DT_USHRT:
			compares = (data.ushort_ != other.data.ushort_);
			break;
		case DT_LONG:
			compares = (data.long_ != other.data.long_);
			break;
		case DT_ULONG:
			compares = (data.ulong_ != other.data.ulong_);
			break;
		case DT_LLONG:
			compares = (data.llong_ != other.data.llong_);
			break;
		case DT_ULLONG:
			compares = (data.ullong_ != other.data.ullong_);
			break;
		case DT_FLT:
			compares = (data.float_ != other.data.float_);
			break;
		case DT_DBL:
			compares = (data.double_ != other.data.double_);
			break;
		}
	} else {
		if ( Maximum() < Maximum(other.type) ) {
			compares = (ToDouble() != ((GenericType)other).ToDouble());
		} else {
			if ( Signed() == ((GenericType)other).Signed() ) {
				compares = (*this != ((GenericType)other).ToString());
			} else if ( !Signed() && ((GenericType)other).Signed() ) {
				GenericType elevated = Elevate(true);
				compares = (elevated != ((GenericType)other).ToString());
			} else if ( Signed() && !((GenericType)other).Signed() ) {
				GenericType elevated = ((GenericType)other).Elevate(true);
				compares = (*this != elevated.ToString());
			}
		}
	}
	return compares;
}
bool MULTIMAP_API GenericType::operator<(const GenericType & other) {
	bool compares = false;
	if ( type == other.type ) {
		switch (type) {
		case DT_CHAR:
			compares = (data.char_ < other.data.char_);
			break;
		case DT_UCHAR:
			compares = (data.uchar_ < other.data.uchar_);
			break;
		case DT_SHRT:
			compares = (data.short_ < other.data.short_);
			break;
		case DT_USHRT:
			compares = (data.ushort_ < other.data.ushort_);
			break;
		case DT_LONG:
			compares = (data.long_ < other.data.long_);
			break;
		case DT_ULONG:
			compares = (data.ulong_ < other.data.ulong_);
			break;
		case DT_LLONG:
			compares = (data.llong_ < other.data.llong_);
			break;
		case DT_ULLONG:
			compares = (data.ullong_ < other.data.ullong_);
			break;
		case DT_FLT:
			compares = (data.float_ < other.data.float_);
			break;
		case DT_DBL:
			compares = (data.double_ < other.data.double_);
			break;
		}
	} else {
		if ( Maximum() < Maximum(other.type) ) {
			compares = (ToDouble() < ((GenericType)other).ToDouble());
		} else {
			if ( Signed() == ((GenericType)other).Signed() ) {
				compares = (*this < ((GenericType)other).ToString());
			} else if ( !Signed() && ((GenericType)other).Signed() ) {
				GenericType elevated = Elevate(true);
				compares = (elevated < ((GenericType)other).ToString());
			} else if ( Signed() && !((GenericType)other).Signed() ) {
				GenericType elevated = ((GenericType)other).Elevate(true);
				compares = (*this < elevated.ToString());
			}
		}
	}
	return compares;
}
bool MULTIMAP_API GenericType::operator<=(const GenericType & other) {
	bool compares = false;
	if ( type == other.type ) {
		switch (type) {
		case DT_CHAR:
			compares = (data.char_ <= other.data.char_);
			break;
		case DT_UCHAR:
			compares = (data.uchar_ <= other.data.uchar_);
			break;
		case DT_SHRT:
			compares = (data.short_ <= other.data.short_);
			break;
		case DT_USHRT:
			compares = (data.ushort_ <= other.data.ushort_);
			break;
		case DT_LONG:
			compares = (data.long_ <= other.data.long_);
			break;
		case DT_ULONG:
			compares = (data.ulong_ <= other.data.ulong_);
			break;
		case DT_LLONG:
			compares = (data.llong_ <= other.data.llong_);
			break;
		case DT_ULLONG:
			compares = (data.ullong_ <= other.data.ullong_);
			break;
		case DT_FLT:
			compares = (data.float_ <= other.data.float_);
			break;
		case DT_DBL:
			compares = (data.double_ <= other.data.double_);
			break;
		}
	} else {
		if ( Maximum() < Maximum(other.type) ) {
			compares = (ToDouble() <= ((GenericType)other).ToDouble());
		} else {
			if ( Signed() == ((GenericType)other).Signed() ) {
				compares = (*this <= ((GenericType)other).ToString());
			} else if ( !Signed() && ((GenericType)other).Signed() ) {
				GenericType elevated = Elevate(true);
				compares = (elevated <= ((GenericType)other).ToString());
			} else if ( Signed() && !((GenericType)other).Signed() ) {
				GenericType elevated = ((GenericType)other).Elevate(true);
				compares = (*this <= elevated.ToString());
			}
		}
	}
	return compares;
}
bool MULTIMAP_API GenericType::operator>(const GenericType & other) {
	bool compares = false;
	if ( type == other.type ) {
		switch (type) {
		case DT_CHAR:
			compares = (data.char_ > other.data.char_);
			break;
		case DT_UCHAR:
			compares = (data.uchar_ > other.data.uchar_);
			break;
		case DT_SHRT:
			compares = (data.short_ > other.data.short_);
			break;
		case DT_USHRT:
			compares = (data.ushort_ > other.data.ushort_);
			break;
		case DT_LONG:
			compares = (data.long_ > other.data.long_);
			break;
		case DT_ULONG:
			compares = (data.ulong_ > other.data.ulong_);
			break;
		case DT_LLONG:
			compares = (data.llong_ > other.data.llong_);
			break;
		case DT_ULLONG:
			compares = (data.ullong_ > other.data.ullong_);
			break;
		case DT_FLT:
			compares = (data.float_ > other.data.float_);
			break;
		case DT_DBL:
			compares = (data.double_ > other.data.double_);
			break;
		}
	} else {
		if ( Maximum() < Maximum(other.type) ) {
			compares = (ToDouble() > ((GenericType)other).ToDouble());
		} else {
			if ( Signed() == ((GenericType)other).Signed() ) {
				compares = (*this > ((GenericType)other).ToString());
			} else if ( !Signed() && ((GenericType)other).Signed() ) {
				GenericType elevated = Elevate(true);
				compares = (elevated > ((GenericType)other).ToString());
			} else if ( Signed() && !((GenericType)other).Signed() ) {
				GenericType elevated = ((GenericType)other).Elevate(true);
				compares = (*this > elevated.ToString());
			}
		}
	}
	return compares;
}
bool MULTIMAP_API GenericType::operator>=(const GenericType & other) {
	bool compares = false;
	if ( type == other.type ) {
		switch (type) {
		case DT_CHAR:
			compares = (data.char_ >= other.data.char_);
			break;
		case DT_UCHAR:
			compares = (data.uchar_ >= other.data.uchar_);
			break;
		case DT_SHRT:
			compares = (data.short_ >= other.data.short_);
			break;
		case DT_USHRT:
			compares = (data.ushort_ >= other.data.ushort_);
			break;
		case DT_LONG:
			compares = (data.long_ >= other.data.long_);
			break;
		case DT_ULONG:
			compares = (data.ulong_ >= other.data.ulong_);
			break;
		case DT_LLONG:
			compares = (data.llong_ >= other.data.llong_);
			break;
		case DT_ULLONG:
			compares = (data.ullong_ >= other.data.ullong_);
			break;
		case DT_FLT:
			compares = (data.float_ >= other.data.float_);
			break;
		case DT_DBL:
			compares = (data.double_ >= other.data.double_);
			break;
		}
	} else {
		if ( Maximum() < Maximum(other.type) ) {
			compares = (ToDouble() >= ((GenericType)other).ToDouble());
		} else {
			if ( Signed() == ((GenericType)other).Signed() ) {
				compares = (*this >= ((GenericType)other).ToString());
			} else if ( !Signed() && ((GenericType)other).Signed() ) {
				GenericType elevated = Elevate(true);
				compares = (elevated >= ((GenericType)other).ToString());
			} else if ( Signed() && !((GenericType)other).Signed() ) {
				GenericType elevated = ((GenericType)other).Elevate(true);
				compares = (*this >= elevated.ToString());
			}
		}
	}
	return compares;
}

bool MULTIMAP_API GenericType::operator==(std::string value) {
	bool compares = false;
	try {
		switch (type) {
		case DT_CHAR:
			{
				char v = static_cast<char>(strtol(value.c_str(),NULL,10));
				compares = (data.char_ == v);
			}
			break;
		case DT_UCHAR:
			{
				unsigned char v = static_cast<unsigned char>(strtoul(value.c_str(),NULL,10));
				compares = (data.uchar_  == v);
			}
			break;
		case DT_SHRT:
			{
				short v = static_cast<short>(strtol(value.c_str(),NULL,10));
				compares = (data.short_ == v);
			}
			break;
		case DT_USHRT:
			{
				unsigned short v = static_cast<unsigned short>(strtoul(value.c_str(),NULL,10));
				compares = (data.ushort_  == v);
			}
			break;
		case DT_LONG:
			{
				long v = strtol(value.c_str(),NULL,10);
				compares = (data.long_ == v);
			}
			break;
		case DT_ULONG:
			{
				unsigned long v = strtoul(value.c_str(),NULL,10);
				compares = (data.ulong_ == v);
			}
			break;
		case DT_LLONG:
			{
				long long v = 0;// strtoll(value.c_str(),NULL,10);
				sscanf(value.c_str(),"%lld",&v);
				compares = (data.llong_ == v);
			}
			break;
		case DT_ULLONG:
			{
				unsigned long long v = 0; //strtoull(value.c_str(),NULL,10);
				sscanf(value.c_str(),"%llu",&v);
				compares = (data.ullong_ == v);
			}
			break;
		case DT_FLT:
			{
				float v = 0.0f; //strtof(value.c_str(),NULL,10);
				sscanf(value.c_str(),"%f",&v);
				compares = (data.float_ == v);
			}
			break;
		case DT_DBL:
			{
				double v = 0.0; //strtod(value.c_str(),NULL,10);
				sscanf(value.c_str(),"%lf",&v);
				compares = (data.double_ == v);
			}
			break;
		}
	} catch ( ... ) {
		compares = false;
	}
	return compares;
}

bool MULTIMAP_API GenericType::operator!=(std::string value) {
	bool compares = false;
	try {
		switch (type) {
		case DT_CHAR:
			{
				char v = static_cast<char>(strtol(value.c_str(),NULL,10));
				compares = (data.char_ != v);
			}
			break;
		case DT_UCHAR:
			{
				unsigned char v = static_cast<unsigned char>(strtoul(value.c_str(),NULL,10));
				compares = (data.uchar_  != v);
			}
			break;
		case DT_SHRT:
			{
				short v = static_cast<short>(strtol(value.c_str(),NULL,10));
				compares = (data.short_ != v);
			}
			break;
		case DT_USHRT:
			{
				unsigned short v = static_cast<unsigned short>(strtoul(value.c_str(),NULL,10));
				compares = (data.ushort_  != v);
			}
			break;
		case DT_LONG:
			{
				long v = strtol(value.c_str(),NULL,10);
				compares = (data.long_ != v);
			}
			break;
		case DT_ULONG:
			{
				unsigned long v = strtoul(value.c_str(),NULL,10);
				compares = (data.ulong_ != v);
			}
			break;
		case DT_LLONG:
			{
				long long v = 0;// strtoll(value.c_str(),NULL,10);
				sscanf(value.c_str(),"%lld",&v);
				compares = (data.llong_ != v);
			}
			break;
		case DT_ULLONG:
			{
				unsigned long long v = 0; //strtoull(value.c_str(),NULL,10);
				sscanf(value.c_str(),"%llu",&v);
				compares = (data.ullong_ != v);
			}
			break;
		case DT_FLT:
			{
				float v = 0.0f; //strtof(value.c_str(),NULL,10);
				sscanf(value.c_str(),"%f",&v);
				compares = (data.float_ != v);
			}
			break;
		case DT_DBL:
			{
				double v = 0.0; //strtod(value.c_str(),NULL,10);
				sscanf(value.c_str(),"%lf",&v);
				compares = (data.double_ != v);
			}
			break;
		}
	} catch ( ... ) {
		compares = false;
	}
	return compares;
}

bool MULTIMAP_API GenericType::operator<(std::string value) {
	bool compares = false;
	try {
		switch (type) {
		case DT_CHAR:
			{
				char v = static_cast<char>(strtol(value.c_str(),NULL,10));
				compares = (data.char_ < v);
			}
			break;
		case DT_UCHAR:
			{
				unsigned char v = static_cast<unsigned char>(strtoul(value.c_str(),NULL,10));
				compares = (data.uchar_  < v);
			}
			break;
		case DT_SHRT:
			{
				short v = static_cast<short>(strtol(value.c_str(),NULL,10));
				compares = (data.short_ < v);
			}
			break;
		case DT_USHRT:
			{
				unsigned short v = static_cast<unsigned short>(strtoul(value.c_str(),NULL,10));
				compares = (data.ushort_  < v);
			}
			break;
		case DT_LONG:
			{
				long v = strtol(value.c_str(),NULL,10);
				compares = (data.long_ < v);
			}
			break;
		case DT_ULONG:
			{
				unsigned long v = strtoul(value.c_str(),NULL,10);
				compares = (data.ulong_ < v);
			}
			break;
		case DT_LLONG:
			{
				long long v = 0;// strtoll(value.c_str(),NULL,10);
				sscanf(value.c_str(),"%lld",&v);
				compares = (data.llong_ < v);
			}
			break;
		case DT_ULLONG:
			{
				unsigned long long v = 0; //strtoull(value.c_str(),NULL,10);
				sscanf(value.c_str(),"%llu",&v);
				compares = (data.ullong_ < v);
			}
			break;
		case DT_FLT:
			{
				float v = 0.0f; //strtof(value.c_str(),NULL,10);
				sscanf(value.c_str(),"%f",&v);
				compares = (data.float_ < v);
			}
			break;
		case DT_DBL:
			{
				double v = 0.0; //strtod(value.c_str(),NULL,10);
				sscanf(value.c_str(),"%lf",&v);
				compares = (data.double_ < v);
			}
			break;
		}
	} catch ( ... ) {
		compares = false;
	}
	return compares;
}
bool MULTIMAP_API GenericType::operator<=(std::string value) {
	bool compares = false;
	try {
		switch (type) {
		case DT_CHAR:
			{
				char v = static_cast<char>(strtol(value.c_str(),NULL,10));
				compares = (data.char_ <= v);
			}
			break;
		case DT_UCHAR:
			{
				unsigned char v = static_cast<unsigned char>(strtoul(value.c_str(),NULL,10));
				compares = (data.uchar_  <= v);
			}
			break;
		case DT_SHRT:
			{
				short v = static_cast<short>(strtol(value.c_str(),NULL,10));
				compares = (data.short_ <= v);
			}
			break;
		case DT_USHRT:
			{
				unsigned short v = static_cast<unsigned short>(strtoul(value.c_str(),NULL,10));
				compares = (data.ushort_  <= v);
			}
			break;
		case DT_LONG:
			{
				long v = strtol(value.c_str(),NULL,10);
				compares = (data.long_ <= v);
			}
			break;
		case DT_ULONG:
			{
				unsigned long v = strtoul(value.c_str(),NULL,10);
				compares = (data.ulong_ <= v);
			}
			break;
		case DT_LLONG:
			{
				long long v = 0;// strtoll(value.c_str(),NULL,10);
				sscanf(value.c_str(),"%lld",&v);
				compares = (data.llong_ <= v);
			}
			break;
		case DT_ULLONG:
			{
				unsigned long long v = 0; //strtoull(value.c_str(),NULL,10);
				sscanf(value.c_str(),"%llu",&v);
				compares = (data.ullong_ <= v);
			}
			break;
		case DT_FLT:
			{
				float v = 0.0f; //strtof(value.c_str(),NULL,10);
				sscanf(value.c_str(),"%f",&v);
				compares = (data.float_ <= v);
			}
			break;
		case DT_DBL:
			{
				double v = 0.0; //strtod(value.c_str(),NULL,10);
				sscanf(value.c_str(),"%lf",&v);
				compares = (data.double_ <= v);
			}
			break;
		}
	} catch ( ... ) {
		compares = false;
	}
	return compares;
}

bool MULTIMAP_API GenericType::operator>(std::string value) {
	bool compares = false;
	try {
		switch (type) {
		case DT_CHAR:
			{
				char v = static_cast<char>(strtol(value.c_str(),NULL,10));
				compares = (data.char_ > v);
			}
			break;
		case DT_UCHAR:
			{
				unsigned char v = static_cast<unsigned char>(strtoul(value.c_str(),NULL,10));
				compares = (data.uchar_  > v);
			}
			break;
		case DT_SHRT:
			{
				short v = static_cast<short>(strtol(value.c_str(),NULL,10));
				compares = (data.short_ > v);
			}
			break;
		case DT_USHRT:
			{
				unsigned short v = static_cast<unsigned short>(strtoul(value.c_str(),NULL,10));
				compares = (data.ushort_  > v);
			}
			break;
		case DT_LONG:
			{
				long v = strtol(value.c_str(),NULL,10);
				compares = (data.long_ > v);
			}
			break;
		case DT_ULONG:
			{
				unsigned long v = strtoul(value.c_str(),NULL,10);
				compares = (data.ulong_ > v);
			}
			break;
		case DT_LLONG:
			{
				long long v = 0;// strtoll(value.c_str(),NULL,10);
				sscanf(value.c_str(),"%lld",&v);
				compares = (data.llong_ > v);
			}
			break;
		case DT_ULLONG:
			{
				unsigned long long v = 0; //strtoull(value.c_str(),NULL,10);
				sscanf(value.c_str(),"%llu",&v);
				compares = (data.ullong_ > v);
			}
			break;
		case DT_FLT:
			{
				float v = 0.0f; //strtof(value.c_str(),NULL,10);
				sscanf(value.c_str(),"%f",&v);
				compares = (data.float_ > v);
			}
			break;
		case DT_DBL:
			{
				double v = 0.0; //strtod(value.c_str(),NULL,10);
				sscanf(value.c_str(),"%lf",&v);
				compares = (data.double_ > v);
			}
			break;
		}
	} catch ( ... ) {
		compares = false;
	}
	return compares;
}
bool MULTIMAP_API GenericType::operator>=(std::string value) {
	bool compares = false;
	try {
		switch (type) {
		case DT_CHAR:
			{
				char v = static_cast<char>(strtol(value.c_str(),NULL,10));
				compares = (data.char_ >= v);
			}
			break;
		case DT_UCHAR:
			{
				unsigned char v = static_cast<unsigned char>(strtoul(value.c_str(),NULL,10));
				compares = (data.uchar_  >= v);
			}
			break;
		case DT_SHRT:
			{
				short v = static_cast<short>(strtol(value.c_str(),NULL,10));
				compares = (data.short_ >= v);
			}
			break;
		case DT_USHRT:
			{
				unsigned short v = static_cast<unsigned short>(strtoul(value.c_str(),NULL,10));
				compares = (data.ushort_  >= v);
			}
			break;
		case DT_LONG:
			{
				long v = strtol(value.c_str(),NULL,10);
				compares = (data.long_ >= v);
			}
			break;
		case DT_ULONG:
			{
				unsigned long v = strtoul(value.c_str(),NULL,10);
				compares = (data.ulong_ >= v);
			}
			break;
		case DT_LLONG:
			{
				long long v = 0;// strtoll(value.c_str(),NULL,10);
				sscanf(value.c_str(),"%lld",&v);
				compares = (data.llong_ >= v);
			}
			break;
		case DT_ULLONG:
			{
				unsigned long long v = 0; //strtoull(value.c_str(),NULL,10);
				sscanf(value.c_str(),"%llu",&v);
				compares = (data.ullong_ >= v);
			}
			break;
		case DT_FLT:
			{
				float v = 0.0f; //strtof(value.c_str(),NULL,10);
				sscanf(value.c_str(),"%f",&v);
				compares = (data.float_ >= v);
			}
			break;
		case DT_DBL:
			{
				double v = 0.0; //strtod(value.c_str(),NULL,10);
				sscanf(value.c_str(),"%lf",&v);
				compares = (data.double_ >= v);
			}
			break;
		}
	} catch ( ... ) {
		compares = false;
	}
	return compares;
}

bool MULTIMAP_API GenericType::operator==(const char* value) {
	bool compares = false;
	try {
		switch (type) {
		case DT_CHAR:
			{
				char v = static_cast<char>(strtol(value,NULL,10));
				compares = (data.char_ == v);
			}
			break;
		case DT_UCHAR:
			{
				unsigned char v = static_cast<unsigned char>(strtoul(value,NULL,10));
				compares = (data.uchar_  == v);
			}
			break;
		case DT_SHRT:
			{
				short v = static_cast<short>(strtol(value,NULL,10));
				compares = (data.short_ == v);
			}
			break;
		case DT_USHRT:
			{
				unsigned short v = static_cast<unsigned short>(strtoul(value,NULL,10));
				compares = (data.ushort_  == v);
			}
			break;
		case DT_LONG:
			{
				long v = strtol(value,NULL,10);
				compares = (data.long_ == v);
			}
			break;
		case DT_ULONG:
			{
				unsigned long v = strtoul(value,NULL,10);
				compares = (data.ulong_ == v);
			}
			break;
		case DT_LLONG:
			{
				long long v = 0;// strtoll(value,NULL,10);
				sscanf(value,"%lld",&v);
				compares = (data.llong_ == v);
			}
			break;
		case DT_ULLONG:
			{
				unsigned long long v = 0; //strtoull(value,NULL,10);
				sscanf(value,"%llu",&v);
				compares = (data.ullong_ == v);
			}
			break;
		case DT_FLT:
			{
				float v = 0.0f; //strtof(value,NULL,10);
				sscanf(value,"%f",&v);
				compares = (data.float_ == v);
			}
			break;
		case DT_DBL:
			{
				double v = 0.0; //strtod(value.c_str(),NULL,10);
				sscanf(value,"%lf",&v);
				compares = (data.double_ == v);
			}
			break;
		}
	} catch ( ... ) {
		compares = false;
	}
	return compares;
}

bool MULTIMAP_API GenericType::operator!=(const char* value) {
	bool compares = false;
	try {
		switch (type) {
		case DT_CHAR:
			{
				char v = static_cast<char>(strtol(value,NULL,10));
				compares = (data.char_ != v);
			}
			break;
		case DT_UCHAR:
			{
				unsigned char v = static_cast<unsigned char>(strtoul(value,NULL,10));
				compares = (data.uchar_  != v);
			}
			break;
		case DT_SHRT:
			{
				short v = static_cast<short>(strtol(value,NULL,10));
				compares = (data.short_ != v);
			}
			break;
		case DT_USHRT:
			{
				unsigned short v = static_cast<unsigned short>(strtoul(value,NULL,10));
				compares = (data.ushort_  != v);
			}
			break;
		case DT_LONG:
			{
				long v = strtol(value,NULL,10);
				compares = (data.long_ != v);
			}
			break;
		case DT_ULONG:
			{
				unsigned long v = strtoul(value,NULL,10);
				compares = (data.ulong_ != v);
			}
			break;
		case DT_LLONG:
			{
				long long v = 0;// strtoll(value,NULL,10);
				sscanf(value,"%lld",&v);
				compares = (data.llong_ != v);
			}
			break;
		case DT_ULLONG:
			{
				unsigned long long v = 0; //strtoull(value,NULL,10);
				sscanf(value,"%llu",&v);
				compares = (data.ullong_ != v);
			}
			break;
		case DT_FLT:
			{
				float v = 0.0f; //strtof(value,NULL,10);
				sscanf(value,"%f",&v);
				compares = (data.float_ != v);
			}
			break;
		case DT_DBL:
			{
				double v = 0.0; //strtod(value.c_str(),NULL,10);
				sscanf(value,"%lf",&v);
				compares = (data.double_ != v);
			}
			break;
		}
	} catch ( ... ) {
		compares = false;
	}
	return compares;
}

bool MULTIMAP_API GenericType::operator<(const char* value) {
	bool compares = false;
	try {
		switch (type) {
		case DT_CHAR:
			{
				char v = static_cast<char>(strtol(value,NULL,10));
				compares = (data.char_ < v);
			}
			break;
		case DT_UCHAR:
			{
				unsigned char v = static_cast<unsigned char>(strtoul(value,NULL,10));
				compares = (data.uchar_  < v);
			}
			break;
		case DT_SHRT:
			{
				short v = static_cast<short>(strtol(value,NULL,10));
				compares = (data.short_ < v);
			}
			break;
		case DT_USHRT:
			{
				unsigned short v = static_cast<unsigned short>(strtoul(value,NULL,10));
				compares = (data.ushort_  < v);
			}
			break;
		case DT_LONG:
			{
				long v = strtol(value,NULL,10);
				compares = (data.long_ < v);
			}
			break;
		case DT_ULONG:
			{
				unsigned long v = strtoul(value,NULL,10);
				compares = (data.ulong_ < v);
			}
			break;
		case DT_LLONG:
			{
				long long v = 0;// strtoll(value,NULL,10);
				sscanf(value,"%lld",&v);
				compares = (data.llong_ < v);
			}
			break;
		case DT_ULLONG:
			{
				unsigned long long v = 0; //strtoull(value,NULL,10);
				sscanf(value,"%llu",&v);
				compares = (data.ullong_ < v);
			}
			break;
		case DT_FLT:
			{
				float v = 0.0f; //strtof(value,NULL,10);
				sscanf(value,"%f",&v);
				compares = (data.float_ < v);
			}
			break;
		case DT_DBL:
			{
				double v = 0.0; //strtod(value,NULL,10);
				sscanf(value,"%lf",&v);
				compares = (data.double_ < v);
			}
			break;
		}
	} catch ( ... ) {
		compares = false;
	}
	return compares;
}
bool MULTIMAP_API GenericType::operator<=(const char* value) {
	bool compares = false;
	try {
		switch (type) {
		case DT_CHAR:
			{
				char v = static_cast<char>(strtol(value,NULL,10));
				compares = (data.char_ <= v);
			}
			break;
		case DT_UCHAR:
			{
				unsigned char v = static_cast<unsigned char>(strtoul(value,NULL,10));
				compares = (data.uchar_  <= v);
			}
			break;
		case DT_SHRT:
			{
				short v = static_cast<short>(strtol(value,NULL,10));
				compares = (data.short_ <= v);
			}
			break;
		case DT_USHRT:
			{
				unsigned short v = static_cast<unsigned short>(strtoul(value,NULL,10));
				compares = (data.ushort_  <= v);
			}
			break;
		case DT_LONG:
			{
				long v = strtol(value,NULL,10);
				compares = (data.long_ <= v);
			}
			break;
		case DT_ULONG:
			{
				unsigned long v = strtoul(value,NULL,10);
				compares = (data.ulong_ <= v);
			}
			break;
		case DT_LLONG:
			{
				long long v = 0;// strtoll(value,NULL,10);
				sscanf(value,"%lld",&v);
				compares = (data.llong_ <= v);
			}
			break;
		case DT_ULLONG:
			{
				unsigned long long v = 0; //strtoull(value,NULL,10);
				sscanf(value,"%llu",&v);
				compares = (data.ullong_ <= v);
			}
			break;
		case DT_FLT:
			{
				float v = 0.0f; //strtof(value,NULL,10);
				sscanf(value,"%f",&v);
				compares = (data.float_ <= v);
			}
			break;
		case DT_DBL:
			{
				double v = 0.0; //strtod(value,NULL,10);
				sscanf(value,"%lf",&v);
				compares = (data.double_ <= v);
			}
			break;
		}
	} catch ( ... ) {
		compares = false;
	}
	return compares;
}

bool MULTIMAP_API GenericType::operator>(const char* value) {
	bool compares = false;
	try {
		switch (type) {
		case DT_CHAR:
			{
				char v = static_cast<char>(strtol(value,NULL,10));
				compares = (data.char_ > v);
			}
			break;
		case DT_UCHAR:
			{
				unsigned char v = static_cast<unsigned char>(strtoul(value,NULL,10));
				compares = (data.uchar_  > v);
			}
			break;
		case DT_SHRT:
			{
				short v = static_cast<short>(strtol(value,NULL,10));
				compares = (data.short_ > v);
			}
			break;
		case DT_USHRT:
			{
				unsigned short v = static_cast<unsigned short>(strtoul(value,NULL,10));
				compares = (data.ushort_  > v);
			}
			break;
		case DT_LONG:
			{
				long v = strtol(value,NULL,10);
				compares = (data.long_ > v);
			}
			break;
		case DT_ULONG:
			{
				unsigned long v = strtoul(value,NULL,10);
				compares = (data.ulong_ > v);
			}
			break;
		case DT_LLONG:
			{
				long long v = 0;// strtoll(value,NULL,10);
				sscanf(value,"%lld",&v);
				compares = (data.llong_ > v);
			}
			break;
		case DT_ULLONG:
			{
				unsigned long long v = 0; //strtoull(value,NULL,10);
				sscanf(value,"%llu",&v);
				compares = (data.ullong_ > v);
			}
			break;
		case DT_FLT:
			{
				float v = 0.0f; //strtof(value,NULL,10);
				sscanf(value,"%f",&v);
				compares = (data.float_ > v);
			}
			break;
		case DT_DBL:
			{
				double v = 0.0; //strtod(value,NULL,10);
				sscanf(value,"%lf",&v);
				compares = (data.double_ > v);
			}
			break;
		}
	} catch ( ... ) {
		compares = false;
	}
	return compares;
}
bool MULTIMAP_API GenericType::operator>=(const char* value) {
	bool compares = false;
	try {
		switch (type) {
		case DT_CHAR:
			{
				char v = static_cast<char>(strtol(value,NULL,10));
				compares = (data.char_ >= v);
			}
			break;
		case DT_UCHAR:
			{
				unsigned char v = static_cast<unsigned char>(strtoul(value,NULL,10));
				compares = (data.uchar_  >= v);
			}
			break;
		case DT_SHRT:
			{
				short v = static_cast<short>(strtol(value,NULL,10));
				compares = (data.short_ >= v);
			}
			break;
		case DT_USHRT:
			{
				unsigned short v = static_cast<unsigned short>(strtoul(value,NULL,10));
				compares = (data.ushort_  >= v);
			}
			break;
		case DT_LONG:
			{
				long v = strtol(value,NULL,10);
				compares = (data.long_ >= v);
			}
			break;
		case DT_ULONG:
			{
				unsigned long v = strtoul(value,NULL,10);
				compares = (data.ulong_ >= v);
			}
			break;
		case DT_LLONG:
			{
				long long v = 0;// strtoll(value,NULL,10);
				sscanf(value,"%lld",&v);
				compares = (data.llong_ >= v);
			}
			break;
		case DT_ULLONG:
			{
				unsigned long long v = 0; //strtoull(value,NULL,10);
				sscanf(value,"%llu",&v);
				compares = (data.ullong_ >= v);
			}
			break;
		case DT_FLT:
			{
				float v = 0.0f; //strtof(value,NULL,10);
				sscanf(value,"%f",&v);
				compares = (data.float_ >= v);
			}
			break;
		case DT_DBL:
			{
				double v = 0.0; //strtod(value,NULL,10);
				sscanf(value,"%lf",&v);
				compares = (data.double_ >= v);
			}
			break;
		}
	} catch ( ... ) {
		compares = false;
	}
	return compares;
}
std::string MULTIMAP_API GenericType::ToString() {
	char buffer[1032];
	try {
		switch (type) {
		case DT_CHAR:
			sprintf(buffer,"%d", data.char_);
			break;
		case DT_UCHAR:
			sprintf(buffer,"%u", data.uchar_);
			break;
		case DT_SHRT:
			sprintf(buffer,"%d", data.short_);
			break;
		case DT_USHRT:
			sprintf(buffer,"%u", data.ushort_);
			break;
		case DT_LONG:
			sprintf(buffer,"%ld", data.long_);
			break;
		case DT_ULONG:
			sprintf(buffer,"%lu", data.ulong_);
			break;
		case DT_LLONG:
			sprintf(buffer,"%lld", data.llong_);
			break;
		case DT_ULLONG:
			sprintf(buffer,"%llu", data.ullong_);
			break;
		case DT_FLT:
			sprintf(buffer,"%f", data.float_);
			break;
		case DT_DBL:
			sprintf(buffer,"%lf", data.double_);
			break;
		}
	} catch ( ... ) {
		buffer[0] = 0;
	}
	return std::string(buffer);
}

double MULTIMAP_API GenericType::ToDouble(void) {
	double trouble;
	try {
		switch (type) {
		case DT_CHAR:
			trouble = static_cast<double>(data.char_);
			break;
		case DT_UCHAR:
			trouble = static_cast<double>(data.uchar_);
			break;
		case DT_SHRT:
			trouble = static_cast<double>(data.short_);
			break;
		case DT_USHRT:
			trouble = static_cast<double>(data.ushort_);
			break;
		case DT_LONG:
			trouble = static_cast<double>(data.long_);
			break;
		case DT_ULONG:
			trouble = static_cast<double>(data.ulong_);
			break;
		case DT_LLONG:
			trouble = static_cast<double>(data.llong_);
			break;
		case DT_ULLONG:
			trouble = static_cast<double>(data.ullong_);
			break;
		case DT_FLT:
			trouble = static_cast<double>(data.float_);
			break;
		case DT_DBL:
			trouble = static_cast<double>(data.double_);
			break;
		}
	} catch ( ... ) {
		trouble = -DBL_MAX;
	}
	return trouble;
}
std::string MULTIMAP_API GenericType::ToString(std::string key) {
	char buffer[1032];
	try {
		switch (type) {
		case DT_CHAR:
			sprintf(buffer,"%s = %d",key.c_str(), data.char_);
			break;
		case DT_UCHAR:
			sprintf(buffer,"%s = %u",key.c_str(), data.uchar_);
			break;
		case DT_SHRT:
			sprintf(buffer,"%s = %d",key.c_str(), data.short_);
			break;
		case DT_USHRT:
			sprintf(buffer,"%s = %u",key.c_str(), data.ushort_);
			break;
		case DT_LONG:
			sprintf(buffer,"%s = %ld",key.c_str(), data.long_);
			break;
		case DT_ULONG:
			sprintf(buffer,"%s = %lu",key.c_str(), data.ulong_);
			break;
		case DT_LLONG:
			sprintf(buffer,"%s = %lld",key.c_str(), data.llong_);
			break;
		case DT_ULLONG:
			sprintf(buffer,"%s = %llu",key.c_str(), data.ullong_);
			break;
		case DT_FLT:
			sprintf(buffer,"%s = %f",key.c_str(), data.float_);
			break;
		case DT_DBL:
			sprintf(buffer,"%s = %lf",key.c_str(), data.double_);
			break;
		}
	} catch ( ... ) {
		buffer[0] = 0;
	}
	return std::string(buffer);
}