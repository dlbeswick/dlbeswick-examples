%include ../../dsound/interface.i
%{
#include "dsound/CoreAudioLibrary.h"
#include "dsound/CoreAudioDevice.h"
%}

%template(CoreAudioDeviceVector) std::vector<CoreAudioDevice*>;

class CoreAudioLibrary
{
public:
	CoreAudioLibrary();

  virtual void close();
	virtual std::vector<CoreAudioDevice*> devices() const;
	virtual void refreshDevices();
	virtual CoreAudioDevice& defaultDevice() const;
};

class CoreAudioDevice : public SoundDevice
{
public:
	CoreAudioDevice(Component component, const std::string& description);
	~CoreAudioDevice();

	bool isDefault() const;
	virtual bool opened() const;

	const std::string& description() const { return m_description; }
	
	void play();
	void stop();
};