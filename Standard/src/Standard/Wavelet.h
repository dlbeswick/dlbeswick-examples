#ifndef STANDARD_WAVELET_H
#define STANDARD_WAVELET_H

#include "Standard/api.h"
#include <Standard/ExceptionStream.h>
#include <Standard/MemoryMappedFileIterator.h>
#include <Standard/streamops.h>

//hack to support SWIG
#if IS_X64
	typedef unsigned long long DSOUND_NumericPtr;
#else
	typedef uint DSOUND_NumericPtr;
#endif

class Wavelet
{
public:
};

template <class T>
class WaveletDiscrete : public Wavelet
{
public:
	typedef T Element;

	uint output_vectors_for(uint input_size)
	{
		return (uint)(log((double)input_size) / log(2.0));
	}

	uint output_size_for(uint input_size)
	{
		return input_size;
	}

	uint output_size_bytes_for(uint input_size)
	{
		return output_size_for(input_size) * sizeof(Element);
	}

	void calc(uint input_address, uint input_size, uint output_address, uint output_size, uint work_address, uint work_size) {}
	void calc(Element* input, uint input_size, Element* output, uint output_size, uint work_address, uint work_size) {}
	void calc(const std::string& input_path, uint input_size, const std::string& output_path, uint output_size, const std::string& work_path, uint work_size) {}
	void calc(const std::string& input_path, const std::string& output_path, const std::string& work_path) {}

	void invert(uint input_address, uint input_size, uint output_address, uint output_size, uint work_address, uint work_size) {}
	void invert(Element* input, uint input_size, Element* output, uint output_size, uint work_address, uint work_size) {}
	void invert(const std::string& input_path, uint input_size, const std::string& output_path, uint output_size, const std::string& work_path, uint work_size) {}
	void invert(const std::string& input_path, const std::string& output_path, const std::string& work_path) {}
};

template <class WaveletFuncT>
class WaveletDiscreteWithFunc : public WaveletDiscrete<typename WaveletFuncT::Element>
{
public:
	typedef typename WaveletFuncT::Element Element;
	typedef typename WaveletFuncT::Array Array;
	typedef typename WaveletFuncT::Result Result;
	typedef std::pair<typename Array::const_iterator, typename Array::const_iterator> ArrayRange;

	WaveletFuncT wavelet_func;

	//hack to support SWIG
	void calc(DSOUND_NumericPtr input_address, uint input_size, DSOUND_NumericPtr output_address, uint output_size, DSOUND_NumericPtr work_address, uint work_size)
	{
		calc((Element*)input_address, input_size, (Element*)output_address, output_size, (Element*)work_address, work_size);
	}

	void calc(Element* input, uint input_size, Element* output, uint output_size, Element* work, uint work_size)
	{
		if (output_size != this->output_size_for(input_size))
			throw(ExceptionString("Output size must equal input size."));

		calc(input, input + input_size, output, output + output_size, work, work + work_size);
	}

	void calc(const std::string& input_path, const std::string& output_path, const std::string& work_path)
	{
		MemoryMappedFile in(input_path);
		MemoryMappedFile work(work_path, in.size());
		MemoryMappedFile out(output_path, in.size());

		MemoryMappedFileIterator<Element> it_in(in);
		MemoryMappedFileIterator<Element> it_work(work);
		MemoryMappedFileIterator<Element> it_out(out);

		MemoryMappedFileIterator<Element> it_in_end(in);
		it_in_end.end();
		MemoryMappedFileIterator<Element> it_work_end(work);
		it_work_end.end();
		MemoryMappedFileIterator<Element> it_out_end(out);
		it_out_end.end();

		calc(it_in, it_in_end, it_out, it_out_end, it_work, it_work_end);
	}

/*	void calc(const Array& input, Result& result)
	{
		Array output;
		output.resize(output_size_for(input.size()));

		calc(input.begin(), input.end(), output.begin());

		typename Array::iterator it = output.begin();

		uint vector_size = input.size() / 2;

		while (vector_size > 0)
		{
			result.push_back(Array(it, it + vector_size));

			it += vector_size;
			vector_size /= 2;
		}

		result.push_back(Array(it, it + 1));
	}*/

