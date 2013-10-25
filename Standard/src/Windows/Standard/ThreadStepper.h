// ------------------------------------------------------------------------------------------------
//
// ThreadStepper
//
// ------------------------------------------------------------------------------------------------
#pragma once

#if _WIN32_WINNT >= 0x0400

#include "Standard/api.h"
#include <Standard/Delegate.h>

class SteppableThreadHost;

class STANDARD_API ThreadStepper
{
public:
	typedef TDelegate<SteppableThreadHost> Delegate;
	ThreadStepper(Delegate func);

	typedef const char* MarkerType;
	typedef std::vector<MarkerType> MarkerList;

	void mark(const char* marker);
	void suspendAt(const char* marker, bool enabled);
	void pause();
	void step();
	void resume();
	void stepToStart();
	void update();

	MarkerList markers() const;
	const char* current() const;

protected:
	friend class SteppableThreadHost;
	typedef stdext::hash_map<MarkerType, bool> Markers;

	static void CALLBACK staticUpdate(LPVOID param);

	Markers m_markers;
	Delegate m_func;
	LPVOID m_fiber;
	LPVOID m_parentFiber;
	bool m_active;
	int m_stepCount;
	static bool m_convertedToFiber;
	const char* m_current;
	bool m_stepToStart;
};

#define STEPMARK(stepper, x) stepper.mark(x)

#endif
