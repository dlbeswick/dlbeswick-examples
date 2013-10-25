#ifndef DSOUND_TRANSCODERSOX_H
#define DSOUND_TRANSCODERSOX_H

#include "dsound/capabilities.h"
#include "Standard/CriticalSection.h"

#if WITH_SOX

#include <sox.h>
#include "dsound/MP3Encoder.h"

class ExceptionTranscoderSox : public ExceptionString
{
public:
	ExceptionTranscoderSox(const std::string& s);
};

class TranscoderSox : public Transcoder
{
public:
	TranscoderSox(class SoundLibrary& library, float samples_per_sec, float channels, const char* fName);
	virtual ~TranscoderSox();

	virtual void flush();

protected:
	// Should return sox_encoding_t
	virtual int encodingValue() = 0;

	virtual int outputBitsPerSample() = 0;
	virtual double compressionValue() = 0;

	virtual void initEncoder();
	virtual void consume(const void* data, int bytes);
	virtual void close();

	CriticalSection _sox_access;
	struct sox_format_t* _format;
	sox_sample_t* _conversion_buffer;
	size_t _conversion_buffer_size;
	int _initialization_iteration;
};

#endif

#endif
