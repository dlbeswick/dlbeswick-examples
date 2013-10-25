#ifndef _MIDIINTERFACE_H
#define _MIDIINTERFACE_H


#include "IMIDIDevice.h"
#include <CoreMIDI/MIDIServices.h>

class MIDIPlatformRef
{
public:
	MIDIPlatformRef(const MIDIEndpointRef& _endpoint) :
		endpoint(_endpoint)
	{
	}
	
	operator const MIDIObjectRef () const { return (MIDIObjectRef)endpoint; }
	operator MIDIEndpointRef () { return endpoint; }
	
	MIDIEndpointRef endpoint;
};

class MIDIDevice : virtual public IMIDIDevice
{
public:
	MIDIDevice(const MIDIPlatformRef& ref);
	virtual ~MIDIDevice();

	virtual std::string name() const;
	
protected:
	MIDIPlatformRef m_ref;
};


#endif