	template <class IteratorT, class OutIteratorT, class WorkIteratorT>
	void calc(
		const IteratorT& begin,
		const IteratorT& end,
		const OutIteratorT& result_begin,
		const OutIteratorT& result_end,
		const WorkIteratorT& work_begin,
		const WorkIteratorT& work_end
		)
	{
		wavelet_func.calc(begin, end, result_begin, result_end, work_begin, work_end);
	}

	//hack to support SWIG
	void invert(DSOUND_NumericPtr input_address, uint input_size, DSOUND_NumericPtr output_address, uint output_size, DSOUND_NumericPtr work_address, uint work_size)
	{
		invert((Element*)input_address, input_size, (Element*)output_address, output_size, (Element*)work_address, work_size);
	}

	void invert(Element* input, uint input_size, Element* output, uint output_size, Element* work, uint work_size)
	{
		if (output_size != input_size)
			throw(ExceptionString("Output size must equal input size."));

/*		std::vector<std::pair<Element*, Element*> > inputs;

		for (uint vector_size = (input_size - 1) / 2; vector_size > 0; vector_size /= 2)
		{
			Element* input_end = input + vector_size;
			assert(input_end - vector_size >= input);
			inputs.push_back(std::pair<Element*, Element*>(input, input_end));
			input = input_end;
		}

		inputs.push_back(std::pair<Element*, Element*>(input, input + 1));*/

		invert(input, input + input_size, output, output + output_size, work, work + work_size);
	}

	/*void invert(const Result& input, Array& result)
	{
		std::vector<ArrayRange> inputs;

		for (typename Result::const_iterator i = input.begin(); i != input.end(); ++i)
		{
			inputs.push_back(ArrayRange(i->begin(), i->end()));
		}

		result.resize(input.back().size() * 2);

		invert<typename WaveletFuncT::Array::const_iterator, typename WaveletFuncT::Array::iterator>(inputs, result.begin());
	}*/

	/*template <class IteratorT, class OutIteratorT>
	void invert(const std::vector<std::pair<IteratorT, IteratorT> >& inputs, const OutIteratorT& result_begin)
	{
		wavelet_func.invert(inputs, result_begin);
	}*/

	void invert(const std::string& input_path, const std::string& output_path, const std::string& work_path)
	{
		MemoryMappedFile in(input_path);
		MemoryMappedFile work(work_path, in.size());
		MemoryMappedFile out(output_path, in.size());

		MemoryMappedFileIterator<Element> it_in(in);
		MemoryMappedFileIterator<Element> it_work(work);
		MemoryMappedFileIterator<Element> it_out(out);

		MemoryMappedFileIterator<Element> it_in_end(in);
		it_in_end.end();
		MemoryMappedFileIterator<Element> it_work_end(work);
		it_work_end.end();
		MemoryMappedFileIterator<Element> it_out_end(out);
		it_out_end.end();

		invert(it_in, it_in_end, it_out, it_out_end, it_work, it_work_end);
	}

	template <class IteratorT, class OutIteratorT, class WorkIteratorT>
	void invert(const IteratorT& input_begin, const IteratorT& input_end, const OutIteratorT& result_begin, const OutIteratorT& result_end, const WorkIteratorT& work_begin, const WorkIteratorT& work_end)
	{
		wavelet_func.invert(input_begin, input_end, result_begin, result_end, work_begin, work_end);
	}
};

template <class T>
class WaveletFuncs
{
public:
	typedef T Element;
	typedef std::vector<Element> Array;
	typedef std::vector<Array> Result;

	template <class IteratorT, class OutIteratorT, class WorkIteratorT>
	void calc(const IteratorT& begin, const IteratorT& end, const OutIteratorT& result_begin, const OutIteratorT& result_end, const WorkIteratorT& work_begin, const WorkIteratorT& work_end)
	{
	}

	template <class IteratorT, class OutIteratorT, class WorkIteratorT>
	void invert(const IteratorT& begin, const IteratorT& end, const OutIteratorT& result_begin, const OutIteratorT& result_end, const WorkIteratorT& work_begin, const WorkIteratorT& work_end)
	{
	}
};

