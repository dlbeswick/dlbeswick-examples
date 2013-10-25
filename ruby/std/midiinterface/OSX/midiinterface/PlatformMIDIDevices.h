#include "MIDIDevices.h"

class PlatformMIDIDevices : public MIDIDevices
{
public:
	PlatformMIDIDevices();
	
protected:
	virtual void refresh();
};