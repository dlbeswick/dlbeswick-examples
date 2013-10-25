#include "BufferFormat.h"
#include "EffectBase.h"
//#include "FFT.h"
//#include "Plot.h"

//#define DoPlot DialogPlot
#define DoPlot int i = 0;//

class FilterKernelFunction
{
public:
	virtual ~FilterKernelFunction() {}

	virtual void operator() (float* buf, int size) = 0;
	virtual int length() const = 0;
	virtual bool frequencyDomain() const = 0;
};

class BlackmanSincLowpass : public FilterKernelFunction
{
public:
	BlackmanSincLowpass(double _cutoff, double _bandwidth)
	{
		//assert(_cutoff >= 0.0f && _cutoff <= 0.5f);
		//assert(_bandwidth >= 0.0f && _bandwidth <= 0.5f);

		cutoff = _cutoff;
		bandwidth = _bandwidth;
	}

	virtual void operator() (float* buf, int size)
	{
		eval(buf, size);

		float sum = std::accumulate(buf, buf + size, 0.0f);
		for (int i = 0; i < size; ++i)
		{
			buf[i] /= sum;
		}
	}

	virtual int length() const
	{
		return (int)(4.0 / bandwidth);
	}

	virtual bool frequencyDomain() const { return false; }

	double bandwidth;
	double cutoff;

protected:
	void eval(float* buf, int size)
	{
		double m = 4.0 / bandwidth;

		for (int i = 0; i < size; ++i)
		{
			float val;

			if (i == size / 2)
			{
				val = (float)(2.0 * DMath::PI * cutoff);
			}
			else
			{
				double di = (double)i;
				double t1 = sin(2.0 * DMath::PI * cutoff * (di - m/2.0)) / (di - m/2.0);
				double t2 = 0.5 * cos((2.0 * DMath::PI * di) / m);
				double t3 = 0.08 * cos((4.0 * DMath::PI * di) / m);
				val = (float)(t1 * (0.42 - t2 + t3));
			}

			buf[i] = val;
		}

		float sum = std::accumulate(buf, buf + size, 0.0f);
		for (int i = 0; i < size; ++i)
		{
			buf[i] /= sum;
		}
	}
};

class BlackmanSincHighpass : public BlackmanSincLowpass
{
public:
	BlackmanSincHighpass(double _cutoff, double _bandwidth) :
	  BlackmanSincLowpass(_cutoff, _bandwidth)
	{
	}

	virtual void operator() (float* buf, int size)
	{
		eval(buf, size);

		for (int i = 1; i < size; i += 2)
			buf[i] *= -1.0f;
	}
};

#if HAVE_FFTW
class FrequencyResponseFFT : public FilterKernelFunction
{
public:
	FrequencyResponseFFT(int size) :
	  m_size(size)
	{
		m_buffer = new float[m_size * 2 + 2];
	}

	~FrequencyResponseFFT()
	{
		delete[] m_buffer;
	}

	virtual void set(float* buf)
	{
		float* dst = m_buffer;

		// dc sample, real + imaginary
		//*dst++ = 0.0f;
		//*dst++ = 0.0f;

		for (int i = 0; i < m_size; ++i)
		{
			// convert to rectangular notation from polar notation
			// normalise kernel so we don't have to normalise on each convolution
			// real
			*dst++ = *buf++ / (m_size * 2); // [mag * cos(phase)]
			// imaginary
			*dst++ = 0.0f;  // [mag * sin(phase)]
		}
	}

	virtual int length() const { return m_size; }

	virtual void operator() (float* buf, int size)
	{
		float* src = m_buffer;

		for (int i = 0; i < m_size; ++i)
		{
			*buf++ = *src;
			src += 2;
		}
	}

	virtual bool frequencyDomain() const { return true; }

protected:
	int m_size;
	float* m_buffer;
};

class FrequencyResponse : public FrequencyResponseFFT
{
public:
	FrequencyResponse(int size) :
	  FrequencyResponseFFT(size),
	  m_fft(size * 2,1)
	{
	}

	virtual void operator() (float* buf, int size)
	{
		m_fft.setTransformed(m_buffer);
		m_fft.invert(m_buffer);

		std::rotate(m_buffer, m_buffer + m_size / 2, m_buffer + m_size);

		double m = (double)m_size - 1;
		for (int i = 0; i < m_size; ++i)
		{
			double di = (double)i;
			double t1 = 0.5 * cos((2.0 * PI * di) / m);
			double t2 = 0.08 * cos((4.0 * PI * di) / m);
			buf[i] = (float)(m_buffer[i] * (0.42 - t1 + t2));
		}

		memcpy(buf, m_buffer, sizeof(float) * size);
	}

