#include "dsound/pch.h"
#include "dsound/PortAudioDevice.h"
#include "dsound/ExceptionPortAudio.h"
#include "dsound/SoundLibrary.h"

PortAudioDevice::PortAudioDevice(SoundLibrary& library, const PaDeviceInfo& deviceInfo, PaDeviceIndex deviceIndex) :
	SoundDevice(library),
	_deviceInfo(deviceInfo),
	_deviceIndex(deviceIndex),
	_stream(0),
	_playing(false)
{
}

std::string PortAudioDevice::description() const
{
	return _deviceInfo.name;
}

bool PortAudioDevice::opened() const
{
	return _stream != 0;
}

bool PortAudioDevice::playing() const
{
	if (_stream == 0)
		return false;

	return Pa_IsStreamActive(_stream);
}

void PortAudioDevice::restart()
{
	stop();
	play();
}

float PortAudioDevice::samplesPerSec() const
{
	return 44100;
}

void PortAudioDevice::setPlayCursor(int samples)
{
}

int PortAudioDevice::playCursor() const
{
	return 0;
}

int PortAudioDevice::playCursorMaxSamples() const
{
	return (int)bufferSize();
}

int PortAudioDevice::writeCursor() const
{
	return 0;
}

void PortAudioDevice::close()
{
	if (_stream)
	{
		PaError result = Pa_StopStream(_stream);
		if (result != paNoError && result != paStreamIsStopped)
			EXCEPTIONSTREAM(ExceptionPortAudio(result), "Error while stopping PortAudio stream.");

		result = Pa_CloseStream(_stream);

		if (result == paNoError)
			_stream = 0;
		else
			EXCEPTIONSTREAM(ExceptionPortAudio(result), "Error while closing PortAudio stream.");
	}
}

double PortAudioDevice::latency() const
{
	if (opened())
		return Pa_GetStreamInfo(_stream)->outputLatency;
	else
		return 0.0;
}

int PortAudioDevice::callback(
		const void *input,
		void *output,
		unsigned long frameCount,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags,
		void *userData)
{
	PortAudioDevice& self = *(PortAudioDevice*)userData;

    int requestedOutputFrames = frameCount;

    bool outputProvided = self.consumeTo((float*)output, requestedOutputFrames);

	if (!outputProvided)
	{
		memset(output, 0, sizeof(float) * frameCount * 2);
	}

	return 0;
}

uint PortAudioDevice::bufferSize() const
{
	return latency() * samplesPerSec();
}

void PortAudioDevice::open()
{
	PaStreamParameters outputParams;
	outputParams.device = _deviceIndex;
	outputParams.channelCount = 2;
	outputParams.sampleFormat = paFloat32;
	outputParams.suggestedLatency = _library.desiredLatency();
	outputParams.hostApiSpecificStreamInfo = 0;

	dlog << "Opening PortAudio stream for '" << description() << "' with requested latency of " << (float)outputParams.suggestedLatency << dlog.endl;

	PaError result = Pa_OpenStream(
		&_stream,
		0,
		&outputParams,
		samplesPerSec(),
		paFramesPerBufferUnspecified,
		paNoFlag,
		&PortAudioDevice::callback,
		(void*)this
	);

	if (result != paNoError)
		EXCEPTIONSTREAM(ExceptionPortAudio(result), "Error while opening PortAudio stream.");
}

void PortAudioDevice::play()
{
	if (_stream)
	{
		PaError result = Pa_StartStream(_stream);
		if (result != paNoError)
			EXCEPTIONSTREAM(ExceptionPortAudio(result), "Error while starting PortAudio stream.");

		dlog << "PortAudio stream playing, latency is " << (float)latency() << dlog.endl;
	}
}

void PortAudioDevice::stop()
{
	if (_stream)
	{
		PaError result = Pa_StopStream(_stream);
		if (result != paNoError)
			EXCEPTIONSTREAM(ExceptionPortAudio(result), "Error while stopping PortAudio stream.");
	}
}
