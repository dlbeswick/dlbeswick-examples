#ifndef _MIDIINTERFACE_IMIDIDEVICE_H
#define _MIDIINTERFACE_IMIDIDEVICE_H


#include <string>

class IMIDIDevice
{
public:
	virtual ~IMIDIDevice() {}
	
	virtual std::string name() const = 0;
	
	virtual void close() = 0;
	virtual void open() = 0;
	
	virtual bool isOpen() const = 0;
};


#endif
