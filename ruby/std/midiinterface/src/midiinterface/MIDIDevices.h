#ifndef _MIDIINTERFACE_MIDIDEVICES_H
#define _MIDIINTERFACE_MIDIDEVICES_H


#include <string>
#include <vector>

class MIDIDevices
{
public:
	MIDIDevices(const std::string& client_program_name);
	virtual ~MIDIDevices();

	const std::vector<class IMIDIDeviceIn*>& devicesIn();
	const std::vector<class IMIDIDeviceOut*>& devicesOut();

	virtual void close();

protected:
	virtual void refresh() = 0;

	std::vector<class IMIDIDeviceIn*> m_devicesIn;
	std::vector<class IMIDIDeviceOut*> m_devicesOut;
	
	mutable bool m_init;
};

#endif
