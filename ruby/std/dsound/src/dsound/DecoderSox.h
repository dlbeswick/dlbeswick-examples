#ifndef DSOUND_DECODERSOX_H
#define DSOUND_DECODERSOX_H

#include "dsound/capabilities.h"

#if WITH_SOX

#include "dsound/Decoder.h"

class ExceptionDecoderSox : public ExceptionDecoder
{
public:
	ExceptionDecoderSox(const std::string& s);
};

class DecoderSox : public DSoundDecoder
{
public:
	DecoderSox(class SoundLibrary& library, const std::string& path);
	~DecoderSox();

	virtual int getPCM(float* out, int samples);
	virtual int getPCM(short* out, int samples);

	virtual bool finished() const;

	virtual double progress() const;
	virtual double lengthMS() const;

	virtual void seek(double ms);

	virtual int rate() const;
	
protected:
	size_t lengthSamples() const;
	void open(const std::string& path);
	void close();

	struct sox_format_t* _format;
	int _decode_buffer_size;
	void* _decode_buffer;
	bool _finished;
	size_t _progress_samples;
	std::string _path;
};

#endif

#endif
