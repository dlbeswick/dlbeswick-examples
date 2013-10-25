#pragma once
#include "SoundConsumer.h"
#include "SoundGenerator.h"

class SoundGeneratorOutput;

class SoundModifier : public SoundConsumer, public SoundGenerator
{
public:
	virtual bool get(SoundGeneratorOutput& output, BufferFormat::LocalFormat::SampleElement* buf, int samples);

protected:
	CriticalSection m_criticalGet;
};
