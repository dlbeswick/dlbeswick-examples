#include "Standard/Exception.h"
#include "Standard/Delegate.h"

class STANDARD_API DeviceNotificationBase
{
public:
	DeviceNotificationBase();
	~DeviceNotificationBase();

	virtual void deviceAddition() {}
	virtual void deviceRemoval() {}
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
