// ------------------------------------------------------------------------------------------------
//
// SteppableThreadHost
//
// ------------------------------------------------------------------------------------------------
#pragma once

#include "Standard/api.h"

class STANDARD_API SteppableThreadHost
{
public:
	virtual ~SteppableThreadHost();

	void add(class ThreadStepper& stepper);
	void update();

private:
	friend class ThreadStepper;
	std::vector<class ThreadStepper*> m_steppers;
};

