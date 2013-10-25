#ifndef STANDARD_THREAD_SAFECOPYACCESSOR_H
#define STANDARD_THREAD_SAFECOPYACCESSOR_H

#include "Standard/api.h"
#include "Standard/CriticalSection.h"

namespace StandardThread
{

template <class T>
class SafeCopyAccessor
{
public:
	SafeCopyAccessor(T value = T()) :
		_value(value)
	{
	}

	operator T() const
	{
		return get();
	}

	SafeCopyAccessor& operator = (const T& value)
	{
		set(value);
		return *this;
	}

	T get() const
	{
		Critical(_critical);
		return _value;
	}

	void set(const T& value)
	{
		Critical(_critical);
		_value = value;
	}

protected:
	CriticalSection _critical;
	T _value;
};

}

#endif
