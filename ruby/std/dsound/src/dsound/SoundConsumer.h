#ifndef DSOUND_SOUNDCONSUMER_H
#define DSOUND_SOUNDCONSUMER_H

#include "BufferFormat.h"

namespace Effect
{
	class MixGeneratorOutputs;
}

class SoundGeneratorOutput;

class SoundConsumer
{
public:
	SoundConsumer();
	virtual ~SoundConsumer();

	virtual void addInput(SoundGeneratorOutput& output);
	virtual void removeInput(SoundGeneratorOutput& output);
	virtual void clearInputs();
	virtual std::vector<SoundGeneratorOutput*> inputs() const;

protected:
	// function returns false if there is nothing to consume
	bool consumeTo(BufferFormat::LocalFormat::SampleElement* buf, int samples);

	bool allInputsValid() const;

	Effect::MixGeneratorOutputs* m_mixInputs;
};

#endif
