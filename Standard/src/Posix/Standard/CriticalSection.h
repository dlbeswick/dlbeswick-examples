// ------------------------------------------------------------------------------------------------
//
// CriticalSection
// Usage:
// define a CriticalSection object in class.
// call CRITICAL(<object>)
//	{
//	}
//
// ------------------------------------------------------------------------------------------------
#ifndef _STANDARD_CRITICALSECTION_H
#define _STANDARD_CRITICALSECTION_H

#include "Standard/api.h"

class STANDARD_API CriticalSection
{
public:
	CriticalSection();

	void enter();

	bool success() const;

	bool tryEnter();

	void leave();

	~CriticalSection();

protected:
	pthread_mutex_t _mutex;
	pthread_mutexattr_t _attr;
	bool m_success;
};

#endif
