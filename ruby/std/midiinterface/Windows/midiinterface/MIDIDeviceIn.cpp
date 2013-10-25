#include "MIDIDeviceIn.h"
#include "Standard/api.h"
#include "Standard/CriticalSectionBlock.h"

void CALLBACK MIDIDeviceIn::MidiInProc(
  HMIDIIN hMidiIn,  
  UINT wMsg,        
  DWORD dwInstance, 
  DWORD dwParam1,   
  DWORD dwParam2    
)
{
	MIDIDeviceIn* self = (MIDIDeviceIn*)dwInstance;

	int messageId = dwParam1 & 0x000000FF;
	int messageData1 = (dwParam1 & 0x0000FF00) >> 8;
	int messageData2 = (dwParam1 & 0x000FF0000) >> 16;

	// some kind of init message -- ignore
	if (messageId == 0 && messageData1 == 0 && messageData2 == 0)
		return;

	Critical(*self->m_fillBufferCS);
	self->m_buffer.push_back(MIDIData(messageId, messageData1, messageData2, dwParam2));
}

MIDIDeviceIn::MIDIDeviceIn(int id, MIDIINCAPS& caps) :
	MIDIDevice(id),
	m_caps(caps),
	m_fillBufferCS(new CriticalSection)
{
}

MIDIDeviceIn::~MIDIDeviceIn()
{
	delete m_fillBufferCS;
}

std::string MIDIDeviceIn::name() const 
{
	return m_caps.szPname;
}

void MIDIDeviceIn::close()
{
	int result;
	
	midiInStop((HMIDIIN)m_handle);
	result = midiInClose((HMIDIIN)m_handle);
	if (result != MMSYSERR_NOERROR)
		error("MIDIDeviceIn::close", result);

	m_handle = -1;
}

void MIDIDeviceIn::open()
{
	if (m_handle != -1)
		return;

	MMRESULT result = midiInOpen((HMIDIIN*)&m_handle, id(), (DWORD_PTR)MidiInProc, (DWORD_PTR)this, CALLBACK_FUNCTION);

	if (result != MMSYSERR_NOERROR)
		error("MIDIDeviceIn::open", result);

	result = midiInStart((HMIDIIN)m_handle);
	if (result != MMSYSERR_NOERROR)
		error("MIDIDeviceIn::open midiInStart", result);
}

MIDIDeviceIn::Buffer MIDIDeviceIn::data()
{
	Buffer ret;

	{
		Critical(*m_fillBufferCS);
		ret = m_buffer;
		m_buffer.clear();
	}

	return ret;
}

bool MIDIDeviceIn::isOpen() const
{
	return m_handle != -1;
}