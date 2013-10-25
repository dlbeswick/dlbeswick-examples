#ifndef _DSOUND_EXCEPTIONPORTAUDIO_H
#define _DSOUND_EXCEPTIONPORTAUDIO_H

#include "dsound/common.h"
#include "Standard/ExceptionStream.h"

class ExceptionPortAudio : public ExceptionStream
{
public:
	ExceptionPortAudio(/*PaError*/int error = /*value: paNoError*/0);
};

#endif
