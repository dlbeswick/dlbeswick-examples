%include ../../dsound/interface.i

%{
#include <windows.h>
#include "dsound/DirectSoundDevice.h"
#include "dsound/DirectSoundLibrary.h"
%}

%template(DevicesVector) std::vector<DirectSoundDevice*>;

class DirectSoundLibrary
{
public:
	DirectSoundLibrary(int threadId = 0);

	std::vector<DirectSoundDevice*> devices() const;
	DirectSoundDevice& defaultDevice() const;
	
	void refreshDevices();
};

class DirectSoundDevice : public SoundDevice
{
public:
	DirectSoundDevice(class SoundLibrary& library, GUID guid, const std::string& description);
	~DirectSoundDevice();

	bool isDefault() const;
	std::string description() const;

	void setWindow(int window);

// SoundDevice
	virtual bool opened() const;

	virtual bool playing() const;

	virtual void restart();
	
	virtual void setPlayCursor(int bytes);

	virtual int playCursor() const;
	virtual int writeCursor() const;

	virtual float samplesPerSec() const;
};
