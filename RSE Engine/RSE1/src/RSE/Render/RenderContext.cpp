// ------------------------------------------------------------------------------------------------
//
// RenderContext
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "RenderContext.h"
#include "Exception/Video.h"
#include <Render/SDeviceD3D.h>

RenderContext::RenderContext() :
	_state(0),
	_created(false)
{
	D3D().addResource(this);
}

void RenderContext::release(class SDeviceD3D& device)
{
	if (_state)
	{
		_state->Release();
		_state = 0;
	}
}

template <class E, class T>
inline E& operator << (ExceptionStream& e, const T& rhs)
{
	e._stream << rhs;
	return e;
}

void RenderContext::create(class SDeviceD3D& device)
{
	IDirect3DDevice9& d = device.device();

	_viewport = device.viewport();

	if (!_created)
	{
		_created = true;

		_software = d.GetSoftwareVertexProcessing() != 0;

		_rs[D3DRS_ALPHABLENDENABLE] = 0;
		_rs[D3DRS_AMBIENT] = 0;
		_rs[D3DRS_CLIPPING] = 0;
		_rs[D3DRS_CLIPPLANEENABLE] = 0;
		_rs[D3DRS_CULLMODE] = 0;
		_rs[D3DRS_FILLMODE] = 0;
		_rs[D3DRS_LIGHTING] = 0;
		_rs[D3DRS_ZENABLE] = 0;
		_rs[D3DRS_ZWRITEENABLE] = 0;

		_ss[D3DSAMP_MAGFILTER] = 0;
		_ss[D3DSAMP_MINFILTER] = 0;
		_ss[D3DSAMP_MIPFILTER] = 0;

		_xs[D3DTS_PROJECTION] = Matrix4();
		_xs[D3DTS_VIEW] = Matrix4();
		_xs[D3DTS_WORLD] = Matrix4();
		_xs[D3DTS_WORLD1] = Matrix4();
		_xs[D3DTS_WORLD2] = Matrix4();
		_xs[D3DTS_WORLD3] = Matrix4();

		for (int i = 0; i < 8; ++i)
		{
			D3DLIGHT9 l;
			memset(&l, 0, sizeof(l));
			BOOL enable = false;
			d.GetLightEnable(i, &enable);

			if (!enable)
			{
				l.Type = (D3DLIGHTTYPE)-1;
			}
			else
			{
				d.GetLight(i, &l);
			}

			_ls.push_back(l);
		}

		for (std::map<D3DRENDERSTATETYPE, int>::iterator i = _rs.begin(); i != _rs.end(); ++i)
		{
			DWORD result;
			d.GetRenderState(i->first, &result);

			i->second = result;
		}

		for (std::map<D3DSAMPLERSTATETYPE, int>::iterator i = _ss.begin(); i != _ss.end(); ++i)
		{
			DWORD result;
			d.GetSamplerState(0, i->first, &result);

			i->second = result;
		}

		for (std::map<D3DTRANSFORMSTATETYPE, Matrix4>::iterator i = _xs.begin(); i != _xs.end(); ++i)
		{
			Matrix4 result;
			d.GetTransform(i->first, result);

			i->second = result;
		}
	}

	DX_ENSURE(d.BeginStateBlock());

	for (std::map<D3DRENDERSTATETYPE, int>::iterator i = _rs.begin(); i != _rs.end(); ++i)
	{
		d.SetRenderState(i->first, i->second);
	}

	for (std::map<D3DSAMPLERSTATETYPE, int>::iterator i = _ss.begin(); i != _ss.end(); ++i)
	{
		d.SetSamplerState(0, i->first, i->second);
	}

	for (std::map<D3DTRANSFORMSTATETYPE, Matrix4>::iterator i = _xs.begin(); i != _xs.end(); ++i)
	{
		d.SetTransform(i->first, i->second);
	}

	for (int i = 0; i < 8; ++i)
	{
		if ((int)_ls[i].Type != -1)
		{
			d.SetLight(i, &_ls[i]);
			d.LightEnable(i, true);
		}
		else
		{
			d.LightEnable(i, false);
		}
	}

	DX_ENSURE(d.EndStateBlock(&_state));

	// Reapply current context. Clients modify render state before construction, so the old render state should be restored.
	if (D3D().context())
		D3D().setContext(*D3D().context(), true);
}

RenderContext::~RenderContext()
{
	D3D().removeResource(this);
}

void RenderContext::apply()
{
	if (_state)
	{
		if (D3D().supportsHardwareVP())
		{
			D3DD().SetSoftwareVertexProcessing(_software);
		}

		DX_ENSURE(_state->Apply());
	}

	D3D().viewport(_viewport);
}
