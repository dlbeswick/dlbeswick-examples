#ifndef _MIDIINTERFACE_IMIDIDEVICEIN_H
#define _MIDIINTERFACE_IMIDIDEVICEIN_H

#include <vector>
#include "midiinterface/IMIDIDevice.h"

struct MIDIData
{
	MIDIData(int _id=0, int _data1=0, int _data2=0, double _time=0) :
	id(_id), data1(_data1), data2(_data2), time(_time)
{}
	
	int id;
	int data1;
	int data2;
	double time;
};

class IMIDIDeviceIn : virtual public IMIDIDevice
{
public:
	virtual ~IMIDIDeviceIn() {}
	
	typedef std::vector<MIDIData> Buffer;
	
	virtual Buffer data() = 0;
};

#endif
