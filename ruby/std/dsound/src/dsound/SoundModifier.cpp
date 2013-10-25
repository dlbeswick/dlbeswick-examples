#include "dsound/pch.h"
#include "SoundModifier.h"
#include "SoundGeneratorOutput.h"

bool SoundModifier::get(SoundGeneratorOutput& output, BufferFormat::LocalFormat::SampleElement* buf, int samples)
{
	Critical(m_criticalGet);

	// if any of our inputs have been invalidated, then invalidate all our outputs
	/*if (inputInvalidated())
	{
		for (uint i = 0; i < m_outputs.size(); ++i)
		{
			m_outputs[i]->invalidate();
		}
	}*/

	return true;
}

