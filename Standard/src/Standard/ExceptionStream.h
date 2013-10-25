#ifndef STANDARD_EXCEPTIONSTREAM_H
#define STANDARD_EXCEPTIONSTREAM_H

#include "Standard/api.h"
#include "Standard/Exception.h"
#include "Standard/textstringstream.h"

class STANDARD_API ExceptionStream : public Exception
{
public:
	template <class T>
	struct exception_stream
	{
		exception_stream(T& _who) : who(_who) {}
		T& who;

		template <class U>
		exception_stream& operator << (const U& rhs) 
		{
			who._stream << rhs; 
			return *this; 
		}
	};

	virtual const char* what() const throw();

	template <class T>
	void addContext(const T& info)
	{
		if ((int)_context.s().tellp() > 0)
			_context << " / ";

		_context << info;
	}

protected:
	ExceptionStream();
	virtual ~ExceptionStream() throw() {}

	otextstringstream _stream;
	otextstringstream _context;
	mutable std::string _string;
};

template <class T>
ExceptionStream::exception_stream<T> exceptionstream(T exceptionStream)
{ 
	return ExceptionStream::exception_stream<T>(exceptionStream);
}

// USAGE: EXCEPTIONSTREAM(class([constructor_arg0, constructor_arg1, ...]), [streamarg0 << streamarg1]);
// Don't forget to specify the constructor braces if the constructor has no args.
#define EXCEPTIONSTREAM(EXCEPTIONSTREAMCLASS, STREAMOPS) \
	throw((exceptionstream(EXCEPTIONSTREAMCLASS) << STREAMOPS).who);

#endif
