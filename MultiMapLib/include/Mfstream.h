#pragma once
#include "MultiMap.h"
#include <fstream>
#include "MappedFile.h"

class MappedFile;

class Mfstream : public std::fstream {
public:
	MULTIMAP_API ~Mfstream(void);
	MULTIMAP_API Mfstream();

	MULTIMAP_API void close();
	MULTIMAP_API bool is_open() const;
	MULTIMAP_API void open(const char* filename, std::ios_base::openmode mode = std::ios_base::in);
	MULTIMAP_API void open(const std::string& filename, std::ios_base::openmode mode = std::ios_base::in);

	MULTIMAP_API std::streamsize gcount() const;
	MULTIMAP_API int peek();
	MULTIMAP_API Mfstream& read (char_type* s, std::streamsize n);
	MULTIMAP_API Mfstream* seekg(std::streampos pos);
	MULTIMAP_API Mfstream* seekg(std::streamoff off, std::ios_base::seekdir way);
	MULTIMAP_API Mfstream* flush();
	MULTIMAP_API Mfstream* seekp(std::streampos pos);
	MULTIMAP_API Mfstream* seekp(std::streamoff off, std::ios_base::seekdir way);
	MULTIMAP_API std::streampos tellp();
	MULTIMAP_API Mfstream& write(const char* s, std::streamsize n);
	MULTIMAP_API int sync();
	MULTIMAP_API std::istream::streampos tellg();
	MULTIMAP_API Mfstream& unget();


	void _Add_vtordisp1(); // required to meet virtual requirements of fstream
	void _Add_vtordisp2(); // required to meet virtual requirements of fstream
protected:
	MappedFile* mf;
	char* currentPosition;
	size_t last_read_count;
	std::ios::iostate iostate;
};