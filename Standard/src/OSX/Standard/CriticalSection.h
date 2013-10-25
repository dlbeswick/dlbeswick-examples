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

#ifdef __OBJC__
	@class NSRecursiveLock;
#else
	class NSRecursiveLock;
#endif

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
	NSRecursiveLock* m_obj;
	bool m_success;
};

#endif