#include "dsound/pch.h"
#include "CoreAudioDevice.h"
#include "CoreAudioLibrary.h"
#include <Standard/Help.h>

void CoreAudioLibrary::close()
{
  for (Devices::iterator i = m_devices.begin(); i != m_devices.end(); ++i)
  {
    if (i->second->opened())
    {
      i->second->close();
    }
  }
}

CoreAudioLibrary::CoreAudioLibrary()
{
	refreshDevices();
}

std::vector<CoreAudioDevice*> CoreAudioLibrary::devices() const
{
	return mapValues(m_devices);
}

void CoreAudioLibrary::refreshDevices()
{
	// tbd: this code will not catch added devices, fix
	if (!m_devices.empty())
		return;

	m_devices.clear();

	UInt32 size;
	osVerify(AudioHardwareGetPropertyInfo(kAudioHardwarePropertyDevices, &size, 0));
	int numDevices = size / sizeof(AudioDeviceID);
	AudioDeviceID* devices = new AudioDeviceID[numDevices];
	osVerify(AudioHardwareGetProperty(kAudioHardwarePropertyDevices, &size, (void*)devices));

	for (int i = 0; i < numDevices; ++i)
	{
		AudioDeviceID id = devices[i];

		CFStringRef name;
		UInt32 size = sizeof(CFStringRef);
		AudioDeviceGetProperty(id, 0, false, kAudioObjectPropertyName, &size, &name);

		char nameCStr[64];
		CFStringGetCString(name, nameCStr, sizeof(nameCStr), kCFStringEncodingUTF8);

		osVerify(AudioDeviceGetPropertyInfo(id, 0, false, kAudioDevicePropertyStreamConfiguration, &size, 0));
		AudioBufferList* streamInfo = (AudioBufferList*)new char[size];
		osVerify(AudioDeviceGetProperty(id, 0, false, kAudioDevicePropertyStreamConfiguration, &size, (void*)streamInfo));

		for (int streamNum = 0; streamNum < streamInfo->mNumberBuffers; ++streamNum)
		{
			std::string name = nameCStr;

			if (streamInfo->mNumberBuffers > 1)
			{
				name += " (channel ";
				name += itostr(streamNum);
				name += ")";
			}

			if (m_devices.find(name) == m_devices.end())
				m_devices[name] = new CoreAudioDevice(id, streamNum, name);
		}

		delete[] streamInfo;

		CFRelease(name);
	}

	delete[] devices;

	if (m_devices.empty())
		throwf("CoreAudioLibrary: No devices found.");
}

CoreAudioDevice& CoreAudioLibrary::defaultDevice() const
{
	return *m_devices.begin()->second;

	throwf("CoreAudioLibrary: No primary device.");
	return *(CoreAudioDevice*)0;
}
