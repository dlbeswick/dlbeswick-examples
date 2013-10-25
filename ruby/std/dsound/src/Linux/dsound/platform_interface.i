%include ../../dsound/interface.i

%{
#include "dsound/PortAudioDevice.h"
#include "dsound/PortAudioLibrary.h"
%}

// tbd: just use pointers of sounddevice objects here, the common platform interface should be
// sufficient for all needs
%template(DevicesVector) std::vector<PortAudioDevice*>;

class PortAudioLibrary : public SoundLibrary
{
public:
	PortAudioLibrary(const std::string& client_name);
	
	virtual void close();
	PortAudioDevice& defaultDevice();
	std::vector<PortAudioDevice*> devices() const;
	virtual void open();
	virtual void refreshDevices();

	virtual void setDesiredLatency(float latency);
	virtual float desiredLatency() const;
};


class PortAudioDevice : public SoundDevice
{
public:
	PortAudioDevice(SoundLibrary& library, const PaDeviceInfo& deviceInfo, PaDeviceIndex deviceIndex);

	virtual std::string description() const;
    virtual bool opened() const;
	virtual bool playing() const;
	virtual void restart();
	virtual float samplesPerSec() const;
	virtual void setPlayCursor(int samples);
	virtual int playCursor() const;
	virtual int playCursorMaxSamples() const;
	virtual int writeCursor() const;
};