	virtual bool frequencyDomain() const { return false; }
	virtual int length() const { return m_size * 2; }

protected:
	FFT<BufferFormat::LocalFormat> m_fft;
};

template <class Format>
class TFilterKernel
{
public:
	TFilterKernel(FilterKernelFunction& function) :
		m_function(function),
		m_needsEval(true)
	{
	}

	void eval(bool lazy = true)
	{
		m_needsEval = true;
		if (!lazy)
			prepareKernel();
	}

	void prepareKernel() const
	{
		if (m_needsEval)
			doEval();
	}

	const std::vector<float>& kernel() const
	{
		prepareKernel();
		return m_kernel;
	}

	virtual bool convolve(typename Format::SampleElement* buf, int samples)
	{
		prepareKernel();

		/*int kernelSamples = kernel.function().length();

		if ((int)m_buffer.size() < (samples + kernelSamples) * 2)
			m_buffer.resize((samples + kernelSamples) * 2);

		BufferFormat::LocalFormat::copy(&m_buffer.front() + kernelSamples * 2, buf, samples);

		kernel.convolve(&m_buffer.front(), samples + kernelSamples);

		BufferFormat::LocalFormat::copy(&m_buffer.front(), buf + (samples - kernelSamples) * 2, kernelSamples);

		BufferFormat::LocalFormat::copy(buf, &m_buffer.front() + kernelSamples * 2, samples);*/

		int outputSamples = samples + (m_kernel.size() - 1) + (m_kernel.size() - 1);

		if ((int)m_output.size() < outputSamples * 2)
		{
			m_output.resize(outputSamples * 2);
		}

		Format::clear(&m_output.front(), outputSamples);

		typename Format::SampleElement* outL = &m_output.front();
		typename Format::SampleElement* outR = &m_output.front() + 1;
		typename Format::SampleElement* bL = buf;
		typename Format::SampleElement* bR = buf + 1;

		for (int i = 0; i < samples; ++i)
		{
			typename Format::SampleElement* h = &m_kernel.front();

			typename Format::SampleElement* outItL = outL;
			typename Format::SampleElement* outItR = outR;

			for (uint j = 0; j < m_kernel.size(); ++j)
			{
				*outItL += *bL * *h;
				*outItR += *bR * *h;

				outItL += 2;
				outItR += 2;

				++h;
			}

			bL += 2;
			bR += 2;
			outL += 2;
			outR += 2;
		}

		Format::copy(buf, &m_output.front(), samples);

		return true;
	}

	FilterKernelFunction& function() const { return m_function; }

protected:
	FilterKernelFunction& m_function;

	virtual void doEval() const
	{
		m_needsEval = false;

		int kernelSize = m_function.length();

		m_kernel.resize(kernelSize);

		// calculate kernel
		m_function(&m_kernel.front(), kernelSize);
	}

	mutable bool m_needsEval;
	mutable std::vector<float> m_kernel;
	std::vector<typename Format::SampleElement> m_output;
};

typedef TFilterKernel<BufferFormat::LocalFormat> FilterKernel;

template <class Format>
class TFilterKernelFFT : public TFilterKernel<BufferFormat::LocalFormat>
{
public:
	TFilterKernelFFT(FilterKernelFunction& function) :
		TFilterKernel<BufferFormat::LocalFormat>(function),
		m_fft(0),
		m_kernelFft(0)
	{
	}

	~TFilterKernelFFT()
	{
		delete m_fft;
		delete m_kernelFft;
	}

