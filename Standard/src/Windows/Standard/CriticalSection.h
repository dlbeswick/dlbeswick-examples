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
#pragma once

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
	CRITICAL_SECTION m_obj;
	bool m_success;
};
