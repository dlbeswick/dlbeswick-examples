#include "MIDIDevice.h"
#include "IMIDIDeviceIn.h"
#include "Standard/CriticalSection.h"

class MIDIDeviceIn : public MIDIDevice, public IMIDIDeviceIn
{
public:
	MIDIDeviceIn(const MIDIPlatformRef& ref);

	virtual void close();
	virtual Buffer data();
	virtual bool isOpen() const;
	virtual void open();
	
protected:
	static void readProc(const MIDIPacketList* packetList, void* readProcRefCon, void* srcConnRefCon);
  	
	CriticalSection _csFillBuffer;
	MIDIClientRef _client;
	Buffer _data;
	bool _open;
	MIDIPortRef _port;
};