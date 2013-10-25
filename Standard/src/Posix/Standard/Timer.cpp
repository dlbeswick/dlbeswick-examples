// ---------------------------------------------------------------------------------------------------------
//
// Timer
//
// ---------------------------------------------------------------------------------------------------------
#include "Standard/Timer.h"

Timer::Timer(double frameClamp)
{
	m_frame = 0.0;
	m_scale = 1.0;
	m_lastTime = 0.0;
	m_time = 0.0;
	setFrameClamp(frameClamp);

	restart();
}

void Timer::setFrameClamp(double d)
{
	m_frameClamp = d;
}

void Timer::restart()
{
	m_startTime = realTime();
	m_lastClockTime = realTime();
	m_time = 0.0;
	m_scale = 1.0;
	m_frame = 0.0;
}

void Timer::startFrame()
{
	m_lastClockTime = realTime();
	m_lastTime = m_time;
}

void Timer::endFrame()
{
	double currentTime = realTime();

	double frameTime = currentTime - m_lastClockTime;
	double frame = frameTime * m_scale;
	m_time += std::min(frame, m_frameClamp);
	m_frame = m_time - m_lastTime;

	// tbd: maybe get rid of this. before startFrame existed, a single call to endFrame calculated delta correctly
	m_lastClockTime = currentTime;
	m_lastTime = m_time;
}

void Timer::setFromRealTime()
{
	m_time = realTime();
}

bool Timer::elapsed(float interval) const
{
	return m_time - m_lastTime >= interval;
}

void Timer::setScale(double d)
{
	m_scale = d;
}

void Timer::advance(double d)
{
	m_time += d;
}

double Timer::realTime() const
{
	timespec tp;

	if (clock_gettime(CLOCK_MONOTONIC, &tp) != 0)
		EXCEPTIONSTREAM(ExceptionTimer(), "clock_gettime");

	return tp.tv_sec + tp.tv_nsec / 1.0e9;
}

double Timer::time() const
{
	return m_time;
}

double Timer::frame() const
{
	return std::min(m_frame, m_frameClamp);
}

double Timer::realFrame() const
{
	return m_frame;
}

double Timer::scale() const
{
	return m_scale;
}

Timer::operator double () const
{
	return time();
}

Timer::operator float () const
{
	return (float)time();
}
