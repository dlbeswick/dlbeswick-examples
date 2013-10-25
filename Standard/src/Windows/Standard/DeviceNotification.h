#include "Standard/api.h"
#if IS_WINDOWS && IS_MSVC && 0 // disabled, never worked
#include "Standard/Exception.h"
#include "Standard/MultiDelegate.h"
#include "Standard/Thread.h"

class STANDARD_API DeviceNotificationBase
{
public:
	DeviceNotificationBase();
	virtual ~DeviceNotificationBase();

	virtual void deviceAddition() {}
	virtual void deviceRemoval() {}

protected:
	HWND m_messageWindow;
	HANDLE m_notification;

	void messageLoop();

	TThread<TDelegate<DeviceNotificationBase> > m_messageLoopThread;
};

template <class BaseClass>
class TDeviceNotification : public DeviceNotificationBase
{
public:
	typedef TDelegate<BaseClass> DelegateType;

	TDeviceNotification()
	{}

	DelegateType onDeviceAdded;
	DelegateType onDeviceRemoved;

protected:
	void deviceAddition()
	{
		onDeviceAdded();
	}

	void deviceRemoval()
	{
		onDeviceRemoved();
	}
};

typedef TDeviceNotification<Base> DeviceNotification;
#endif
