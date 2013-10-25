//#include "Standard/DeviceNotification.h"
#include "Standard/STLGuid.h"
#include "dsound/SoundLibrary.h"
class DirectSoundDevice;

class DirectSoundLibrary : public SoundLibrary
{
public:
	DirectSoundLibrary(int threadId = 0);

	std::vector<DirectSoundDevice*> devices() const;
	void refreshDevices();
	DirectSoundDevice& defaultDevice() const;

protected:
	static BOOL CALLBACK enumDevice(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext);

	typedef std::map<GUID, DirectSoundDevice*> Devices;
	Devices _devices;

	//TDeviceNotification<DirectSoundLibrary> m_notify;
};
