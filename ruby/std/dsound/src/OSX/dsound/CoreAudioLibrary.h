#include "Standard/STLHelp.h"
class CoreAudioDevice;

class CoreAudioLibrary
{
public:
	CoreAudioLibrary();

  virtual void close();
	virtual std::vector<CoreAudioDevice*> devices() const;
	virtual void refreshDevices();
	virtual CoreAudioDevice& defaultDevice() const;

protected:
	typedef std::map<std::string, CoreAudioDevice*> Devices;
	Devices m_devices;
};