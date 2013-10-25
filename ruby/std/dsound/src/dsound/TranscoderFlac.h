#ifndef DSOUND_TRANSCODERFLAC_H
#define DSOUND_TRANSCODERFLAC_H

#include "dsound/capabilities.h"

#if WITH_FLAC

#include "dsound/TranscoderSox.h"

class TranscoderFlac : public TranscoderSox
{
public:
	TranscoderFlac(class SoundLibrary& library, float samples_per_sec, float channels, const char* fName);

protected:
	virtual int encodingValue();
	virtual int outputBitsPerSample();
	virtual double compressionValue();
};

#endif

#endif
