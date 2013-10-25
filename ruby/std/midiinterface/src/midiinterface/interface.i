%{
#undef read
#undef write
#undef close
#undef T_DATA
#include "midiinterface/IMIDIDevice.h"
#include "midiinterface/IMIDIDeviceIn.h"
#include "midiinterface/PlatformMIDIDevices.h"
#include "Standard/api.h"
#include "Standard/Exception.h"
%}

%module midiinterface

%include std_vector.i
%include std_string.i

%template(DevicesInVector) std::vector<IMIDIDeviceIn*>;
%template(DevicesOutVector) std::vector<IMIDIDeviceOut*>;
%template(MIDIDataVector) std::vector<MIDIData>;

%exception {
 try {
   $action
 }
 catch (const Exception& e) {
   rb_raise(rb_eStandardError, e.what());
 }
}

struct MIDIData
{
	MIDIData(int _id=0, int _data1=0, int _data2=0, double _time=0);

	int id;
	int data1;
	int data2;
	double time;
};

// This was causing crashes. Possibly because the first access was to a temporary local variable through 'each' over a vector.
// So the 'tracked object' ended up being garbage collected.
//%trackobjects;

class IMIDIDevice
{
public:
	virtual ~IMIDIDevice() {}
	
	virtual std::string name() const = 0;
	
	virtual void close() = 0;
	virtual void open() = 0;
};

class IMIDIDeviceIn
{
public:
	virtual ~IMIDIDeviceIn() {}
	
	typedef std::vector<MIDIData> Buffer;
	virtual std::string name() const;
	
	virtual Buffer data() = 0;

	virtual void close() = 0;
	virtual void open() = 0;
};

class MIDIDevices
{
public:
	~MIDIDevices();

	const std::vector<class IMIDIDeviceIn*>& devicesIn();
	const std::vector<class IMIDIDeviceOut*>& devicesOut();

protected:
	MIDIDevices();
};

class PlatformMIDIDevices : public MIDIDevices
{
public:
	PlatformMIDIDevices(const std::string& client_program_name);
	virtual void close();
};