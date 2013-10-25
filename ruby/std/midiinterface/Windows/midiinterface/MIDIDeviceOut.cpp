#include "MIDIDeviceOut.h"

MIDIDeviceOut::MIDIDeviceOut(int id, MIDIOUTCAPS& caps) :
	MIDIDevice(id),
	m_caps(caps)
{
	memset(&m_hdr, 0, sizeof(MIDIHDR));
	m_hdr.dwFlags = MHDR_DONE;
}

void MIDIDeviceOut::close()
{
	int result;
	
	result = midiOutClose((HMIDIOUT)m_handle);
	if (result != MMSYSERR_NOERROR)
		error("MIDIDeviceOut::close", result);

	m_handle = -1;
}

std::string MIDIDeviceOut::name() const 
{
	return m_caps.szPname;
}

void MIDIDeviceOut::open()
{
	MMRESULT result = midiOutOpen((HMIDIOUT*)&m_handle, id(), 0, 0, 0);

	if (result != MMSYSERR_NOERROR)
		error("MIDIDeviceOut::open", result);
}

void MIDIDeviceOut::shortMsgOut(int status, int byte1, int byte2)
{
	DWORD data = 0;
	data = status;
	data |= byte1 << 8;
	data |= byte2 << 16;
	data &= 0x00FFFFFF;

	MMRESULT result = midiOutShortMsg((HMIDIOUT)m_handle, data);
	if (result != MMSYSERR_NOERROR)
		error("MIDIDeviceOut::shortMsgOut", result);
}

void MIDIDeviceOut::longMsgOut(const char* data, int size)
{
	m_hdr.dwBufferLength = size;
	m_hdr.dwBytesRecorded = size;
	m_hdr.lpData = (char*)data;

	MMRESULT result;

	result = midiOutPrepareHeader((HMIDIOUT)m_handle, &m_hdr, sizeof(m_hdr));
	if (result != MMSYSERR_NOERROR)
		error("MIDIDeviceOut::longMsgOut (prepare)", result);

	result = midiOutLongMsg((HMIDIOUT)m_handle, &m_hdr, sizeof(m_hdr));
	if (result != MMSYSERR_NOERROR)
		error("MIDIDeviceOut::longMsgOut", result);

	result = midiOutUnprepareHeader((HMIDIOUT)m_handle, &m_hdr, sizeof(m_hdr));
	if (result != MMSYSERR_NOERROR)
		error("MIDIDeviceOut::longMsgOut (unprepare)", result);
}

bool MIDIDeviceOut::isOpen() const
{
	return m_handle != -1;
}