#include "BufferModifier.h"
#pragma warning(disable:4244)

namespace Effect
{
	class Hold : public Base<Hold>
	{
	public:
		Hold(float maxSeconds=0.5f)
		{
			m_holdSize = 0;
			m_holdBuffer.resize((int)(maxSeconds * samplesPerSec() * 2));

			m_maxSeconds = maxSeconds;
			setHoldSeconds(maxSeconds * 0.1f);
			release();
		}

		template <class T>
		bool modify(T*& buf, int& samples, int stride)
		{
			Critical(m_modifying);

			int addElements = std::min(samples * 2, (int)m_holdElements);
			bool record = m_hold;

			if (record)
			{
				if (!m_hold)
				{
					// make room for new samples
					if (m_holdSize + addElements > m_holdElements)
					{
						int remove = m_holdSize + addElements - m_holdElements;
						std::copy(m_holdBuffer.begin() + remove, m_holdBuffer.begin() + m_holdElements, m_holdBuffer.begin());
						m_holdSize -= remove;
					}
				}

				// fill the hold buffer up to maximum
				if (m_holdSize < (int)m_holdBuffer.size())
				{
					T* copyStart = buf;

					// discard samples that won't fit
					if (samples * 2 > m_holdElements)
						copyStart += samples * 2 - m_holdElements;

					std::copy(copyStart, copyStart + addElements, m_holdBuffer.begin() + m_holdSize);
					m_holdSize += addElements;
				}
			}

			if (m_hold)
			{
				int elementsLeft = samples * 2;
				BufferFormat::LocalFormat::SampleElement* copyBuf = (BufferFormat::LocalFormat::SampleElement*)buf;

				while (elementsLeft > 0)
				{
					int copyElements = std::min(m_holdElements, elementsLeft);

					// output the hold buffer
					m_circularHold.copyTo(m_holdReadCursor, copyBuf, copyElements);
					m_circularHold.advance(m_holdReadCursor, copyElements);
					elementsLeft -= copyElements;
					copyBuf += copyElements;
				}

				return true;
			}

			return false;
		}

		void hold()
		{
			m_hold = true;
		}

		void release()
		{
			m_holdSize = 0;
			m_hold = false;
			m_holdReadCursor = 0;
		}

		void setHoldSeconds(float seconds)
		{
			Critical(m_modifying);

			clamp(seconds, 0.0f, m_maxSeconds);
			m_holdSeconds = std::max(seconds, 0.002f);

			m_holdElements = std::max((int)(m_holdSeconds * samplesPerSec() * 2), 4);

			m_circularHold.set(&*m_holdBuffer.begin(), m_holdElements);
			m_holdReadCursor = std::min(m_holdReadCursor, (int)m_holdElements);
		}

		float maxHoldSeconds() const
		{
			return m_maxSeconds;
		}

		float holdSeconds() const
		{
			return m_holdSeconds;
		}

	protected:
		float m_maxSeconds;
		float m_holdSeconds;
		int m_holdElements;
		int m_holdSize;
		bool m_hold;
		bool m_lastHold;
		int m_holdReadCursor;
		CriticalSection m_modifying;

		std::vector<BufferFormat::LocalFormat::SampleElement> m_holdBuffer;
		CircularBufferPtr<BufferFormat::LocalFormat::SampleElement> m_circularHold;
	};
}
