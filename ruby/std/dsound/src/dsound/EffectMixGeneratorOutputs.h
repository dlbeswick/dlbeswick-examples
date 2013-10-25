#include "EffectBase.h"
#include "SoundGeneratorOutput.h"

namespace Effect
{
	class MixGeneratorOutputs : public Base<MixGeneratorOutputs>
	{
	public:
		typedef std::vector<SoundGeneratorOutput*> Inputs;

		MixGeneratorOutputs(float seconds = 1.0f)
		{
			m_bufferSamples = (int)(seconds * 44100);
			m_mixBuf = BufferFormat::Local.allocate(m_bufferSamples);
		}

		void add(SoundGeneratorOutput& g)
		{
			Critical(_critical);

			if (std::find(m_generators.begin(), m_generators.end(), &g) != m_generators.end())
				throwf("MixGeneratorOutputs: tried to add an input that was already connected.");

			m_generators.push_back(&g);
		}

		void clear()
		{
			Critical(_critical);
			m_generators.clear();
		}

		bool empty()
		{
			Critical(_critical);
			return m_generators.empty();
		}

		template <class T>
		bool modify(T*& buf, int samples, int stride)
		{
            Critical(_critical);

			if (m_generators.empty())
				return false;

			bool modified = false;

			for (uint i = 0; i < m_generators.size(); ++i)
			{
				if (m_generators[i]->get(m_mixBuf, samples))
				{
					modified = true;
					BufferFormat::Local.mix((BufferFormat::LocalFormat::SampleElement*)buf, m_mixBuf, samples, dryToWetMultiplier());
				}
			}

			return modified;
		}

		Inputs inputs() const
		{
			Critical(_critical);
			return m_generators;
		}

		void remove(SoundGeneratorOutput& g)
		{
			Critical(_critical);

			if (std::find(m_generators.begin(), m_generators.end(), &g) == m_generators.end())
				throwf("MixGeneratorOutputs: tried to remove an input that was not connected.");

			m_generators.erase(std::remove(m_generators.begin(), m_generators.end(), &g), m_generators.end());
		}

	protected:
		BufferFormat::LocalFormat::SampleElement* m_mixBuf;
		Inputs m_generators;
		int m_bufferSamples;
	};
}
