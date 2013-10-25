#include "MIDIDevice.h"
#include <vector>
#define _WIN32_WINNT 0x0400
#include <windows.h>
#include <mmsystem.h>
#include "../IMIDIDeviceIn.h"

class MIDIDeviceIn : public MIDIDevice, public IMIDIDeviceIn
{
public:
	MIDIDeviceIn(int id, MIDIINCAPS& caps);
	~MIDIDeviceIn();

	typedef std::vector<MIDIData> Buffer;

	virtual void close();
	std::string name() const;
	virtual void open();
	virtual Buffer data();
	virtual bool isOpen() const;

protected:
	static void CALLBACK MidiInProc(
		HMIDIIN hMidiIn,  
		UINT wMsg,        
		DWORD dwInstance, 
		DWORD dwParam1,   
		DWORD dwParam2
	);

	MIDIINCAPS m_caps;

	Buffer m_buffer;
	class CriticalSection* m_fillBufferCS;
};