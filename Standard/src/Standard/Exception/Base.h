#ifndef STANDARD_EXCEPTION_BASE_H
#define STANDARD_EXCEPTION_BASE_H

#include "Standard/api.h"
#include "Standard/ExceptionStream.h"

class STANDARD_API ExceptionBaseNoRegistrant : public ExceptionStream
{
public:
	ExceptionBaseNoRegistrant(const std::string& className)
	{
		 exceptionstream(*this) << "No such registrant.";
		 addContext(className);
	}
};

#endif
