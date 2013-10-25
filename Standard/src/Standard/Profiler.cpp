// ------------------------------------------------------------------------------------------------
//
// Profiler
//
// ------------------------------------------------------------------------------------------------
#include "Standard/pch.h"
#include "Profiler.h"
#include "Help.h"
#include "Log.h"

#if IS_WINDOWS

SProfiler::Tick SProfiler::m_ticksPerSecond;

SProfiler::SProfiler()
{
#if PROFILER_RDTSC
	m_ticksPerSecond = getCPUTicksPerSecond();
#else
	QueryPerformanceFrequency((LARGE_INTEGER*)&m_ticksPerSecond);
#endif
}

void SProfiler::start(const char* tag)
{
	sProfileRecord& r = m_records[tag];
	m_activeProfilers.push(&m_records[tag]);

	Tick ticks;
#if PROFILER_RDTSC
#if IS_MSVC
	__asm
	{
		rdtsc
		mov [ticks], eax
	}
#elif IS_GNUC
	asm
	(
		"rdtsc;"
		"mov %%eax, %0"
		:"=r"(ticks)
		:
		:"%eax"
	);
#endif
#else
	QueryPerformanceCounter((LARGE_INTEGER*)&ticks);
#endif

	r.sampleTicks = ticks;
}

void SProfiler::accumulate(const char* tag, double value)
{
	sProfileRecord& r = m_records[tag];

	r.sampleTicks = 0;
	r.frameTicks = 0;
	r.accumulated += value;
	r.totalMS += value;
}

void SProfiler::end()
{
	Tick ticks;
#if PROFILER_RDTSC
	#if IS_MSVC
		__asm
		{
			rdtsc
			mov [ticks], eax
		}
	#elif IS_GNUC
		asm
		(
			"rdtsc;"
			"mov %%eax, %0"
			:"=r"(ticks)
			:
			:"%eax"
		);
	#endif
#else
	QueryPerformanceCounter((LARGE_INTEGER*)&ticks);
#endif

	if (m_activeProfilers.empty())
		return;

	sProfileRecord& r = *m_activeProfilers.top();

	r.frameTicks += ticks - r.sampleTicks;
	r.totalMS += (r.frameTicks / (double)m_ticksPerSecond) * 1000.0;

	m_activeProfilers.pop();
}

void SProfiler::finalise()
{
	m_lastFrameRecords = m_records;
	if (!m_activeProfilers.empty())
		dlog << "UNCLOSED PROFILE CALL" << dlog.endl;

	for (Records::iterator i = m_records.begin(); i != m_records.end(); ++i)
	{
		i->second.accumulated = 0;
		i->second.frameTicks = 0;
	}
}

#endif
