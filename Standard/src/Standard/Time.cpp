#include "Standard/pch.h"
#include "Time.h"
#if !IS_LINUX // tbd
#include "Standard/Timer.h"

Time::Time() :
	_ms(0)
{
}

Time::Time(double ms) :
	_ms(ms)
{
}

bool Time::elapsed(const Timer& current, double testSeconds) const
{
	return current.time() - _ms > testSeconds;
}

bool Time::elapsed(const Time& current, double testSeconds) const
{
	return current._ms - _ms > testSeconds;
}

Time& Time::operator=(double ms)
{
	_ms = ms;

	return *this;
}
#endif
