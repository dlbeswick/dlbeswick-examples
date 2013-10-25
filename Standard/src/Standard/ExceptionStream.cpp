#include "Standard/pch.h"
#include "ExceptionStream.h"
#include <sstream>

ExceptionStream::ExceptionStream()
{
}

const char* ExceptionStream::what() const throw()
{
	_string = _stream.str();

	std::string contextStr = _context.str();

	if (!contextStr.empty())
		_string += " (" + contextStr + ")";

	return _string.c_str();
}
