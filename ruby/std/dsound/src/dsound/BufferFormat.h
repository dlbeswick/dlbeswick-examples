#ifndef DSOUND_BUFFERFORMAT_H
#define DSOUND_BUFFERFORMAT_H

#include <cassert>
#include <cstring>

// I chose to call 2 channels of audio data a "sample" here.
namespace BufferFormat
{
	template <class T>
	class Base
	{
	public:
		typedef T SampleElement;
		static SampleElement* allocate(int samples, bool clear=true)
		{
			assert(samples > 0);

			SampleElement* buf = new SampleElement[samples * 2];
			if (clear)
				Base::clear(buf, samples);

			return buf;
		}

		static SampleElement* buf(void* buf) { return (SampleElement*)buf; }
		static int bytesPerSample() { return sizeof(SampleElement) * 2; }
		static float max() { return 1.0f; }

		static void copy(SampleElement* buf, const SampleElement* srcBuf, int samples)
		{
			memcpy(buf, srcBuf, bytesPerSample() * samples);
		}

		static void mix(SampleElement* buf, const SampleElement* srcBuf, int samples, float dryToWet)
		{
			for (int i = 0; i < samples; ++i)
			{
				*(buf++) += *(srcBuf++) * dryToWet;
				*(buf++) += *(srcBuf++) * dryToWet;
			}
		}

		static void scaleCopy(SampleElement* buf, const SampleElement* srcBuf, int samples, float scale)
		{
			for (int i = 0; i < samples; ++i)
			{
				*(buf++) += *(srcBuf++) * scale;
				*(buf++) += *(srcBuf++) * scale;
			}
		}

		static void clear(SampleElement* buf, int samples)
		{
			memset(buf, 0, samples * bytesPerSample());
		}
	};

	class Float : public Base<float>
	{
	public:
		static void copy(SampleElement* buf, const SampleElement* srcBuf, int samples)
		{
			memcpy(buf, srcBuf, bytesPerSample() * samples);
		}
	};

	class Short : public Base<short>
	{
	public:
		static void copy(SampleElement* buf, const SampleElement* srcBuf, int samples)
		{
			memcpy(buf, srcBuf, bytesPerSample() * samples);
		}

		static void copy(SampleElement* buf, const Float::SampleElement* srcBuf, int samples)
		{
			/*__m64* mSrcBuf = (__m64*)srcBuf;
			__m64 val;
			int* pVal = (int*)&val;
			int iVal;
			int* iBuf = (int*)buf;

			for (int i = 0; i < samples; ++i)
			{
				val = _m_pf2id(*mSrcBuf++);
				iVal = *pVal << 16;
				iVal |= *(pVal+1) & 0x0000FFFF;
				*iBuf++ = iVal;
			}

			_mm_empty();*/
			for (int i = 0; i < samples; ++i)
			{
				*buf++ = (short)*srcBuf++;
				*buf++ = (short)*srcBuf++;
			}
		}

		static void scaleCopy(SampleElement* buf, const Float::SampleElement* srcBuf, int samples, float scale)
		{
			for (int i = 0; i < samples; ++i)
			{
				*buf++ = (short)((*srcBuf++) * scale);
				*buf++ = (short)((*srcBuf++) * scale);
			}
		}
	};

	__forceinline float anyToFloat(int bytesPerSample, void* element)
	{
		// make a guess as the type of the sample, and convert it to a float
		if (bytesPerSample == Short::bytesPerSample())
			return (float)*Short::buf(element);
		else if (bytesPerSample == Float::bytesPerSample())
			return (float)*Float::buf(element);

		assert(0);
		return 0;
	}

	typedef Float LocalFormat;
	typedef Short LibraryFormat;

	extern LocalFormat Local;
	extern LibraryFormat Library;
}

#endif
