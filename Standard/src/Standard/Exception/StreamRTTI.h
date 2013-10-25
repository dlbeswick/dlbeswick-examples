#ifndef STANDARD_STREAMRTTI_H
#define STANDARD_STREAMRTTI_H

#include "Standard/ExceptionStream.h"

class STANDARD_API ExceptionStreamRTTI : public ExceptionStream
{
public:
	ExceptionStreamRTTI(const class Base& subject);
};

#define EXCEPTIONRTTI(api, className) \
	class api Exception##className : public ExceptionStreamRTTI \
	{ \
	public: \
		Exception##className(const class Base& subject) \
			: ExceptionStreamRTTI(subject) \
		{} \
	};

#endif