template <class T>
class WaveletDiscreteFuncs : public WaveletFuncs<T>
{
public:
	template <class IteratorT, class OutIteratorT>
	void calc(const IteratorT& begin, const IteratorT& end, const OutIteratorT& result_begin)
	{
		// this was an assumption made to speed development
		assert(_scaling.size() == _wavelet.size());

		if (begin == end)
			return;

		IteratorT input_begin = begin;
		IteratorT input_end = end;
		OutIteratorT result_it = result_begin;

		int output_size = std::distance(begin, end);

		typename WaveletFuncs<T>::Array temp_result;
		temp_result.resize(output_size);

		while (output_size > 0)
		{
			typename WaveletFuncs<T>::Array::iterator detail_out = temp_result.begin();
			typename WaveletFuncs<T>::Array::iterator scaling_out = detail_out + output_size / 2;

			step(input_begin, input_end, detail_out, scaling_out);

			std::copy(temp_result.begin(), temp_result.begin() + output_size, result_it);

			output_size /= 2;

			input_begin = result_it + output_size;
			input_end = input_begin + output_size;

			result_it += output_size;
		}
	}

	// function expects begin/end iterator pairs progressing from detail to scaling.
	template <class IteratorT, class OutIteratorT>
	void invert(const std::vector<std::pair<IteratorT, IteratorT> >& inputs, const OutIteratorT& result_begin)
	{
		if (inputs.size() < 2)
			throw(ExceptionString("Not enough arrays given as input (must provide at least two arrays - a detail and scaling result)"));

		typename WaveletFuncs<T>::Array temp_result;
		temp_result.resize(std::distance(inputs.front().first, inputs.front().second) * 2);

		invert_step(inputs.rbegin()->first, inputs.rbegin()->second, (inputs.rbegin() + 1)->first, (inputs.rbegin() + 1)->second, result_begin);

		for (typename std::vector<std::pair<IteratorT, IteratorT> >::const_reverse_iterator it = inputs.rbegin() + 2; it != inputs.rend(); ++it)
		{
			uint input_size = std::distance(it->first, it->second);
			invert_step((IteratorT)result_begin, (IteratorT)result_begin + input_size, it->first, it->second, temp_result.begin());
			std::copy(temp_result.begin(), temp_result.begin() + input_size * 2, result_begin);
		}
	}

	const typename WaveletFuncs<T>::Array& wavelet() { return _wavelet; }
	const typename WaveletFuncs<T>::Array& scaling() { return _scaling; }
	const typename WaveletFuncs<T>::Array& wavelet_inverse() { return _wavelet_inverse; }
	const typename WaveletFuncs<T>::Array& scaling_inverse() { return _scaling_inverse; }

protected:
	template <class IteratorT, class RevIteratorT, class OutIteratorT>
	void convolve(const IteratorT& input_begin, const IteratorT& input_end, const RevIteratorT& kernel_r_begin, const RevIteratorT& kernel_r_end, const OutIteratorT& result)
	{
		OutIteratorT result_it = result;

		for (IteratorT i = input_begin; i != input_end; ++i)
		{
			float result_i = 0.0f;

			RevIteratorT it_kernel = kernel_r_begin;

			for (IteratorT it_input = i;
				it_kernel != kernel_r_end && it_input != input_end;
				++it_kernel, ++it_input)
			{
				result_i += *it_input * *it_kernel;
			}

			*(result_it++) = result_i;
		}
	}

	/*template <class IteratorT, class OutIteratorT>
	void invert_step(const IteratorT& scaling_begin, const IteratorT& scaling_end, const IteratorT& detail_begin, const IteratorT& detail_end, const OutIteratorT& result)
	{
		if (std::distance(detail_begin, detail_end) != std::distance(scaling_begin, scaling_end))
			throw(ExceptionString("Scaling and detail arrays must be of equal length."));

		typename WaveletFuncs<T>::Array detail_zero_expand;
		zero_expand(detail_begin, detail_end, detail_zero_expand);

		convolve(detail_zero_expand.begin(), detail_zero_expand.end(), _wavelet_inverse.rbegin(), _wavelet_inverse.rend(), result);

		typename WaveletFuncs<T>::Array scaling_zero_expand;
		zero_expand(scaling_begin, scaling_end, scaling_zero_expand);

		typename WaveletFuncs<T>::Array scaling_result;
		scaling_result.resize(scaling_zero_expand.size());
		convolve(scaling_zero_expand.begin(), scaling_zero_expand.end(), _scaling_inverse.rbegin(), _scaling_inverse.rend(), scaling_result.begin());

		// sum detail and scaling results
		OutIteratorT it0 = result;
		typename WaveletFuncs<T>::Array::const_iterator it1 = scaling_result.begin();
		for (; it1 != scaling_result.end(); ++it0, ++it1)
		{
			*it0 += *it1;
		}
	}*/

