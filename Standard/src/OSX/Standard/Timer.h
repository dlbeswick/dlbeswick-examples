// ---------------------------------------------------------------------------------------------------------
// 
// Timer
// 
// ---------------------------------------------------------------------------------------------------------
#pragma once


#include "Standard/Help.h"
#include <CoreServices/CoreServices.h>


class Timer
{
public:
	Timer(double frameClamp = 0.2);

	void setFrameClamp(double d)
	{
		m_frameClamp = d;
	}

	void restart();

	void startFrame();
	
	void endFrame();

	void setFromRealTime()
	{
		m_time = realTime();
	}

	bool elapsed(float interval) const
	{
		return m_time - m_lastTime >= interval;
	}

	void setScale(double d)
	{
		m_scale = d;
	}

	void advance(double d)
	{
		m_time += d;
	}

	// actual time from system clock
	double realTime() const;

	// time at last endFrame
	double time() const
	{
		return m_time;
	}

	double frame() const { return std::min(m_frame, m_frameClamp); }
	double realFrame() const { return m_frame; }
	double scale() const { return m_scale; }

	operator double () const { return time(); }
	operator float () const { return (float)time(); }

private:
	AbsoluteTime systemTime() const;

	AbsoluteTime	m_lastValue64;
	AbsoluteTime	m_startValue64;
	double	m_frame;
	double	m_time;
	double	m_scale;
	double	m_lastTime;
	double	m_frameClamp;
};