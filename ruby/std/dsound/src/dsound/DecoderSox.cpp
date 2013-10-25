#include "dsound/DecoderSox.h"

#if WITH_SOX

#include "dsound/SoundLibrary.h"
#include "dsound/common.h"
#include <assert.h>
//#include <FLAC/stream_encoder.h>
//#include <FLAC/stream_decoder.h>

ExceptionDecoderSox::ExceptionDecoderSox(const std::string& s) :
	ExceptionDecoder(s)
{
}

DecoderSox::DecoderSox(class SoundLibrary& library, const std::string& path) :
	DSoundDecoder(library),
	_format(0),
	_decode_buffer(0),
	_decode_buffer_size(0),
	_finished(false),
	_progress_samples(0),
	_path(path)
{
	library.ensureSox();
	open(_path);
}

void DecoderSox::open(const std::string& path)
{
	_format = sox_open_read(path.c_str(), 0, 0, 0);

	if (!_format)
		throw(ExceptionDecoderSox("sox_open_read failed."));

	const int channels = _format->signal.channels;

	if (channels > 2)
		throw(ExceptionDecoderSox("Decoding is supported only for files with 2 channels or less."));

	_decode_buffer_size = 11026;

	// ensure that decode buffer has a number of elements that's a multiple of "channels".
	_decode_buffer_size += (_decode_buffer_size % channels);

	_decode_buffer = new sox_sample_t[_decode_buffer_size];
}

void DecoderSox::close()
{
	if (_format)
	{
		sox_close(_format);
		_format = 0;
	}
}

DecoderSox::~DecoderSox()
{
	delete[] (sox_sample_t*)_decode_buffer;
}

int DecoderSox::getPCM(float* out, int samples)
{
	if (finished())
		return 0;

	int retrieved_samples = 0;
	const int channels = _format->signal.channels;

	while (samples > 0)
	{
		sox_sample_t* const decode_buffer = (sox_sample_t*)_decode_buffer;

		size_t result = sox_read(_format, decode_buffer, std::min(samples * channels, _decode_buffer_size));

		if (result == 0 || result == SOX_EOF)
		{
			_finished = true;
			break;
		}

		SOX_SAMPLE_LOCALS;
		size_t clips;
		const int retrieved_samples_block = result;

		if (channels == 2)
		{
			for (int i = 0; i < retrieved_samples_block; ++i)
				*out++ = SOX_SAMPLE_TO_FLOAT_32BIT(decode_buffer[i], clips);
		}
		else
		{
			float sample;

			for (int i = 0; i < retrieved_samples_block; ++i)
			{
				sample = SOX_SAMPLE_TO_FLOAT_32BIT(decode_buffer[i], clips);
				*out++ = sample;
				*out++ = sample;
			}
		}

		samples -= retrieved_samples_block / channels;
		retrieved_samples += retrieved_samples_block / channels;
		_progress_samples += retrieved_samples_block / channels;
	}

	return retrieved_samples;
}

int DecoderSox::getPCM(short* out, int samples)
{
	assert(0);
	return 0;
}

bool DecoderSox::finished() const
{
	return _finished;
}

double DecoderSox::progress() const
{
	return 1000.0 * (float)_progress_samples / _format->signal.rate;
}

size_t DecoderSox::lengthSamples() const
{
	return _format->signal.length / _format->signal.channels;
}

double DecoderSox::lengthMS() const
{
	return 1000.0 * (double)lengthSamples() / _format->signal.rate;
}

int DecoderSox::rate() const
{
  if (!_format)
    return 0;

  return _format->signal.rate;
}

#if 0
typedef struct {
  /* Info: */
  unsigned bits_per_sample;
  unsigned channels;
  unsigned sample_rate;
  unsigned total_samples;

  /* Decode buffer: */
  FLAC__int32 const * const * decoded_wide_samples;
  unsigned number_of_wide_samples;
  unsigned wide_sample_number;

  FLAC__StreamDecoder * decoder;
  FLAC__bool eof;
  sox_bool seek_pending;
  uint64_t seek_offset;

  /* Encode buffer: */
  FLAC__int32 * decoded_samples;
  unsigned number_of_samples;

  FLAC__StreamEncoder * encoder;
  FLAC__StreamMetadata * metadata[2];
  unsigned num_metadata;
} priv_t;
#endif

void DecoderSox::seek(double ms)
{
	/*if (finished())
	{
		close();
		open(_path);
	}*/

	// note: sox seems to not reset some internal "progress" counter when sox_seek
	// is called. temporarily opening and closing on seek.
	close();
	open(_path);

	if (!_format->seekable)
		throw(ExceptionDecoderSox("Format isn't seekable."));

	const size_t seek_samples = (ms / 1000.0) * _format->signal.rate;

	if (sox_seek(_format, seek_samples * _format->signal.channels, SOX_SEEK_SET) != 0)
	{
		_progress_samples = lengthSamples();
		_finished = true;
	}
	else
	{
		_progress_samples = seek_samples;
		_finished = false;
	}
}

#endif