	template <class IteratorT>
	void zero_expand(const IteratorT& begin, const IteratorT& end, typename WaveletFuncs<T>::Array& result)
	{
		result.resize(std::distance(begin, end) * 2);

		IteratorT i = begin;
		typename WaveletFuncs<T>::Array::iterator outIt = result.begin();

		for (; i != end; ++i)
		{
			*(outIt++) = *i;
			*(outIt++) = (typename WaveletFuncs<T>::Element)0;
		}
	}

	template <class IteratorT, class OutIteratorT>
	void step(
		const IteratorT& begin,
		const IteratorT& end,
		const OutIteratorT& detail_begin,
		const OutIteratorT& scaling_begin
	)
	{
		int inputSize = std::distance(begin, end);

		int kernelSize = _wavelet.size();

		OutIteratorT detail_it = detail_begin;
		OutIteratorT scaling_it = scaling_begin;

  		for (IteratorT it = begin; it < end; it += 2)
  		{
  			typename WaveletFuncs<T>::Element rd = 0;
  			typename WaveletFuncs<T>::Element rs = 0;

  			for (int i = 0; i < _wavelet.size(); ++i)
  			{
  				IteratorT nextIt = it + i;

  				if (nextIt >= end)
  					nextIt = begin + (nextIt - end);

  				typename WaveletFuncs<T>::Element input = *nextIt;
  				rd += input * _wavelet[kernelSize - i - 1];
  				rs += input * _scaling[kernelSize - i - 1];
  			}

  			*(detail_it++) = rd;
  			*(scaling_it++) = rs;
  		}
	}

	void invert_fir(typename WaveletFuncs<T>::Array& output, const typename WaveletFuncs<T>::Array& filter_partner)
	{
		for (int i = 0; i < (int)filter_partner.size(); ++i)
		{
			output.push_back(pow(-1.0f, 1.0f - i) * filter_partner[(1 - i) % filter_partner.size()]);
		}
	}

	typename WaveletFuncs<T>::Array _wavelet;
	typename WaveletFuncs<T>::Array _scaling;
	typename WaveletFuncs<T>::Array _wavelet_inverse;
	typename WaveletFuncs<T>::Array _scaling_inverse;
};

template <class T>
class WaveletDiscreteHaarFuncs : public WaveletDiscreteFuncs<T>
{
public:
	WaveletDiscreteHaarFuncs()
	{
		WaveletDiscreteFuncs<T>::_wavelet.push_back(1.0 / sqrt(2.0));
		WaveletDiscreteFuncs<T>::_wavelet.push_back(1.0 / sqrt(2.0));

		WaveletDiscreteFuncs<T>::_scaling.push_back(1.0 / sqrt(2.0));
		WaveletDiscreteFuncs<T>::_scaling.push_back(-1.0 / sqrt(2.0));

		WaveletDiscreteFuncs<T>::invert_fir(WaveletDiscreteFuncs<T>::_wavelet_inverse, WaveletDiscreteFuncs<T>::_scaling);
		WaveletDiscreteFuncs<T>::invert_fir(WaveletDiscreteFuncs<T>::_scaling_inverse, WaveletDiscreteFuncs<T>::_wavelet);
	}
};

template <class T>
class WaveletLiftingHaarFuncs : public WaveletFuncs<T>
{
public:
	class Exception : public ExceptionStream
	{
	};

	WaveletLiftingHaarFuncs()
	{
	}

