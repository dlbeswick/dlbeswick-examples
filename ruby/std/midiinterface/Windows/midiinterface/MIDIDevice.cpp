#include "MIDIDevice.h"
#define _WIN32_WINNT 0x0400
#include <windows.h>
#include <mmsystem.h>
#include <Standard/api.h>
#include <Standard/Exception.h>

MIDIDevice::MIDIDevice(int id) :
	m_id(id),
	m_handle(-1)
{
}

MIDIDevice::~MIDIDevice()
{
}

int MIDIDevice::id() const
{
	return m_id;
}

void MIDIDevice::error(const std::string& where, int code)
{
	const char* msg;

	switch (code)
	{
	case MMSYSERR_ALLOCATED:
		msg = "The specified resource is already allocated.";
		break;
	case MMSYSERR_BADDEVICEID:
		msg = "The specified device identifier is out of range.";
		break;
	case MMSYSERR_INVALFLAG:
		msg = "The flags specified by dwFlags are invalid.";
		break;
	case MMSYSERR_INVALPARAM:
		msg = "The specified pointer or structure is invalid.";
		break;
	case MMSYSERR_NOMEM:
		msg = "The system is unable to allocate or lock memory.";
		break;
	case MIDIERR_NODEVICE:
		msg = "No MIDI port was found. This error occurs only when the mapper is opened.";
		break;
	case MIDIERR_STILLPLAYING:
		msg = "Buffers are still in the queue.";
		break;
	case MMSYSERR_INVALHANDLE:
		msg = "The specified device handle is invalid.";
		break;
	case MIDIERR_BADOPENMODE:
		msg = "The application sent a message without a status byte to a stream handle.";
		break;
	case MIDIERR_NOTREADY:
		msg = "The hardware is busy with other data.";
		break;
	case MMSYSERR_HANDLEBUSY:
		msg = "Handle being used.";
		break;
	default:
		msg = "Unknown error.";
	}

	throwf(name() + "." + where + ": " + msg);
}
