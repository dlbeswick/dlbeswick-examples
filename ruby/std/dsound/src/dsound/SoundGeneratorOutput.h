#ifndef DSOUND_SOUNDGENERATOROUTPUT_H
#define DSOUND_SOUNDGENERATOROUTPUT_H

#include "SoundGenerator.h"
#include "Standard/Lockable.h"

class SoundGeneratorOutput : public Lockable
{
public:
	SoundGeneratorOutput(SoundGenerator& generator);

	template <class T>
	bool get(T* buf, int samples)
	{
		return m_generator.get(*this, buf, samples);
	}

	void invalidate() 
	{ 
		Critical(*this);
		m_valid = false;
		readCursor = -1; 
		remainingSamples = 0; 
	}

	bool valid() const { return m_valid; }
	void validate() { m_valid = true; }

	SoundGenerator& m_generator;

	int readCursor;
	int remainingSamples;

	void safeSetReadCursor(int pos)
	{
		Critical(*this);
		readCursor = pos;
	}

	int safeReadCursor() const
	{
		Critical(*this);
		return readCursor;
	}

	void safeSetRemainingSamples(int samples)
	{
		Critical(*this);
		remainingSamples = samples;
	}

	int safeRemainingSamples() const
	{
		Critical(*this);
		return remainingSamples;
	}

	void syncTo(SoundGeneratorOutput& output);

protected:
	bool m_valid;
};

#endif
