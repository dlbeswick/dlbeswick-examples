#include "MIDIDevices.h"
#include "IMIDIDeviceIn.h"
#include "IMIDIDeviceOut.h"

MIDIDevices::MIDIDevices(const std::string& client_program_name) :
	m_init(false)
{
}

MIDIDevices::~MIDIDevices()
{
}

void MIDIDevices::close()
{
	for (int i = 0; i < (int)m_devicesIn.size(); ++i)
	{
		if (m_devicesIn[i]->isOpen())
			m_devicesIn[i]->close();
		delete m_devicesIn[i];
	}

	for (int i = 0; i < (int)m_devicesOut.size(); ++i)
	{
		if (m_devicesOut[i]->isOpen())
			m_devicesOut[i]->close();
		delete m_devicesOut[i];
	}
}

const std::vector<class IMIDIDeviceIn*>& MIDIDevices::devicesIn()
{
	if (!m_init)
	{
		m_init = true;
		refresh();
	}
	
	return m_devicesIn; 
}

const std::vector<class IMIDIDeviceOut*>& MIDIDevices::devicesOut()
{ 
	if (!m_init)
	{
		m_init = true;
		refresh();
	}
	
	return m_devicesOut; 
}
