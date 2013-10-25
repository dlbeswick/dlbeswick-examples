// ---------------------------------------------------------------------------------------------------------
//
// Timer
//
// ---------------------------------------------------------------------------------------------------------
#ifndef _STANDARD_TIMER_H
#define _STANDARD_TIMER_H

#include "Standard/api.h"
#include "Standard/ExceptionPosix.h"

class ExceptionTimer : public ExceptionPosix
{
public:
	ExceptionTimer()
	{
		addContext("Timer");
	}
};

class Timer
{
public:
	Timer(double frameClamp = 0.2);

	void setFrameClamp(double d);
	void restart();
	void startFrame();
	void endFrame();
	void setFromRealTime();
	bool elapsed(float interval) const;
	void setScale(double d);
	void advance(double d);

	// actual time from system clock
	double realTime() const;

	// time at last endFrame
	double time() const;

	double frame() const;
	double realFrame() const;
	double scale() const;

	operator double () const;
	operator float () const;

private:
	double	m_lastClockTime;
	double	m_startTime;
	double	m_frame;
	double	m_time;
	double	m_scale;
	double	m_lastTime;
	double	m_frameClamp;
};

#endif
