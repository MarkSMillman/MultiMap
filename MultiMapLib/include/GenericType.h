#pragma once
#include "MultiMap.h"
#include <stdint.h>
#include <string>
#include <vector>

typedef enum {DT_UNDEF=0,DT_UCHAR=1,DT_CHAR=2,DT_USHRT=3,DT_SHRT=4,DT_ULONG=5,DT_LONG=6,DT_ULLONG=7,DT_LLONG=8,DT_FLT=9,DT_DBL=10,DT_MAX} DTYPE;
typedef union generictype {
    int8_t               char_;
	uint8_t             uchar_;
	int16_t             short_;
	uint16_t           ushort_;
	long                 long_;
	unsigned long       ulong_;
	long long           llong_;
	unsigned long long ullong_;
	float               float_;
	double             double_;
} generictype;

class GenericType {
public:
	MULTIMAP_API GenericType(void);
	MULTIMAP_API GenericType(bool _bool);
	MULTIMAP_API GenericType(char _char);
	MULTIMAP_API GenericType(unsigned char _uchar);
	MULTIMAP_API GenericType(int _int);
	MULTIMAP_API GenericType(unsigned int _uint);
	MULTIMAP_API GenericType(short _short);
	MULTIMAP_API GenericType(unsigned short _ushort);
	MULTIMAP_API GenericType(long _long);
	MULTIMAP_API GenericType(unsigned long _ulong);
	MULTIMAP_API GenericType(long long _llong);
	MULTIMAP_API GenericType(unsigned long long _ullong);
	MULTIMAP_API GenericType(float _float);
	MULTIMAP_API GenericType(double _double);
	MULTIMAP_API GenericType(const GenericType& other);
	MULTIMAP_API GenericType(DTYPE type, void* data);

	MULTIMAP_API GenericType& operator=(const GenericType& other);
	friend std::ostream& operator<< (std::ostream& stream, const GenericType& generictype);

	void MULTIMAP_API Set(char _char);
	void MULTIMAP_API Set(unsigned char _uchar);
	void MULTIMAP_API Set(short _short);
	void MULTIMAP_API Set(unsigned short _ushort);
	void MULTIMAP_API Set(long _long);
	void MULTIMAP_API Set(int _int);
	void MULTIMAP_API Set(unsigned int _uint);
	void MULTIMAP_API Set(unsigned long _ulong);
	void MULTIMAP_API Set(long long _llong);
	void MULTIMAP_API Set(unsigned long long _ullong);
	void MULTIMAP_API Set(float _float);
	void MULTIMAP_API Set(double _double);

	bool MULTIMAP_API Signed(void);
	bool MULTIMAP_API Integer(void);
	bool MULTIMAP_API FloatingPoint(void);
	GenericType MULTIMAP_API Elevate(bool forceSigned = false);
	unsigned long long MULTIMAP_API Maximum(void);
	unsigned long long MULTIMAP_API Maximum(DTYPE _type);

	bool MULTIMAP_API operator==(const GenericType & other);
	bool MULTIMAP_API operator!=(const GenericType & other);
	bool MULTIMAP_API operator<(const GenericType & other);
	bool MULTIMAP_API operator<=(const GenericType & other);
	bool MULTIMAP_API operator>(const GenericType & other);
	bool MULTIMAP_API operator>=(const GenericType & other);

	bool MULTIMAP_API operator==(std::string value);
	bool MULTIMAP_API operator!=(std::string value);
	bool MULTIMAP_API operator<(std::string value);
	bool MULTIMAP_API operator<=(std::string value);
	bool MULTIMAP_API operator>(std::string value);
	bool MULTIMAP_API operator>=(std::string value);

	bool MULTIMAP_API operator==(const char* value);
	bool MULTIMAP_API operator!=(const char* value);
	bool MULTIMAP_API operator<(const char* value);
	bool MULTIMAP_API operator<=(const char* value);
	bool MULTIMAP_API operator>(const char* value);
	bool MULTIMAP_API operator>=(const char* value);

	double MULTIMAP_API ToDouble(void);
	std::string MULTIMAP_API ToString(void);
	std::string MULTIMAP_API ToString(std::string key);

	DTYPE type;
	generictype data;

private:

};

class NumberedValue {
public:
	size_t         key;
	std::string  value;
};
//class ExtraType {
//public:
//	double            scale;
//	double           offset;
//	GenericType         min;
//	GenericType         max;
//	GenericType      nodata;
//	char          no_data:1;
//	char          has_min:1;
//	char          has_max:1;
//	char        has_scale:1;
//	char       has_offset:1;
//	char         reserved:3;
//	std::string        name;
//	std::string description;
//};
//
//class NumberedValue {
//public:
//	size_t     number;
//	std::string value;
//};