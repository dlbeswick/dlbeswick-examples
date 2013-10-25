#include "pch.h"
#include "MIDIDevice.h"
#include <Standard/ExceptionOS.h>

MIDIDevice::MIDIDevice(const MIDIPlatformRef& ref) :
	m_ref(ref)
{
}

MIDIDevice::~MIDIDevice()
{
}

std::string MIDIDevice::name() const
{
	CFStringRef str;
	osVerify(MIDIObjectGetStringProperty(m_ref, kMIDIPropertyDisplayName, &str));
	
	char buf[256] = "Midi object.";
	CFStringGetCString(str, buf, sizeof(buf), kCFStringEncodingMacRoman);
	
	return buf;
}