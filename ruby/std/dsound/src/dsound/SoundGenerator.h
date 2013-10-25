#pragma once
#include "BufferFormat.h"
#include "BufferModifierHost.h"

class SoundGeneratorOutput;

class SoundGenerator
{
public:
	virtual ~SoundGenerator();

	SoundGeneratorOutput* requestOutput();

	virtual void setEnabled(bool b) { m_enabled = b; }
	virtual bool enabled() const { return m_enabled; }

	inline float samplesPerSec() const { return 44100.0; }

	// When a SoundGenerator has been invalidated, all clients who have consumed data from it must
	// consider that data invalid, and re-request it as soon as possible.
	// SoundGenerators can be invalidated at any stage of processing.
	virtual void invalidate();

	std::vector<SoundGeneratorOutput*> outputs() const { return m_outputs; }

protected:
	SoundGenerator() :
		m_enabled(true)
	{
	}

	friend class SoundGeneratorOutput;

	virtual SoundGeneratorOutput* createOutput();
	virtual void removeOutput(SoundGeneratorOutput* output);

	// functions should return false if no output is available
	virtual bool get(SoundGeneratorOutput& output, BufferFormat::LocalFormat::SampleElement* buf, int samples) { throwf("No implementation."); return false; }

	CriticalSection _accessOutputs;
	std::vector<SoundGeneratorOutput*> m_outputs;

	bool m_enabled;
};
