// ------------------------------------------------------------------------------------------------
//
// Profiler
//
// ------------------------------------------------------------------------------------------------
#pragma once

#include "Standard/api.h"
#include "Singleton.h"

#if IS_X86
    #define PROFILER_RDTSC 1
#endif

class STANDARD_API SProfiler : public Singleton<SProfiler>
{
public:
#if IS_WINDOWS
#if PROFILER_RDTSC
	typedef unsigned long Tick;
#else
	typedef long long Tick;
#endif

	SProfiler();

	struct sProfileRecord
	{
		sProfileRecord() :
			totalMS(0),
			frameTicks(0)
		{
		}

		Tick sampleTicks;
		Tick frameTicks;
		double accumulated;
		double totalMS;
		double frameMS() const { return (frameTicks / (double)SProfiler::m_ticksPerSecond) * 1000.0; }
	};

	typedef stdext::hash_map<const char*, sProfileRecord> Records;

	void accumulate(const char* tag, double value);
	void start(const char* tag);
	void end();
	void finalise();

	const Records& records() { return m_lastFrameRecords; }

private:
	Records m_records;
	Records m_lastFrameRecords;

	static Tick m_ticksPerSecond;

	std::stack<sProfileRecord*> m_activeProfilers;
#endif
};

#if IS_WINDOWS
inline SProfiler& Profiler() { return SProfiler::instance(); }

// auto-ending of profile tags
class STANDARD_API ProfilerSection
{
public:
	ProfilerSection(const char* tag)
	{
		Profiler().start(tag);
	}

	~ProfilerSection()
	{
		Profiler().end();
	}
};

#define Profile(x) ProfilerSection profile_helper(x)
#define ProfileValue(x, y) Profiler().accumulate(x, y);

#endif
