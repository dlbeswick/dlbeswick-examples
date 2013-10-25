#ifndef STANDARD_TIME_H
#define STANDARD_TIME_H

#include "Standard/api.h"
class Timer;

class STANDARD_API Time
{
public:
	Time();
	Time(double ms);

	Time& operator=(double ms);

	bool elapsed(const Timer& current, double testSeconds) const;
	bool elapsed(const Time& current, double testSeconds) const;

protected:
	double _ms;
};

#endif
