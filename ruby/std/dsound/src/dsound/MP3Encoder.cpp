#include "dsound/pch.h"
#include "MP3Encoder.h"

#include "BufferFormat.h"

Transcoder::TranscodeConsumer::TranscodeConsumer(Transcoder& encoder) :
	_encoder(encoder)
{}

void Transcoder::TranscodeConsumer::operator () ()
{
	std::vector<char> buffer_copy;

	while (!exiting())
	{
		AsyncWait(_encoder._data_available);

		if (!_encoder._transcoder_initialized)
		{
			_encoder.initEncoder();
			_encoder._transcoder_initialized = true;
		}

		{
			Critical(_encoder._data_buffer_access);
			buffer_copy = _encoder._data_buffer;
			_encoder._data_buffer.clear();
		}

		if (!buffer_copy.empty())
		{
			_encoder.consume(&buffer_copy.front(), buffer_copy.size());
			buffer_copy.clear();
		}
	}
}

Transcoder::Transcoder(class SoundLibrary& library, float samples_per_sec, int channels, const char* fName) :
	m_path(fName),
	_channels(channels),
	_samples_per_sec(samples_per_sec),
	_transcoder_initialized(false)
{
	const int sample_bytes = sizeof(float) * 2;
	const int samples_per_second = 44100;

	// reserve roughly half a second of audio data.
	_data_buffer.reserve((sample_bytes * samples_per_second) / 2);

	_consumer_thread.run(TranscodeConsumer(*this));
}

Transcoder::~Transcoder()
{
	_exiting.signal();
	_data_available.signal();

	if (_consumer_thread.alive())
		_consumer_thread->stop();
}

void Transcoder::destroy()
{
	close();
}

void Transcoder::write(const void* data, int bytes)
{
	bool do_signal = false;

	{
		Critical(_data_buffer_access);
		// check that the buffer isn't getting too big. drop samples if so, to avoid endless memory consumption.
		if (_data_buffer.size() > 1048576 * 10)
		{
			assert(false);
		}
		else
		{
			_data_buffer.insert(_data_buffer.end(), (const char*)data, (const char*)data + bytes);
		}

		do_signal = _data_buffer.size() >= 32768;
	}

	if (do_signal)
		_data_available.signal();
}
