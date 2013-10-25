#include "pch.h"
#include "MIDIDeviceIn.h"
#include <Foundation/NSAutoreleasePool.h>
#include <Foundation/NSString.h>

MIDIDeviceIn::MIDIDeviceIn(const MIDIPlatformRef& endpoint) :
	MIDIDevice(endpoint),
	_client(0),
	_open(false),
	_port(0)
{
}

void MIDIDeviceIn::open()
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	
	if (_open)
		throwf("Device already open.");
	
	_open = true;
	
	// create client and ports
	NSMutableString* nameStr = [[NSMutableString alloc] initWithString:@"MIDIDeviceIn_Client_"];
	[nameStr appendString:[NSString stringWithUTF8String:name().c_str()]];
	
	osVerify(MIDIClientCreate((CFStringRef)nameStr, NULL, NULL, &_client));
	
	nameStr = [[NSMutableString alloc] initWithString:@"MIDIDeviceIn_InputPort_"];
	[nameStr appendString:[NSString stringWithUTF8String:name().c_str()]];
	
	osVerify(MIDIInputPortCreate(_client, (CFStringRef)nameStr, &MIDIDeviceIn::readProc, NULL, &_port));
	
	osVerify(MIDIPortConnectSource(_port, m_ref, (void*)this));

	[pool release];
}

void MIDIDeviceIn::readProc(const MIDIPacketList* packetList, void* readProcRefCon, void* srcConnRefCon)
{
	MIDIDeviceIn* self = (MIDIDeviceIn*)srcConnRefCon;
	
	const MIDIPacket* packet = &packetList->packet[0];
	for (int i = 0; i < packetList->numPackets; ++i) 
	{
		const Byte* pd = packet->data;
		
		if (pd[0] >= 0xB0 && pd[0] <= 0xBF)
		{
			MIDIData data;
			data.id = pd[0];
			data.data1 = pd[1];
			data.data2 = pd[2];
			data.time = packet->timeStamp;
			
			Critical(self->_csFillBuffer);
			self->_data.push_back(data);
		}

		packet = MIDIPacketNext(packet);
	}
}

MIDIDeviceIn::Buffer MIDIDeviceIn::data() 
{ 
	Buffer ret;
	
	{
		Critical(_csFillBuffer);
		ret = _data;
		_data.clear();
	}
	
	return ret;
}

void MIDIDeviceIn::close()
{
	if (!_open)
		throwf("Device is not open.");
	
	osVerify(MIDIPortDisconnectSource(_port, m_ref));
	
	osVerify(MIDIPortDispose(_port));
	_port = 0;
	
	osVerify(MIDIClientDispose(_client));
	_client = 0;
	
	_open = false;
}

bool MIDIDeviceIn::isOpen() const
{
    return _open;
}
