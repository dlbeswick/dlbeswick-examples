#include "fftw3.h"

class FFTBase
{
public:
	FFTBase(int samples, int stride);
	~FFTBase();

	int samples() const { return m_samples; }
	float* result() const { return m_fft; }
	float* resultEnd() const { return m_fft + m_resultSamples * 2; }

protected:
	__forceinline fftwf_plan& planForward(float* inBuf)
	{
		if (!m_forwardInit)
		{
			int inembed = 0;
			int onembed = 0;
			m_fftPlanForward = fftwf_plan_many_dft_r2c(1, &m_samples, 1, (float*)inBuf, &inembed, m_stride, 0, (fftwf_complex*)m_fft, &onembed, 1, 0, FFTW_ESTIMATE);
			m_forwardInit = true;
		}

		return m_fftPlanForward;
	}

	__forceinline fftwf_plan& planBackward(float* outBuf)
	{
		if (!m_backwardInit)
		{
			int inembed = 0;
			int onembed = 0;
			m_fftPlanBackward = fftwf_plan_many_dft_c2r(1, &m_samples, 1, (fftwf_complex*)m_fft, &inembed, 1, 0, outBuf, &onembed, m_stride, 0, FFTW_ESTIMATE);
			m_backwardInit = true;
		}

		return m_fftPlanBackward;
	}

	float* m_src;
	float* m_fft;

	bool m_forwardInit;
	fftwf_plan m_fftPlanForward;
	bool m_backwardInit;
	fftwf_plan m_fftPlanBackward;

	int m_samples;
	int m_resultSamples;
	int m_stride;
};

template <class BufferFormat>
class FFT : public FFTBase
{
public:
	FFT(int samples, int stride) :
		FFTBase(samples, stride)
	{
	}

	__forceinline void transform(typename BufferFormat::SampleElement* source)
	{
		fftwf_execute_dft_r2c(planForward(fromSrc(source)), fromSrc(source), (fftwf_complex*)m_fft);
	}

	template <class T>
	__forceinline void invert(T* target)
	{
		fftwf_execute_dft_c2r(planBackward(m_src), (fftwf_complex*)m_fft, m_src);
		BufferFormat::copy(m_src, target, m_samples);
	}

	__forceinline void invert(float* target)
	{
		fftwf_execute_dft_c2r(planBackward(m_src), (fftwf_complex*)m_fft, target);
	}

	// accepts an array of m_samples/2+1 complex numbers (i.e. m_samples+2 floats)
	__forceinline void setTransformed(float* source)
	{
		memcpy(m_fft, source, sizeof(fftwf_complex) * (m_samples / 2 + 1));
	}

protected:
	template <class T>
	__forceinline float* fromSrc(T* input)
	{
		if (!m_src)
			m_src = new float[m_samples];

		BufferFormat::copy(m_src, input, m_samples);

		return m_src;
	}

	__forceinline float* fromSrc(float* input)
	{
		return input;
	}
};
