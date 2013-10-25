#include "midiinterface/IMIDIDeviceOut.h"
#include "MIDIDevice.h"
#include <vector>
#define _WIN32_WINNT 0x0400
#include <windows.h>
#include <mmsystem.h>

class MIDIDeviceOut : public MIDIDevice, public IMIDIDeviceOut
{
public:
	MIDIDeviceOut(int id, MIDIOUTCAPS& caps);

	virtual void close();
	std::string name() const;
	virtual void open();
	virtual bool isOpen() const;

	virtual void shortMsgOut(int status, int byte1, int byte2);
	virtual void longMsgOut(const char* data, int size);

protected:
	MIDIOUTCAPS m_caps;
	MIDIHDR m_hdr;
};