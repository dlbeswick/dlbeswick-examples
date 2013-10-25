#ifndef DSOUND_EFFECTRESAMPLE_H
#define DSOUND_EFFECTRESAMPLE_H

#include "BufferModifier.h"
#include "Standard/Thread/SafeCopyAccessor.h"
#include "Standard/Exception.h"
#include <samplerate.h>

class EffectResample : public Effect::Base<EffectResample>
{
public:
  EffectResample() :
    _resample(0),
	_available_samples(0)
  {
    int result;
    
    _resample =
      src_new(SRC_SINC_FASTEST, 2, &result);

    if (result != 0)
      throw(ExceptionString(src_strerror(result)));
  }

  ~EffectResample()
  {
    src_delete(_resample);
  }
  
  bool modify(short int*& buf, int samples, int stride)
  {
	assert(0);
	return false;
  }
  
  bool modify(float*& buf, int samples, int stride)
  {
    const int element_size = sizeof(float);
    float* output = buf;
    const float* output_end = buf + samples;

    while (output != output_end)
	{
	  assert(_available_samples <= _output_buffer.size());

      const int samples_to_copy = std::min(_available_samples, samples);
      memcpy(output, &_output_buffer[0], samples_to_copy * element_size * channels());

      output += samples_to_copy;

      if (output != output_end)
      {
		float ratio = _factor;
		src_set_ratio(_resample, ratio);

		const int output_samples_max = (int)(ceilf(samples * ratio));
		const int output_bytes = output_samples_max * channels() * element_size;
	
		if (_output_buffer.size() < output_bytes)
		  _output_buffer.resize(output_bytes);
    
		int samples_to_consume = samples;
		char* src_output_buf = &_output_buffer[0];
	
		while (samples_to_consume > 0)
		{
		  SRC_DATA data;
		  data.data_in = output;
		  data.data_out = (float*)src_output_buf;
		  data.input_frames = samples;
		  data.output_frames = output_samples_max;
		  data.src_ratio = ratio;
		  data.end_of_input = 0;

		  assert(src_output_buf < &_output_buffer[0] + _output_buffer.size());
		  src_process(_resample, &data);
		  
		  samples_to_consume -= data.input_frames_used;
		  src_output_buf += data.output_frames_gen * channels() * element_size;
		  _available_samples += data.output_frames_gen;
		}
      }
	}
	
	return true;
  }

  void setInputRate(int rate)
  {
    _input_rate = rate;
    _factor = _output_rate / _input_rate;
  }

  void setOutputRate(int rate)
  {
    _output_rate = rate;

    if (_input_rate)
      _factor = _output_rate / _input_rate;
  }
  
private:
  int _input_rate;
  int _output_rate;
  int _available_samples;
  StandardThread::SafeCopyAccessor<double> _factor;
  std::vector<char> _output_buffer;
  SRC_STATE* _resample;
};

#endif
