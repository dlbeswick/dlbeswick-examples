#ifndef DSOUND_MP3ENCODER_H
#define DSOUND_MP3ENCODER_H

#include "BufferFormat.h"
#include "Standard/AsyncEvent.h"
#include "Standard/CriticalSection.h"
#include "Standard/Thread.h"
#include <fstream>
#include <ostream>
#include <string>

class Transcoder
{
public:
	virtual ~Transcoder();

	virtual void write(const void* data, int bytes);

	// To be called priod to the transcoder's destructor. The default implementation
	// calls 'close'.
	virtual void destroy();

	virtual void flush() = 0;

protected:
	Transcoder(class SoundLibrary& library, float samples_per_sec, int channels, const char* fName);

	struct TranscodeConsumer : public ThreadFunctor::Functor
	{
		TranscodeConsumer(Transcoder& transcoder);

		void operator () ();

		Transcoder& _encoder;
	};

	virtual void initEncoder() = 0;

	virtual void consume(const void* data, int bytes) = 0;

	// Free any resources being used by the transcoder, subject to possible future
	// re-opening.
	virtual void close() = 0;

	virtual int samplesPerSec() const { return _samples_per_sec; }
	virtual int channels() const { return _channels; }
	virtual int sampleBits() const { return 32; }

	std::string m_path;
	int _samples_per_sec;
	int _channels;
	CriticalSection m_writing;
	CriticalSection _data_buffer_access;
	AsyncEvent _data_available;
	AsyncEvent _exiting;
	ThreadFunctorPtr<TranscodeConsumer> _consumer_thread;
	std::vector<char> _data_buffer;

	bool _transcoder_initialized;
};

#endif
