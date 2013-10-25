#include "dsound/pch.h"
#include "SoundGeneratorOutput.h"

SoundGeneratorOutput::SoundGeneratorOutput(SoundGenerator& generator) :
	m_generator(generator),
	readCursor(-1),
	remainingSamples(0),
	m_valid(false)
{
}

void SoundGeneratorOutput::syncTo(SoundGeneratorOutput& output)
{
	Critical(*this);
	{
		Critical(output);

		readCursor = output.readCursor;
		remainingSamples = output.remainingSamples;
	}
}
