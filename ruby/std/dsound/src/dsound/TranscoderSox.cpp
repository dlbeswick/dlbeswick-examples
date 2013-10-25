#include "dsound/TranscoderSox.h"
#include "dsound/SoundLibrary.h"
#include "dsound/common.h"

#if WITH_SOX

#define SOX_BUFFER_OVERFLOW_WORKAROUND 1

ExceptionTranscoderSox::ExceptionTranscoderSox(const std::string& s) :
	ExceptionString(s)
{
}

TranscoderSox::TranscoderSox(class SoundLibrary& library, float samples_per_sec, float channels, const char* fName) :
	Transcoder(library, samples_per_sec, channels, fName),
	_format(0),
	_initialization_iteration(0),
	_conversion_buffer(0),
	_conversion_buffer_size(0)
{
	library.ensureSox();
}

TranscoderSox::~TranscoderSox()
{
	delete[] _conversion_buffer;
}

void TranscoderSox::initEncoder()
{
	++_initialization_iteration;

	sox_signalinfo_t signal;
	signal.rate = samplesPerSec();
	signal.channels = channels();
	signal.precision = SOX_SAMPLE_PRECISION;
	signal.length = SOX_UNSPEC;
	signal.mult = 0;

	sox_encodinginfo_t encoding;
	encoding.encoding = (sox_encoding_t)encodingValue();
	encoding.bits_per_sample = 16;
	encoding.compression = 5;
	encoding.reverse_bytes = sox_option_default;
	encoding.reverse_nibbles = sox_option_default;
	encoding.reverse_bits = sox_option_default;
	encoding.opposite_endian = sox_false;

	// append "(<number of times initialized>)" to the filename if being initialized for a second time.
	std::string output_path = m_path;

	if (_initialization_iteration > 1)
		output_path += std::string("(") + (_initialization_iteration - 1) + ") ";

	output_path += ".flac";

	_format = sox_open_write(output_path.c_str(), &signal, &encoding, 0, 0, 0);

	if (!_format)
		throw(ExceptionTranscoderSox("sox_open_write failed."));

	_conversion_buffer_size = 11026;
	_conversion_buffer = new sox_sample_t[_conversion_buffer_size];
}

void TranscoderSox::consume(const void* data, int bytes)
{
	assert(_format);
	assert(bytes % (sampleBits() / 8 * channels()) == 0);

	const BufferFormat::LocalFormat::SampleElement* typed_data = BufferFormat::Local.buf((void*)data);

	// tbd: update terminology, see BufferFormat.h for reason for division.
	// samples here is total across all channels
	const int bytesPerSample = BufferFormat::Local.bytesPerSample() / channels();
	const int totalSamples = bytes / bytesPerSample;

	SOX_SAMPLE_LOCALS;
	int clips;

	int remainingSamples = totalSamples;
	sox_sample_t* conversion_buffer_it = _conversion_buffer;
	const sox_sample_t* conversion_buffer_end = _conversion_buffer + _conversion_buffer_size;

	while (remainingSamples != 0)
	{
		while (conversion_buffer_it != conversion_buffer_end && remainingSamples != 0)
		{
			*conversion_buffer_it++ = SOX_FLOAT_32BIT_TO_SAMPLE(*typed_data++, clips);
			--remainingSamples;
		}

		sox_sample_t* conversion_buffer_write_it = _conversion_buffer;

		{
			Critical(_sox_access);
			while (conversion_buffer_write_it != conversion_buffer_it)
			{
				int written = 0;

				written = sox_write(
					_format,
					conversion_buffer_write_it,
#if SOX_BUFFER_OVERFLOW_WORKAROUND
					std::min(sox_get_globals()->bufsiz, (size_t)(conversion_buffer_it - conversion_buffer_write_it))
#else
					conversion_buffer_it - conversion_buffer_write_it
#endif
				);

				assert(written > 0);
				if (written == 0)
					break;

				conversion_buffer_write_it += written;
			}
		}

		conversion_buffer_it = _conversion_buffer;
	}
}

void TranscoderSox::close()
{
	Critical(_sox_access);
	sox_close(_format);
	_format = 0;
}

void TranscoderSox::flush()
{
	// I don't think anything can be done here.
}

#endif