	template <class IteratorT, class OutIteratorT, class WorkIteratorT>
	void calc(const IteratorT& begin, const IteratorT& end, const OutIteratorT& result_begin, const OutIteratorT& result_end, const WorkIteratorT& work_begin, const WorkIteratorT& work_end)
	{
		uint input_size = std::distance(begin, end);

		uint work_size = std::distance(work_begin, work_end);
		if (work_size < input_size)
		{
			EXCEPTIONSTREAM(Exception(), "Size of work buffer must be at least as large as the input buffer (work " << work_size << " vs. " << input_size << ").");
		}

		IteratorT scaling_in_it = begin;
		IteratorT detail_in_it = begin;
		std::advance(detail_in_it, 1);

		const OutIteratorT detail_out = work_begin;
		OutIteratorT scaling_out = work_begin;
		std::advance(scaling_out, input_size / 2);

		OutIteratorT detail_out_it = detail_out;
		OutIteratorT scaling_out_it = scaling_out;

		while (detail_in_it < end)
		{
			typename WaveletFuncs<T>::Element scaling_in = *scaling_in_it;
			typename WaveletFuncs<T>::Element detail_out;

			if (detail_in_it >= end)
			{
				detail_out = (typename WaveletFuncs<T>::Element)0.0 - scaling_in;
			}
			else
			{
				detail_out = *detail_in_it - scaling_in;
				*detail_out_it = detail_out;
			}

			*scaling_out_it = scaling_in + (typename WaveletFuncs<T>::Element)0.5 * detail_out;

			std::advance(scaling_in_it, 2);
			std::advance(detail_in_it, 2);
			++scaling_out_it;
			++detail_out_it;
		}

		WorkIteratorT copy_end = work_begin;
		std::advance(copy_end, input_size);
		std::copy(work_begin, copy_end, result_begin);

		if (input_size > 1)
		{
			IteratorT next_in = result_begin;
			std::advance(next_in, input_size / 2);
			calc(next_in, result_end, next_in, result_end, work_begin, work_end);
		}
	}

	template <class IteratorT, class OutIteratorT, class WorkIteratorT>
	void invert(const IteratorT& begin, const IteratorT& end, const OutIteratorT& result_begin, const OutIteratorT& result_end, const WorkIteratorT& work_begin, const WorkIteratorT& work_end)
	{
		uint input_size = std::distance(begin, end);

		invert_step(begin, end, result_begin, result_end, work_begin, work_end);
	}

	template <class IteratorT, class OutIteratorT, class WorkIteratorT>
	void invert_step(const IteratorT& begin, const IteratorT& end, const OutIteratorT& result_begin, const OutIteratorT& result_end, const WorkIteratorT& work_begin, const WorkIteratorT& work_end)
	{
		uint input_size = std::distance(begin, end);

		uint result_size = std::distance(result_begin, result_end);
		if (result_size < input_size)
		{
			EXCEPTIONSTREAM(Exception(), "Result buffer must be at least as large as the input buffer (result " << result_size << " vs. " << input_size << ").");
		}

		const IteratorT detail_begin = begin;
		IteratorT detail_end = begin + input_size / 2;
		IteratorT scaling_begin = result_begin + input_size / 2;
		const IteratorT scaling_end = result_end;

		if (input_size > 2)
		{
			invert_step(detail_end, end, scaling_begin, result_end, work_begin, work_end);
		}

		uint work_size = std::distance(work_begin, work_end);
		if (work_size < result_size)
		{
			EXCEPTIONSTREAM(Exception(), "Work buffer must be at least as large as the result buffer (work " << work_size << " vs. " << result_size << ").");
	 	}

		IteratorT scaling_in_it = scaling_begin;
		IteratorT detail_in_it = detail_begin;

		OutIteratorT scaling_out_it = work_begin;
		OutIteratorT detail_out_it = work_begin + 1;

		while (detail_in_it < detail_end)
		{
			typename WaveletFuncs<T>::Element scaling_out;

			if (scaling_in_it < scaling_end)
			{
				scaling_out = *scaling_in_it - (typename WaveletFuncs<T>::Element)0.5 * *detail_in_it;
				*detail_out_it = *detail_in_it + scaling_out;
			}
			else
			{
				scaling_out = (typename WaveletFuncs<T>::Element)0.0 - (typename WaveletFuncs<T>::Element)0.5 * *detail_in_it;
			}

			*scaling_out_it = scaling_out;

			++scaling_in_it;
			++detail_in_it;
			scaling_out_it += 2;
			detail_out_it += 2;
		}

		std::copy(work_begin, work_begin + input_size, result_begin);
	}
};

typedef WaveletDiscrete<float> WaveletDiscreteFloat;
typedef WaveletDiscreteWithFunc<WaveletDiscreteHaarFuncs<float> > WaveletDiscreteHaar;
typedef WaveletDiscreteWithFunc<WaveletLiftingHaarFuncs<float> > WaveletLiftingHaar;

#endif
