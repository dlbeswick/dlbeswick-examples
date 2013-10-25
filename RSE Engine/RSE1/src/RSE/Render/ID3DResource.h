// ------------------------------------------------------------------------------------------------
//
// ID3DResource
//
// ------------------------------------------------------------------------------------------------
#pragma once

// Interface for records of resources used by the renderer that must be restored when the device is lost
class RSE_API ID3DResource
{
public:
	virtual void create(class SDeviceD3D& device) = 0;

	/// Release render resources. This is called when the resource is removed from the manager.
	virtual void release(class SDeviceD3D& device) = 0;
};


