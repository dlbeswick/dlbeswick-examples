// ------------------------------------------------------------------------------------------------
//
// Exception
//
// ------------------------------------------------------------------------------------------------
#ifndef STANDARD_EXCEPTION_H
#define STANDARD_EXCEPTION_H

#include "Standard/api.h"
#include <cstdarg>
#include <cstdio>
#include <string>
#include <exception>

class STANDARD_API Exception : public std::exception
{
public:
	virtual ~Exception() throw() {}

	operator std::string () const { return what(); }
	virtual const char* what() const throw();
};

class STANDARD_API ExceptionString : public Exception
{
public:
	~ExceptionString() throw() {};

	ExceptionString(const std::string& s);

	virtual const char* what() const throw();

	std::string message;
};

template <class T>
inline void throwft(const std::string& s, ...)
{
	char buf[2048];
	va_list v;
	va_start(v, s);

#if _MSC_VER
	_vsnprintf(buf, 2048, s.c_str(), v);
#else
	vsnprintf(buf, 2048, s.c_str(), v);
#endif

	T e(buf);

	throw(e);
}

#define throwf throwft<ExceptionString>

#endif
