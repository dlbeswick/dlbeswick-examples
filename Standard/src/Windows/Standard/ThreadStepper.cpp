// ------------------------------------------------------------------------------------------------
//
// ThreadStepper
//
// ------------------------------------------------------------------------------------------------
#include "Standard/pch.h"
#include "ThreadStepper.h"
#include "Standard/SteppableThreadHost.h"
#include "Standard/STLHelp.h"
#include "Standard/Log.h"

// ThreadStepper
bool ThreadStepper::m_convertedToFiber = false;

ThreadStepper::ThreadStepper(Delegate func) :
	m_active(true),
	m_stepCount(0),
	m_current(""),
	m_func(func),
	m_stepToStart(false)
{
	if (!m_convertedToFiber)
	{
		m_parentFiber = ConvertThreadToFiber(0);
		m_convertedToFiber = true;
	}

	m_fiber = CreateFiber(0, ThreadStepper::staticUpdate, (PVOID)this);
	if (!m_fiber)
		throwf("Couldn't create fiber.");
}

void CALLBACK ThreadStepper::staticUpdate(PVOID param)
{
	ThreadStepper* target = (ThreadStepper*)param;
	do
	{
		target->m_func();
		SwitchToFiber(target->m_parentFiber);
	}
	while (true);
}

ThreadStepper::MarkerList ThreadStepper::markers() const
{
	return sortedKeys(m_markers);
}

void ThreadStepper::mark(const char* marker)
{
	if (!m_active/* || m_markers[marker]*/)
	{
		m_current = marker;
		SwitchToFiber(m_parentFiber);
	}
}

void ThreadStepper::suspendAt(const char* marker, bool enabled)
{
	m_markers[marker] = enabled;
}

void ThreadStepper::pause()
{
	m_stepCount = 1;
	m_active = false;
}

void ThreadStepper::step()
{
	++m_stepCount;
}

void ThreadStepper::resume()
{
	m_active = true;
}

void ThreadStepper::stepToStart()
{
	resume();
	m_stepToStart = true;
}

void ThreadStepper::update()
{
	if (m_active)
	{
		SwitchToFiber(m_fiber);
		if (m_stepToStart)
		{
			m_active = false;
			m_stepToStart = false;
		}
	}
	else
	{
		for (; m_stepCount > 0; --m_stepCount)
			SwitchToFiber(m_fiber);
	}
}

const char* ThreadStepper::current() const
{
	if (m_active)
		return 0;
	else
		return m_current;
}
