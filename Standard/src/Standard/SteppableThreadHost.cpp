// ------------------------------------------------------------------------------------------------
//
// SteppableThreadHost
//
// ------------------------------------------------------------------------------------------------
#include "Standard/pch.h"
#if IS_WINDOWS
#include "SteppableThreadHost.h"
#include "Standard/ThreadStepper.h"
#include "Help.h"
#include "STLHelp.h"

// SteppableThreadHost
SteppableThreadHost::~SteppableThreadHost()
{
}

void SteppableThreadHost::add(ThreadStepper& stepper)
{
	m_steppers.push_back(&stepper);
}

void SteppableThreadHost::update()
{
	for (uint i = 0; i < m_steppers.size(); ++i)
	{
		m_steppers[i]->update();
	}
}

#endif
