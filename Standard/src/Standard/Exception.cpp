#include "Standard/pch.h"
#include "Exception.h"

const char* Exception::what() const throw()
{ 
	return "Exception: Unknown exception"; 
}

ExceptionString::ExceptionString(const std::string& s) :
	message(s)
{
}

const char* ExceptionString::what() const throw()
{ 
	return message.c_str(); 
}

