#include "dsound/pch.h"
#include "SoundConsumer.h"
#include "EffectMixGeneratorOutputs.h"

SoundConsumer::SoundConsumer() :
	m_mixInputs(new Effect::MixGeneratorOutputs)
{
}

SoundConsumer::~SoundConsumer()
{
	delete m_mixInputs;
}

void SoundConsumer::addInput(SoundGeneratorOutput& output)
{
	m_mixInputs->add(output);
}

void SoundConsumer::removeInput(SoundGeneratorOutput& output)
{
	m_mixInputs->remove(output);
}

void SoundConsumer::clearInputs()
{
	m_mixInputs->clear();
}

bool SoundConsumer::consumeTo(BufferFormat::LocalFormat::SampleElement* buf, int samples)
{
	BufferFormat::LocalFormat::clear(buf, samples);

	bool result = m_mixInputs->modify(buf, samples, 2);

	return result;
}

bool SoundConsumer::allInputsValid() const
{
	const Effect::MixGeneratorOutputs::Inputs& inputs = m_mixInputs->inputs();
	
	for (uint i = 0; i < inputs.size(); ++i)
	{
		if (!inputs[i]->valid())
			return false;
	}

	return true;
}

std::vector<SoundGeneratorOutput*> SoundConsumer::inputs() const 
{ 
	return m_mixInputs->inputs(); 
}
