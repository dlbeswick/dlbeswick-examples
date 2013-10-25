#include "BufferModifier.h"
#include "Standard/Thread/SafeCopyAccessor.h"

namespace Effect
{
	class Frequency : public Base<Frequency>
	{
	public:
		Frequency() :
			frequencyScale(1.0f),
			m_bufferSeconds(0.5f)
		{
			m_bufferSamples = (int)(m_bufferSeconds * samplesPerSec());
			m_src = BufferFormat::Local.allocate(m_bufferSamples, true);
			m_output = BufferFormat::Local.allocate(m_bufferSamples, true);
		}

		~Frequency()
		{
			delete[] m_src;
			delete[] m_output;
		}

		template <class T>
		bool modify(T*& buf, int& samples, int stride)
		{
			float currentFrequencyScale = frequencyScale;

			if (currentFrequencyScale == 1.0f)
				return false;

			// copy incoming buffer so we can properly interpolate from last sample
			// never copy over first sample -- it's only used for interpolation
			BufferFormat::Local.copy(m_src + 2, (BufferFormat::LocalFormat::SampleElement*)buf, samples);

			float invFreqScale = 1.0f / currentFrequencyScale;
			int newSamples = (int)std::min(samples * invFreqScale, (float)m_bufferSamples);

			assert(newSamples <= m_bufferSamples);

			if (samples == 1.0f)
			{
				return true;
			}

			T* b = (T*)m_output;

			// first sample is used for interpolation
			T* src = (T*)m_src + 2;

			for (int i = 0; i < newSamples; ++i)
			{
				float idx = Mapping::linear((float)i, 0.0f, (float)newSamples-1, 0.0f, (float)samples-1);
				int intIdx = (int)idx;
				float fraction = idx - intIdx;

				assert((intIdx*stride) % 2 == 0);
				assert(intIdx*stride < samples*2);
				*b = (T)Interpolation::linear((float)src[intIdx*stride-stride], (float)src[intIdx*stride], fraction);
				++b;

				assert(((intIdx*stride)+1) % 2 == 1);
				assert(intIdx*stride+1 < samples*2);
				*b = (T)Interpolation::linear((float)src[intIdx*stride-stride+1], (float)src[intIdx*stride+1], fraction);
				++b;
			}

			m_src[0] = b[-stride];
			m_src[1] = b[-stride+1];

			buf = (T*)m_output;
			samples = newSamples;

			return true;
		}

		StandardThread::SafeCopyAccessor<float> frequencyScale;

	protected:
		BufferFormat::LocalFormat::SampleElement* m_src;
		BufferFormat::LocalFormat::SampleElement* m_output;
		int m_bufferSamples;
		float m_bufferSeconds;
	};
}