	// a latency of kernelSize samples will be introduced.
	// kernelSize samples must be buffered, then they will be convolved with the proceeding input.
	virtual bool convolve(typename Format::SampleElement* buf, int samples)
	{
		prepareKernel();

		// fft convolve using overlap-add
		int segmentSamples = m_fft->samples();

		if ((int)m_output.size() < segmentSamples * 2)
		{
			m_output.resize(segmentSamples * 2);
		}

		while (samples > 0)
		{
			const int workSamples = std::min(samples, segmentSamples / 2);
			const int workElements = workSamples * 2;

			float* fftResult;
			float* fftEnd;

			// put signal at the start and pad with zeroes
			std::copy(buf, buf + workElements, m_output.begin());
			std::fill(m_output.begin() + workElements, m_output.end(), 0.0f);

			//if (buf[0] != 0.0f)
			//	DoPlot(Plot(&m_output.front(), &m_output.back() + 1, 2), RGBA(1, 1, 1));

			// window signal
			/*double m = (double)workSamples - 1;
			for (int i = 0; i < workElements; i += 2)
			{
				float di = (float)(i / 2);
				float t1 = 0.5f * cos((2.0f * PI * di) / m);
				float t2 = 0.08f * cos((4.0f * PI * di) / m);
				float v = 0.42f - t1 + t2;

				m_output[i] *= v;
				m_output[i+1] *= v;
			}

			if (buf[0] != 0.0f)
				DoPlot(Plot(&m_output.front(), &m_output.back() + 1, 2), RGBA(1, 1, 1));
			*/

			// convolve left channel
			m_fft->transform(&m_output.front());
			fftResult = m_fft->result();
			fftEnd = m_fft->resultEnd();
			//if (buf[0] != 0.0f)
			//	DoPlot(Plot(m_fft->result(), fftEnd, 2), RGBA(1, 0, 1));
			for (uint i = 0; i < m_kernel.size(); ++i)
			{
				// the rectangular form multiplication equation is simplified by assuming the imaginary
				// part of the kernel is always 0, as it is for a zero-phase frequency domain kernel.
				fftResult[0] *= m_kernel[i];
				fftResult[1] *= m_kernel[i];
				fftResult += 2;
			}
			std::fill(fftResult, fftEnd, 0.0f);
			//if (buf[0] != 0.0f)
			//	DoPlot(Plot(m_fft->result(), fftEnd, 2), RGBA(1, 0, 1));
			m_fft->invert(&m_output.front());

			//if (buf[0] != 0.0f)
			//	DoPlot(Plot(&m_output.front(), &m_output.back() + 1, 2), RGBA(1, 0, 0));

			// convolve right channel
			m_fft->transform(&m_output.front() + 1);
			fftResult = m_fft->result();
			fftEnd = m_fft->resultEnd();
			for (uint i = 0; i < m_kernel.size(); ++i)
			{
				// the rectangular form multiplication equation is simplified by assuming the imaginary
				// part of the kernel is always 0, as it is for a zero-phase frequency domain kernel.
				fftResult[0] *= m_kernel[i];
				fftResult[1] *= m_kernel[i];
				fftResult += 2;
			}
			std::fill(fftResult, fftEnd, 0.0f);
			m_fft->invert(&m_output.front() + 1);

			// copy segment to output
			Format::copy(buf, &m_output.front(), workSamples);

			//if (buf[0] != 0.0f)
			//	DoPlot(Plot(buf, buf + workElements, 2), RGBA(0, 1, 0));

			// overlap add
			if (!m_add.empty())
			{
				int addElements = std::min(workElements, (int)m_add.size());

				//if (buf[0] != 0.0f)
				//	DoPlot(Plot(buf, buf + workElements, 2), RGBA(0, 1, 1));

				for (int i = 0; i < addElements; ++i)
				{
//					assert(buf[i] < 40000);
					//buf[i] += m_add[i];
				}

//				m_add.resize(0);

			//if (buf[0] != 0.0f)
			//	DoPlot(Plot(buf, buf + workElements, 2), RGBA(1, 1, 0));
			}

//			m_add.insert(m_add.end(), m_output.begin() + workElements, m_output.begin() + workElements * 2);

//			if (buf[0] != 0.0f)
//				DoPlot(Plot(&m_add.front(), &m_add.back() + 1, 2), RGBA(0.5f, 0.5f, 1));

			buf += workElements;
			samples -= workSamples;
		}

		return true;
	}

protected:
	mutable FFT<BufferFormat::LocalFormat>* m_fft;
	mutable FFT<BufferFormat::LocalFormat>* m_kernelFft;
	std::vector<typename Format::SampleElement> m_add;
	std::vector<typename Format::SampleElement> m_bufferedSamples;

	virtual void doEval() const
	{
		TFilterKernel<Format>::doEval();

		int kernelSize = m_kernel.size();

		if (!m_fft || kernelSize != m_fft->samples())
		{
			delete m_fft;
			m_fft = new FFT<BufferFormat::LocalFormat>(kernelSize, 2);
		}

		// transform kernel to frequency domain if required
		if (!m_function.frequencyDomain())
		{
			if (!m_kernelFft || kernelSize != m_kernelFft->samples())
			{
				delete m_kernelFft;
				m_kernelFft = new FFT<BufferFormat::LocalFormat>(kernelSize, 1);
			}

			m_kernelFft->transform(&m_kernel.front());

			// store frequency domain kernel and premultiply by scaling factor to avoid normalisation after inverse FFT
			float factor = m_fft->samples();
			float* result = m_kernelFft->result();
			for (uint i = 0; i < m_kernel.size(); ++i)
			{
				m_kernel[i] = result[i] / factor;
			}
		}

//		m_bufferedSamples.resize(kernelSize * 2);
	}
};

typedef TFilterKernelFFT<BufferFormat::LocalFormat> FilterKernelFFT;
#endif

