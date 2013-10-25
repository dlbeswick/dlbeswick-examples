// ------------------------------------------------------------------------------------------------
//
// binstream
//
// ------------------------------------------------------------------------------------------------
#include "Standard/pch.h"
#include "binstream.h"
#include "Help.h"

void ibinstream::toBuf(char*& buf, size_t& length)
{
	// TBD: memory mapping (CreateFile, CreateFileMapping, MapViewOfFile, UnMapViewOfFile)

	// remember get pos
	size_t oldPos = (size_t)s().tellg();

	// find file length
	s().seekg(0, std::ios::end);
	length = (size_t)s().tellg();

	// allocate buffer and read file
	s().seekg(0);
	buf = new char[length];
	s().read(buf, length);

	// restore get pos
	s().seekg(oldPos);
}
