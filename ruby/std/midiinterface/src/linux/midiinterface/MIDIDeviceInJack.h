#ifndef MIDIINTERFACE_MIDIDEVICEINJACK_H
#define MIDIINTERFACE_MIDIDEVICEINJACK_H

#include "midiinterface/MIDIDeviceJack.h"
#include "midiinterface/IMIDIDeviceIn.h"
#include <string>
#include <vector>

class MIDIDeviceInJack : public MIDIDeviceJack, public IMIDIDeviceIn
{
public:
	MIDIDeviceInJack(jack_client_t* client, const std::string& port_name);
	~MIDIDeviceInJack();

	typedef std::vector<MIDIData> Buffer;

	virtual void jack_callback(jack_nframes_t frames);

	virtual Buffer data();

protected:
	friend class PlatformMIDIDevicesJack;

	virtual jack_port_t* open_jack_port();

	Buffer m_buffer;
	class CriticalSection* m_fillBufferCS;
};

#endif
