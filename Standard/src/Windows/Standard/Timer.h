// ---------------------------------------------------------------------------------------------------------
//
// Timer
//
// ---------------------------------------------------------------------------------------------------------
#pragma once

#include "Standard/api.h"
#include "mmsystem.h"
#include "Standard/Help.h"

class Timer
{
public:
	Timer(double frameClamp = 0.2)
	{
		QueryPerformanceFrequency((LARGE_INTEGER*)&m_freq64);
		m_frame = 0.0;
		m_scale = 1.0;
		m_lastTime = 0.0;
		m_time = 0.0;
		setFrameClamp(frameClamp);

		restart();
	}

	void setFrameClamp(double d)
	{
		m_frameClamp = d;
	}

	void restart()
	{
		QueryPerformanceCounter((LARGE_INTEGER*)&m_startValue64);
		QueryPerformanceCounter((LARGE_INTEGER*)&m_lastValue64);
		m_time = 0.0;
		m_scale = 1.0;
		m_frame = 0.0;
	}

	void startFrame()
	{
		int64 val64;
		QueryPerformanceCounter((LARGE_INTEGER*)&val64);

		m_lastValue64 = val64;
		m_lastTime = m_time;
	}

	void endFrame()
	{
		int64 val64;
		int64 frame64;

		QueryPerformanceCounter((LARGE_INTEGER*)&val64);

		frame64 = val64 - m_lastValue64;
		double frame = ((double)frame64 / m_freq64) * m_scale;
		m_time += std::min(frame, m_frameClamp);
		m_frame = m_time - m_lastTime;

		// tbd: maybe get rid of this. before startFrame existed, a single call to endFrame calculated delta correctly
		m_lastValue64 = val64;
		m_lastTime = m_time;
	}

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
	double realTime() const
	{
		int64 val64;

		QueryPerformanceCounter((LARGE_INTEGER*)&val64);
		return (double)(val64 - m_startValue64) / m_freq64;
	}

	// time at last endFrame
	double time() const
	{
		return m_time;
	}

	int64 time64()
	{
		int64 val64;

		QueryPerformanceCounter((LARGE_INTEGER*)&val64);
		return val64;
	}

	int64 freq64()
	{
		return m_freq64;
	}

	double frame() const { return std::min(m_frame, m_frameClamp); }
	double realFrame() const { return m_frame; }
	double scale() const { return m_scale; }

	operator double () const { return time(); }
	operator float () const { return (float)time(); }

private:
	int64	m_freq64;
	int64	m_lastValue64;
	int64	m_startValue64;
	double	m_frame;
	double	m_time;
	double	m_scale;
	double	m_lastTime;
	double	m_frameClamp;
};
