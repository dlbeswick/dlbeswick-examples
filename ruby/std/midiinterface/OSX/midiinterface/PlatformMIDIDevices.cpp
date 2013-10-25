#include "pch.h"
#include "PlatformMIDIDevices.h"
#include "MIDIDeviceIn.h"
#include "MIDIDeviceOut.h"

PlatformMIDIDevices::PlatformMIDIDevices()
{
}

void PlatformMIDIDevices::refresh()
{
	for (int i = 0; i < MIDIGetNumberOfSources(); ++i)
	{
		m_devicesIn.push_back(new MIDIDeviceIn(MIDIGetSource(i)));
	}

	for (int i = 0; i < MIDIGetNumberOfDestinations(); ++i)
	{
		//m_devicesOut.push_back(new MIDIDeviceOut(MIDIGetDestination(i)));
	}
}