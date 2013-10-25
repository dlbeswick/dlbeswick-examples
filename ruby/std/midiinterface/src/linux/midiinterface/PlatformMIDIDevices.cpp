#include "PlatformMIDIDevices.h"
#include "MIDIDeviceInJack.h"
#include "MIDIDeviceOutJack.h"
#include "Standard/Exception.h"
#include "Standard/CriticalSectionBlock.h"

int PlatformMIDIDevices::JackProcessCallback(jack_nframes_t nframes, void *arg)
{
	PlatformMIDIDevices& self = *(PlatformMIDIDevices*)arg;

	Critical(*self._criticalMidiDevice);

	// hack, make threadsafe
	if (self._midiDeviceIn && self._midiDeviceIn->isOpen())
		self._midiDeviceIn->jack_callback(nframes);

	return 0;
}

PlatformMIDIDevices::PlatformMIDIDevices(const std::string& client_program_name) :
	MIDIDevices(client_program_name),
	_client(0),
	_criticalMidiDevice(new CriticalSection),
	_midiDeviceIn(0)
{
	jack_status_t result;
	_client = jack_client_open(client_program_name.c_str(), JackNullOption, &result);
	if (!_client)
		throwf("Jack failed to initialise client.");

	if (jack_set_process_callback(_client, &PlatformMIDIDevices::JackProcessCallback, (void*)this) != 0)
		throwf("Jack failed to set process callback.");

	if (jack_activate(_client) != 0)
		throwf("Jack failed to activate.");
}

PlatformMIDIDevices::~PlatformMIDIDevices()
{
	if (_midiDeviceIn)
		delete _midiDeviceIn;

	if (_client)
		jack_client_close(_client);

	delete _criticalMidiDevice;
}

void PlatformMIDIDevices::refresh()
{
	Critical(*_criticalMidiDevice);

	if (!_midiDeviceIn)
	{
		_midiDeviceIn = new MIDIDeviceInJack(_client, "JACK MIDI In");
		m_devicesIn.push_back(_midiDeviceIn);
	}
}
