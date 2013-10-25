#ifndef _STANDARD_EXCEPTIONWINAPI_H
#define _STANDARD_EXCEPTIONWINAPI_H

#include "Standard/api.h"
#include "Standard/Exception.h"

class STANDARD_API ExceptionWinAPI : public Exception
{
public:
	ExceptionWinAPI(const std::string& context = "");
	~ExceptionWinAPI() throw() {}

	virtual const char* what() const throw();

protected:
	char m_buffer[1024];
	std::string m_result;
};

#endif
