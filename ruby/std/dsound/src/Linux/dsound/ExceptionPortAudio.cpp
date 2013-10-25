#include "dsound/pch.h"
#include "dsound/ExceptionPortAudio.h"
#include <portaudio.h>

ExceptionPortAudio::ExceptionPortAudio(PaError error)
{
	if (error != paNoError)
		addContext(Pa_GetErrorText(error));
}
