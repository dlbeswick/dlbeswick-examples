#include "midiinterface/MIDIDevices.h"
#include <jack/jack.h>
#include <string>
#include <vector>

class PlatformMIDIDevices : public MIDIDevices
{
public:
	PlatformMIDIDevices(const std::string& client_program_name);
	~PlatformMIDIDevices();

	void open(const std::string& client_name);

protected:
	static int JackProcessCallback(jack_nframes_t nframes, void *arg);

	virtual void refresh();

	jack_client_t* _client;
	class MIDIDeviceInJack* _midiDeviceIn;
	class CriticalSection* _criticalMidiDevice;
};
