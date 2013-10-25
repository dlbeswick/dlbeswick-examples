#include "Standard/pch.h"
#if IS_WINDOWS && IS_MSVC && 0// disabled, never worked
#include "Standard/Help.h"
#include <dbt.h>
#include <devguid.h>
#include "DeviceNotification.h"

DeviceNotificationBase::DeviceNotificationBase()
{
/*	WNDCLASSEX wndClass;
	zero(wndClass);
	wndClass.lpfnWndProc =
	RegisterClassEx(&wndClass);*/

	m_messageWindow = CreateWindowEx(0, "MESSAGE", 0, 0, 0, 0, 0, 0, HWND_MESSAGE, 0, 0, 0);
	if (!m_messageWindow)
		throwf("DeviceNotificationBase: couldn't create message window.");

    DEV_BROADCAST_DEVICEINTERFACE notificationFilter;

    zero(notificationFilter);
    notificationFilter.dbcc_size =
        sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    notificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;

	// {A5DCBF10-6530-11D2-901F-00C04FB951ED}
	GUID guid = { 0xA5DCBF10, 0x6530, 0x11D2, { 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED } };
    notificationFilter.dbcc_classguid = guid;

	m_notification = RegisterDeviceNotification(m_messageWindow, &notificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
	if (!m_notification)
		throwf("DeviceNotificationBase: couldn't register for notification.");

	m_messageLoopThread.run(TDelegate<DeviceNotificationBase>(this, messageLoop));
}

void DeviceNotificationBase::messageLoop()
{
	MSG msg;
	zero(msg);

	while (GetMessage(&msg, m_messageWindow, 0, 0))
	{
		switch (msg.wParam)
		{
			// use threads to call functions, so system remains responsive
			case DBT_DEVICEARRIVAL:
				new TThread<TDelegate<DeviceNotificationBase> >(TDelegate<DeviceNotificationBase>(this, deviceAddition));
				break;

			case DBT_DEVICEREMOVECOMPLETE:
				new TThread<TDelegate<DeviceNotificationBase> >(TDelegate<DeviceNotificationBase>(this, deviceRemoval));
				break;
		};
	}
}

DeviceNotificationBase::~DeviceNotificationBase()
{
	UnregisterDeviceNotification(m_notification);
}

#endif
