#ifndef _STANDARD_EXCEPTIONOS_H
#define _STANDARD_EXCEPTIONOS_H

#include "Standard/Exception.h"

class STANDARD_API ExceptionOS : public Exception
{
public:
	ExceptionOS(const std::string& line, OSStatus error);
	~ExceptionOS() throw();
	
	virtual const char* what() const throw();

protected:
	std::string _message;
};

#define osVerify(x) { OSStatus result = x; if (result != 0) throw(ExceptionOS(#x, result)); }

#endif