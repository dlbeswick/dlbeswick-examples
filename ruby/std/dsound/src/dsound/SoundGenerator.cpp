#include "dsound/pch.h"
#include "SoundGenerator.h"
#include "SoundGeneratorOutput.h"

SoundGenerator::~SoundGenerator()
{
	{
		Critical(_accessOutputs);
		std::vector<SoundGeneratorOutput*> outputs = m_outputs;
		for (uint i = 0; i < outputs.size(); ++i)
		{
			delete m_outputs[i];
		}
	}
}

SoundGeneratorOutput* SoundGenerator::requestOutput()
{
	SoundGeneratorOutput* output = createOutput();
	output->invalidate();
	
	{
		Critical(_accessOutputs);
		m_outputs.push_back(output);
	}
	
	return output;
}

SoundGeneratorOutput* SoundGenerator::createOutput()
{
	return new SoundGeneratorOutput(*this);
}

void SoundGenerator::removeOutput(SoundGeneratorOutput* output)
{
	Critical(_accessOutputs);
	m_outputs.erase(std::remove(m_outputs.begin(), m_outputs.end(), output), m_outputs.end());
}

void SoundGenerator::invalidate()
{
	for (uint i = 0; i < m_outputs.size(); ++i)
	{
		m_outputs[i]->invalidate();
	}
}
