#include "dsound/pch.h"
#include "ModifierNode.h"
#include "SoundGeneratorOutput.h"

ModifierNode::ModifierNode(float bufferSeconds) :
	m_samplesRemaining(0),
	m_writeCursor(0),
	m_lastWritePos(0)
{
	m_buffer.resize((uint)(bufferSeconds * samplesPerSec() * 2));
	m_work = new BufferFormat::LocalFormat::SampleElement[m_buffer.size()];
}

ModifierNode::~ModifierNode()
{
	delete[] m_work;
}

bool ModifierNode::get(SoundGeneratorOutput& output, BufferFormat::LocalFormat::SampleElement* buf, int samples)
{
	SoundModifier::get(output, buf, samples);

	if (!enabled())
		return false;

	// copy modified samples to the output
	while (samples > 0)
	{
		bool needGet;

		{
			Critical(output);
			needGet = output.remainingSamples <= 0;
		}

		// we're out of samples, get samples from stream buffer and modify them
		if (needGet)
		{
			int workSamples = samples;

			{
				Critical(m_criticalGet);

				// we should request a minimum number of samples from our modifiers.
				// this can introduce more latency for effects changes, but many effects will not produce
				// meaningful results for small sample inputs.
				clamp(workSamples, BufferModifier::minimumSamples(), m_buffer.size() / 2);

#if _DEBUG
				// check to see if we'll overwrite data any of our clients are still using; shouldn't happen in normal operation.
	/*			for (uint i = 0; i < m_outputs.size(); ++i)
				{
					SoundGeneratorBufferedOutput* o = (SoundGeneratorBufferedOutput*)m_outputs[i];
					assert(!o->m_buffer || o == &output || (o->m_buffer - m_modified) > workSamples);
				}*/
#endif

				// mix inputs to work buffer.
				// the input is not mixed directly to our circular buffer to ensure that a minimum
				// number of samples can be guaranteed to applyModifiers. it also avoids two modify calls.
				if (!consumeTo(m_work, workSamples))
                {
                    if (hasModifiers())
                    {
                        // If no inputs have been added, sound data may still be available from modifiers.
                        // For instance, one of the modifiers may be a MixGeneratorOutputs that is connected to
                        // a sound generator.
                        // Pass blank sound to modifiers.
                        // This scenario should be fixed once effects are connected nodes, rather than lists in a BufferModifier.
                        BufferFormat::LocalFormat::clear(m_work, workSamples);

                        if (!applyModifiers<BufferFormat::LocalFormat>(m_work, workSamples, 2))
                            return false;
                    }
                    else
                    {
                        return false;
                    }
                }
                else
                {
                    // apply modifiers to the work buffer
                    applyModifiers<BufferFormat::LocalFormat>(m_work, workSamples, 2);
                }

				// copy from work buffer to master buffer
				m_buffer.copyFrom(m_writeCursor, m_work, workSamples * 2);

				m_lastWritePos = m_writeCursor;

				m_buffer.advance(m_writeCursor, workSamples * 2);
			}

			// tell outputs there is new data
			for (uint i = 0; i < m_outputs.size(); ++i)
			{
				SoundGeneratorOutput& o = *m_outputs[i];

				Critical(o);

				// tbd: find a way to test for overwrites on outputs
				o.remainingSamples += workSamples;
				if (!o.valid())
				{
					o.readCursor = m_lastWritePos;
					o.validate();
				}
			}
		}

		int copySamples;

		{
			Critical(output);
			copySamples = std::min(samples, output.remainingSamples);

			// copy modify buffer to output buffer
			if (output.readCursor == -1)
				return false;

			m_buffer.copyTo(output.readCursor, buf, copySamples * 2);
			assert(copySamples > 0);
			m_buffer.advance(output.readCursor, copySamples * 2);
			assert(copySamples > 0);
			output.remainingSamples -= copySamples;
		}

		buf += copySamples * 2;
		samples -= copySamples;
	}

/*	for (int i = 0; i < m_modified.size(); ++i)
	{
		assert(*(m_modified.buf() + i) == 0);
	}*/

	return true;
}

int ModifierNode::visBufferSize() const
{
	return m_buffer.size() / 2;
}

void ModifierNode::visGet(int position, int samples, void const*& data0, int& samples0, void const*& data1, int& samples1) const
{
	m_buffer.get(position * 2, samples * 2, (BufferFormat::LocalFormat::SampleElement const*&)data0, samples0, (BufferFormat::LocalFormat::SampleElement const*&)data1, samples1);
	samples0 /= 2;
	samples1 /= 2;
}

int ModifierNode::visPosition() const
{
	return m_writeCursor / 2;
}

int ModifierNode::visBytesPerSample() const
{
	return BufferFormat::Local.bytesPerSample();
}

int ModifierNode::visSampleDistance(int pos0, int pos1) const
{
	return m_buffer.distance(pos0 * 2, pos1 * 2) / 2;
}
