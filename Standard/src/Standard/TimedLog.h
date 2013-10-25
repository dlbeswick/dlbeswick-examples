#ifndef STANDARD_TIMEDLOG_H
#define STANDARD_TIMEDLOG_H

#include "Standard/api.h"
#include "Standard/Timer.h"
#include "Log.h"

class TimedLog : public dlogstreamstdout
{
public:
	TimedLog(float interval = 0.5f) :
		m_interval(interval)
	{
		m_timer.endFrame();
	}

protected:
	virtual bool shouldPrint()
	{
		m_timer.setFromRealTime();
		return m_timer.elapsed(m_interval);
	}

	float m_interval;
	mutable Timer m_timer;
};

class TimedLogErr : public dlogstreamstdout
{
public:
	TimedLogErr(float interval = 0.5f) :
		m_interval(interval)
	{
		m_timer.endFrame();
	}

protected:
	virtual bool shouldPrint()
	{
		m_timer.setFromRealTime();
		return m_timer.elapsed(m_interval);
	}

	float m_interval;
	mutable Timer m_timer;
};
#endif
