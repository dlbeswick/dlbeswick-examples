#ifndef STANDARD_BINBUFSTREAM_H
#define STANDARD_BINBUFSTREAM_H

#include "Standard/api.h"

#if IS_GNUC
#undef __DEPRECATED
#endif

#include <strstream>

#if IS_GNUC
#define __DEPRECATED
#endif

#include "binstream.h"

class STANDARD_API ibinbufstream : public ibinstream
{
public:
	ibinbufstream(const char* input) :
		ibinstream(new std::istrstream(input))
	{
	}

	std::strstream& s() { return (std::strstream&)ibinstream::s(); }
	const std::strstream& s() const { return (const std::strstream&)ibinstream::s(); }
};

#endif