namespace Effect
{
#if HAVE_FFTW
	class Filter : public Base<Filter>
	{
	public:
		Filter(FilterKernel& _kernel) :
			kernel(_kernel)
		{
		}

		~Filter()
		{
		}

		bool modify(short*& buf, int& samples, int stride)
		{
			assert(0);
			return false;
		}

		bool modify(float*& buf, int& samples, int stride)
		{
			kernel.convolve(buf, samples);

			return true;
		}

		FilterKernel& kernel;
		std::vector<float> b;

	protected:
		std::vector<BufferFormat::LocalFormat::SampleElement> m_buffer;
	};
#endif

	class FilterIIR : public Base<FilterIIR>
	{
	public:
		void setCoefficients(const std::vector<float>& coefficientsA, const std::vector<float>& coefficientsB)
		{
			m_coefficientsA = coefficientsA;
			m_coefficientsB = coefficientsB;

			// avoid having to iterate twice over output by adding a first B coefficient of zero
			m_coefficientsB.insert(m_coefficientsB.begin(), 1, 0.0f);

			// ensure size of both arrays are equal
			int largest = std::max(m_coefficientsA.size(), m_coefficientsB.size());
			m_coefficientsA.resize(largest);
			m_coefficientsB.resize(largest);
		}

		template <class T>
		bool modify(T*& buf, int& samples, int stride)
		{
			const int historyElements = historySamples() * 2;
			const int requiredElements = samples * 2 + historyElements;

			if ((int)m_input.size() < requiredElements)
				m_input.resize(requiredElements);
			if ((int)m_output.size() < requiredElements)
				m_output.resize(requiredElements);

			std::copy(buf, buf + samples * 2, m_input.begin() + historyElements);

			T* inBegin = (T*)&m_input[historyElements];
			T* outBegin = (T*)&m_output[historyElements];
			T* src = inBegin;
			T* out = outBegin;

			for (int i = 0; i < samples * 2; ++i)
			{
				*out = (T)0.0f;

				for (int j = 0; j < (int)m_coefficientsA.size(); ++j)
				{
					*out += (T)((m_coefficientsA[j] * src[-j*2]) + (m_coefficientsB[j] * out[-j*2]));
				}

				++out;
				++src;
			}

			int outElements = samples * 2;

			// copy output to buffer
			std::copy(outBegin, outBegin + outElements, buf);

			// copy end of output to start of output and input buffers, for use by next frame
			std::copy(outBegin + outElements - historyElements, outBegin + outElements, &m_output[0]);
			std::copy(inBegin + outElements - historyElements, inBegin + outElements, &m_input[0]);

			return true;
		}

	protected:
		int historySamples()
		{
			return m_coefficientsA.size();
		}

		std::vector<float> m_coefficientsA;
		std::vector<float> m_coefficientsB;
		std::vector<BufferFormat::LocalFormat::SampleElement> m_previous;
		std::vector<BufferFormat::LocalFormat::SampleElement> m_input;
		std::vector<BufferFormat::LocalFormat::SampleElement> m_output;
	};

	class FilterOnePole : public FilterIIR
	{
	public:
		FilterOnePole() :
		  m_currentCutoff(FLT_MAX),
		  cutoff(0.0f)
		{
		}

		float cutoff;

		virtual bool modify(float*& buf, int& samples, int stride)
		{
			{
				Critical(_critical);
				if (cutoff != m_currentCutoff)
				{
					setCutoff(cutoff);
					m_currentCutoff = cutoff;
				}
			}

			return FilterIIR::modify(buf, samples, stride);
		}

	protected:
		virtual void setCutoff(float freq) = 0;

		float m_currentCutoff;
	};

	class FilterLowpass : public FilterOnePole
	{
	protected:
		virtual bool needsModify() const { return m_currentCutoff > 0; }

		virtual void setCutoff(float freq)
		{
			float x = freq;
			assert(x >= 0.0f && x <= 1.0f);

			std::vector<float> a;
			a.push_back(1.0f - x);

			std::vector<float> b;
			b.push_back(x);

			setCoefficients(a, b);
		}

		float m_currentCutoff;
	};

	class FilterHighpass : public FilterOnePole
	{
	protected:
		virtual bool needsModify() const { return m_currentCutoff > 0; }

		virtual void setCutoff(float freq)
		{
			float x = exp(-2.0f * DMath::PI * (freq / 2));
			assert(x >= 0.0f && x <= 1.0f);

			std::vector<float> a;
			a.push_back((1.0f + x) / 2.0f);
			a.push_back(-(1.0f + x) / 2.0f);

			std::vector<float> b;
			b.push_back(x);

			setCoefficients(a, b);
		}

		float m_currentCutoff;
	};
}
