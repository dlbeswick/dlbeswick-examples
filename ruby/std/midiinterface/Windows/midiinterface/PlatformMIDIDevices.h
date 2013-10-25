#ifndef MIDIINTERFACE_PLATFORMMIDIDEVICES_H
#define MIDIINTERFACE_PLATFORMMIDIDEVICES_H

#include <vector>
#include "../MIDIDevices.h"

class PlatformMIDIDevices : public MIDIDevices
{
public:
	PlatformMIDIDevices(const std::string& client_program_name);

protected:
	virtual void refresh();
};

#endif
