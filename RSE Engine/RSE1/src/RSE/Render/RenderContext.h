// ------------------------------------------------------------------------------------------------
//
// RenderContext
//
// ------------------------------------------------------------------------------------------------
#pragma once

#include "Render/ID3DResource.h"
#include "Standard/Rect.h"

/// Hold a set of render state that can describe a given context in which rendering operations occur,
/// such as GUI rendering, scene rendering, etc. Wraps the graphics API's "state block" concept and
/// also holds additional relevant render state data such as software rendering flags.
/// State blocks must be destroyed when the device is lost. Render state data is duplicated in this 
/// class so it can be recreated on device reset. If render state is not duplicated by RenderContext
/// then it will not be applied or restored.
class RSE_API RenderContext : public ID3DResource
{
public:
	RenderContext();
	~RenderContext();

	void apply();

protected:
	virtual void release(class SDeviceD3D& device);
	virtual void create(class SDeviceD3D& device);

	IDirect3DStateBlock9* _state;

	bool _created;

	std::vector<D3DLIGHT9> _ls;
	std::map<D3DRENDERSTATETYPE, int> _rs;
	bool _software;
	std::map<D3DSAMPLERSTATETYPE, int> _ss;
	std::map<D3DTRANSFORMSTATETYPE, Matrix4> _xs;
	
	David::Rect _viewport;
};