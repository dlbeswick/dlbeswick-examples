#include "BufferModifier.h"
#pragma warning(disable:4244)

namespace Effect
{
	class Limiter : public Base<Limiter>
	{
	public:
		Limiter(const ::Gain& dbThreshold=::Gain(-0.2f), float attackSeconds=1, float releaseSeconds=1) :
			m_gain(1.0f),
			m_currentHold(0)
		{
			setAttack(attackSeconds);
			setRelease(releaseSeconds);
			setThreshold(dbThreshold);

			m_releaseDurationElements = m_releaseElements;
		}

		void shiftBuffer(int elements, const std::vector<BufferFormat::LocalFormat::SampleElement>& buffer)
		{
		}

		template <class T>
		bool modify(T*& buf, int samples, int stride)
		{
			T* lookAhead;
			T* out;

			// Maybe I planned to have lookahead sometime, but I don't know why, it
			// introduces lag. It's disabled.
			if (!m_lookAhead.empty() && samples < (int)m_lookAhead.size() / 2)
			{
				assert(0); // lookahead not working
				memmove(&*(m_lookAhead.begin() + samples * 2), &*m_lookAhead.begin(), sizeof(T) * (m_lookAhead.size() - samples * 2));
				BufferFormat::LocalFormat::copy((BufferFormat::LocalFormat::SampleElement*)&m_lookAhead[0], (BufferFormat::LocalFormat::SampleElement*)buf, samples);
				lookAhead = (T*)&m_lookAhead[0];
				out = (T*)&*(m_lookAhead.end() - samples * 2);
			}
			else
			{
				lookAhead = buf;
				out = buf;
			}

			T* b = out;

			for (int i = 0; i < samples * 2; ++i)
			{
				BufferFormat::LocalFormat::SampleElement level = abs(*lookAhead);

				if (m_releaseDurationElements < m_releaseElements && m_currentHold <= 0)
				{
					++m_releaseDurationElements;
					m_gain = std::min(1.0f, Mapping::linear(m_releaseDurationElements, 0.0f, m_releaseElements, m_gainReleaseStart, 1.0f));
				}

				if (m_currentHold > 0)
					--m_currentHold;

				if (level > m_threshold)
				{
					float newGain = m_threshold / level;

					if (newGain < m_gain)
					{
						m_gain = newGain;
						m_gainReleaseStart = m_gain;
					}

					m_releaseDurationElements = 0;
					m_currentHold = m_hold;
				}

				assert(abs(*b * m_gain) <= m_threshold + 10);
				*b = (T)clamp(*b * m_gain, -m_threshold, m_threshold);
				++b;
				++lookAhead;
			}

			buf = out;

			return true;
		}

		void setThreshold(const ::Gain& dbThreshold)
		{
			Critical(_critical);
            m_threshold = dbThreshold.scale() * BufferFormat::Local.max();
		}

		void setAttack(float time)
		{
			Critical(_critical);
			m_attack = time;
			m_lookAheadTime = time;
			m_hold = (int)m_lookAheadTime;
		}

		void setRelease(float time)
		{
			Critical(_critical);
			m_release = time;
			m_releaseElements = m_release * samplesPerSec() * 2;
		}

	protected:
		float m_gain;
		float m_gainReleaseStart;
		float m_lookAheadTime;
		float m_threshold;
		float m_attack;
		float m_release;
		float m_releaseElements;
		float m_releaseDurationElements;
		int m_currentHold;
		int m_hold;
		std::vector<BufferFormat::LocalFormat::SampleElement> m_lookAhead;
		std::vector<BufferFormat::LocalFormat::SampleElement> m_lookAheadResults;
	};
}
