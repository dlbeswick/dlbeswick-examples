#include "dsound/pch.h"
#include "dsound/PortAudioLibrary.h"
#include "dsound/PortAudioDevice.h"
#include "dsound/ExceptionPortAudio.h"
#include <portaudio.h>
#include "pa_jack.h"

PortAudioLibrary::PortAudioLibrary(const std::string& client_name) :
	SoundLibrary(client_name),
	_open(false),
	_desiredLatency(0.05f),
	_client_name(client_name)
{
	PaJack_SetClientName(_client_name.c_str());
}

void PortAudioLibrary::close()
{
  if (_open)
	{
		PaError result = Pa_Terminate();
		if (result != paNoError)
			EXCEPTIONSTREAM(ExceptionPortAudio(result), "Error while closing PortAudio library.");

		_open = false;
		}
}

void PortAudioLibrary::open()
{
	//dlog << "Opening PortAudio: " << Pa_GetVersionText() << dlog.endl;

	PaError result = Pa_Initialize();
	if (result != paNoError)
		EXCEPTIONSTREAM(ExceptionPortAudio(result), "Error while opening PortAudio library.");

	_open = true;
}

PortAudioDevice& PortAudioLibrary::defaultDevice()
{
	std::vector<PortAudioDevice*> devices = this->devices();

	if (devices.empty())
		EXCEPTIONSTREAM(ExceptionPortAudio(), "Error while getting default device (no devices available).");

	// tbd: take account of addition/removal of sound devices
	PaDeviceIndex defaultDevice = Pa_GetDefaultOutputDevice();

	if (defaultDevice >= 0 && defaultDevice < devices.size())
		return *devices[defaultDevice];
	else
		return *devices.front();
}

std::vector<PortAudioDevice*> PortAudioLibrary::devices() const
{
	if (_devices.empty())
		const_cast<PortAudioLibrary*>(this)->refreshDevices();

	return _devices;
}

void PortAudioLibrary::refreshDevices()
{
	if (!_open)
		EXCEPTIONSTREAM(ExceptionPortAudio(0), "Library hasn't been opened.");

	// tbd: take account of addition/removal of sound devices
	if (!_devices.empty())
		return;

	int numDevices;

	numDevices = Pa_GetDeviceCount();
	if (numDevices < 0)
	{
		EXCEPTIONSTREAM(ExceptionPortAudio(numDevices), "Error while getting device count.");
	}

	for (int i = 0; i < numDevices; i++)
	{
		const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i);

		if (deviceInfo)
			_devices.push_back(new PortAudioDevice(*this, *deviceInfo, (PaDeviceIndex)i));
	}
}

void PortAudioLibrary::setDesiredLatency(float latency)
{
	_desiredLatency = latency;
}

float PortAudioLibrary::desiredLatency() const
{
	return _desiredLatency;
}
