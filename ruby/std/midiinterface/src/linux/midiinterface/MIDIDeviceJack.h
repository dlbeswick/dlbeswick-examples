#ifndef MIDIINTERFACE_MIDIDEVICEJACK_H
#define MIDIINTERFACE_MIDIDEVICEJACK_H

#include "midiinterface/IMIDIDevice.h"
#include <string>
#include <jack/jack.h>

class MIDIDeviceJack : virtual public IMIDIDevice
{
public:
	MIDIDeviceJack(jack_client_t* client, const std::string& port_name);
	virtual ~MIDIDeviceJack();

	virtual void jack_callback(jack_nframes_t frames) = 0;

	virtual std::string name() const;
	
	virtual void close();
	virtual void open();
	virtual bool isOpen() const;

protected:
	virtual jack_port_t* open_jack_port() = 0;

	jack_client_t* _client;
	jack_port_t* _port;
	std::string _port_name;

private:
	friend class PlatformMIDIDevices;
};

#endif
