#pragma once
#include <string>
#include "../IMIDIDevice.h"

class MIDIDevice : virtual public IMIDIDevice
{
public:
	MIDIDevice(int id);
	virtual ~MIDIDevice();

	virtual std::string name() const = 0;
	
	int id() const;

	virtual void close() = 0;
	virtual void open() = 0;

protected:
	void error(const std::string& where, int code);

	int m_id;
	int m_handle;
};