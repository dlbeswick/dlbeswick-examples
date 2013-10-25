#include "dsound/pch.h"
#include "FFT.h"

FFTBase::FFTBase(int samples, int stride) :
	m_forwardInit(false),
	m_backwardInit(false),
	m_samples(samples),
	m_resultSamples(samples / 2 + 1),
	m_stride(stride),
	m_src(0)
{
	m_fft = (float*)fftwf_malloc(sizeof(fftwf_complex) * m_resultSamples);
}

FFTBase::~FFTBase()
{
	fftwf_free(m_fft);
	delete[] m_src;

	if (m_forwardInit)
		fftwf_destroy_plan(m_fftPlanForward);

	if (m_backwardInit)
		fftwf_destroy_plan(m_fftPlanBackward);
}
