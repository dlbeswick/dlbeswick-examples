#include "MIDIDevice.h"

class MIDIDeviceOut : public MIDIDevice
{
public:
	MIDIDeviceOut(const MIDIEndpointRef& endpoint);
};