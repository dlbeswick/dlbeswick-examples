#include "MIDIDeviceJack.h"
#include <Standard/api.h>
#include <Standard/Exception.h>
#include <jack/midiport.h>

MIDIDeviceJack::MIDIDeviceJack(jack_client_t* client, const std::string& port_name) :
	_client(client),
	_port(0),
	_port_name(port_name)
{
}

MIDIDeviceJack::~MIDIDeviceJack()
{
	close();
}

void MIDIDeviceJack::close()
{
	if (_port)
		jack_port_unregister(_client, _port);
}

void MIDIDeviceJack::open()
{
	if (!_port)
		_port = open_jack_port();

	if (!_port)
		throw("Jack failed to initialise port.");
}

std::string MIDIDeviceJack::name() const
{
	return _port_name;
}

bool MIDIDeviceJack::isOpen() const
{
	return _port != 0;
}
