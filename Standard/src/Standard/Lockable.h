#ifndef STANDARD_LOCKABLE_H
#define STANDARD_LOCKABLE_H

#include "Standard/api.h"

class Lockable
{
public:
	operator CriticalSection& () { return m_lock; }
	operator CriticalSection& () const { return const_cast<Lockable*>(this)->m_lock; }

private:
	CriticalSection m_lock;
};

#endif
