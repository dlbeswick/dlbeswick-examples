#ifndef STANDARD_TEXTSTRINGSTREAM_H
#define STANDARD_TEXTSTRINGSTREAM_H

#include "Standard/api.h"
#include <sstream>
#include "textstream.h"
#include "streamops.h"

class STANDARD_API itextstringstream : public itextstream
{
public:
	itextstringstream(const std::string& input) :
		itextstream(new std::stringstream(input))
	{
	}

	std::stringstream& s() { return (std::stringstream&)itextstream::s(); }
	const std::stringstream& s() const { return (const std::stringstream&)itextstream::s(); }
	std::string str() const { return s().str(); }
};

class STANDARD_API otextstringstream : public otextstream
{
public:
	otextstringstream() :
		otextstream(new std::stringstream)
	{
	}

	std::stringstream& s() { return (std::stringstream&)otextstream::s(); }
	const std::stringstream& s() const { return (const std::stringstream&)otextstream::s(); }
	std::string str() const { return s().str(); }
};

#endif
