#ifndef DSOUND_DECODER_H
#define DSOUND_DECODER_H

#include <Standard/Exception.h>

class ExceptionDecoder : public ExceptionString
{
public:
	ExceptionDecoder(const std::string& s) : ExceptionString(s) {}
};

class DSoundDecoder
{
public:
	DSoundDecoder(class SoundLibrary& library) {}
	virtual ~DSoundDecoder() {}

	// Should return the number of samples retrieved.
	virtual int getPCM(float* out, int samples) = 0;
	virtual int getPCM(short* out, int samples) = 0;

	virtual bool finished() const = 0;

	virtual double progress() const = 0;
	virtual double lengthMS() const = 0;

	virtual void seek(double ms) = 0;

	virtual int rate() const = 0;
};

#endif
