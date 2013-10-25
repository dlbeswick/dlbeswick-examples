#include "PlatformMIDIDevices.h"
#include "MIDIDeviceIn.h"
#include "MIDIDeviceOut.h"

PlatformMIDIDevices::PlatformMIDIDevices(const std::string& client_program_name) :
	MIDIDevices(client_program_name)
{
}

void PlatformMIDIDevices::refresh()
{
	for (int i = 0; ; ++i)
	{
		MIDIOUTCAPS caps;
		if (midiOutGetDevCaps(i, &caps, sizeof(caps)) == MMSYSERR_NOERROR)
		{
			m_devicesOut.push_back(new MIDIDeviceOut(i, caps));
		}
		else
		{
			break;
		}
	}
		
	for (int i = 0; ; ++i)
	{
		MIDIINCAPS caps;
		if (midiInGetDevCaps(i, &caps, sizeof(caps)) == MMSYSERR_NOERROR)
		{
			m_devicesIn.push_back(new MIDIDeviceIn(i, caps));
		}
		else
		{
			break;
		}
	}
}
