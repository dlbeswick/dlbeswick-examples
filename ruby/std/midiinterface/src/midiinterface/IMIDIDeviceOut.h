#ifndef _MIDIINTERFACE_IMIDIDEVICEOUT_H
#define _MIDIINTERFACE_IMIDIDEVICEOUT_H

#include "midiinterface/IMIDIDevice.h"

class IMIDIDeviceOut : virtual public IMIDIDevice
{
public:
	virtual void shortMsgOut(int status, int byte1, int byte2) = 0;
	virtual void longMsgOut(const char* data, int size) = 0;
};

#endif
