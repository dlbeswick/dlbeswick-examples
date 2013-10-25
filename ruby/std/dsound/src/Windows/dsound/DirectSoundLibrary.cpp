#include "dsound/pch.h"
#include "DirectSoundDevice.h"
#include "DirectSoundLibrary.h"

DirectSoundLibrary::DirectSoundLibrary(int threadId) :
	SoundLibrary("DirectSound Client")
{
//	m_notify.onDeviceAdded = TDelegate<DirectSoundLibrary>(this, refreshDevices);
//	m_notify.onDeviceRemoved = TDelegate<DirectSoundLibrary>(this, refreshDevices);
	refreshDevices();
}

std::vector<DirectSoundDevice*> DirectSoundLibrary::devices() const
{
	return mapValues(_devices);
}

BOOL DirectSoundLibrary::enumDevice(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext)
{
	if (!lpGuid)
		return true;

	DirectSoundLibrary& self = *(DirectSoundLibrary*)lpContext;

	Devices::iterator i = self._devices.find(*lpGuid);

	if (i == self._devices.end())
		self._devices[*lpGuid] = new DirectSoundDevice(self, *lpGuid, lpcstrDescription);

	return true;
}

void DirectSoundLibrary::refreshDevices()
{
	DirectSoundEnumerate(enumDevice, (void*)this);

	if (_devices.empty())
		throwf("DirectSoundLibrary: No devices found.");
}

DirectSoundDevice& DirectSoundLibrary::defaultDevice() const
{
	for (Devices::const_iterator i = _devices.begin(); i != _devices.end(); ++i)
	{
		if (i->second->isDefault())
			return *i->second;
	}

	throwf("DirectSoundLibrary: No primary device.");
	return *(DirectSoundDevice*)0;
}

