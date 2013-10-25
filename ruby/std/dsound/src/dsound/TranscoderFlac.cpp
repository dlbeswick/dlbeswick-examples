#include "dsound/TranscoderFlac.h"
#include "dsound/common.h"

#if WITH_FLAC

TranscoderFlac::TranscoderFlac(class SoundLibrary& library, float samples_per_sec, float channels, const char* fName) :
	TranscoderSox(library, samples_per_sec, channels, fName)
{
}

int TranscoderFlac::encodingValue()
{
	return SOX_ENCODING_FLAC;
}

int TranscoderFlac::outputBitsPerSample()
{
	return 16;
}

double TranscoderFlac::compressionValue()
{
	return 5;
}

#endif
