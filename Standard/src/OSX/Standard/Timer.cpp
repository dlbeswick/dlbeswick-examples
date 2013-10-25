#include "Standard/pch.h"
#include "Timer.h"

Timer::Timer(double frameClamp)
{
	m_frame = 0.0;
	m_scale = 1.0;
	m_lastTime = 0.0;
	m_time = 0.0;
	setFrameClamp(frameClamp);

	restart();
}

void Timer::restart()
{
	m_startValue64 = UpTime();
	m_lastValue64 = m_startValue64;
	m_time = 0.0;
	m_scale = 1.0;
	m_frame = 0.0;
}

void Timer::startFrame()
{
	AbsoluteTime val64;
	val64 = UpTime();

	m_lastValue64 = val64;
	m_lastTime = m_time;
}

void Timer::endFrame()
{
	AbsoluteTime val64;

	val64 = UpTime();

	double frame = (AbsoluteToDuration(val64) - AbsoluteToDuration(m_lastValue64)) * 0.001;
	m_time += std::min(frame, m_frameClamp);
	m_frame = m_time - m_lastTime;

	// tbd: maybe get rid of this. before startFrame existed, a single call to endFrame calculated delta correctly
	m_lastValue64 = val64;
	m_lastTime = m_time;
}

double Timer::realTime() const
{
	AbsoluteTime val64;

	val64 = UpTime();
	return (double)AbsoluteToDuration(val64) * 0.001;
}
