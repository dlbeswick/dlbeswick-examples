// ------------------------------------------------------------------------------------------------
//
// MaterialTextureBase
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "MaterialTextureBase.h"
#include "Render/SDeviceD3D.h"

IMPLEMENT_RTTI(MaterialTextureBase);

#define METADATA MaterialTextureBase
STREAM
	STREAMVAR(uvOrigin);
	STREAMVAR(uvScale);
	STREAMVAR(wrap);
}

#undef METADATA

MaterialTextureBase::MaterialTextureBase(
	const Point2& _uvOrigin,
	const Point2& _uvScale,
	const Point2i& _wrap,
	const RGBA& _diffuse,
	const RGBA& _specular,
	const RGBA& _ambient,
	const RGBA& _emissive,
	float _power
	) :
	MaterialSolid(
		_diffuse,
		_specular,
		_ambient,
		_emissive,
		_power
		),
	uvScale(_uvScale),
	uvOrigin(_uvOrigin),
	wrap(_wrap)
{
}

void MaterialTextureBase::apply(const PtrD3D<IDirect3DTexture9>& texture, const Point2& _uvOrigin, const Point2& _uvScale) const
{
	applySolidProperties();

	// fix, test should not be required
	if (texture)
		D3DD().SetTexture(0, texture.ptr());
	else
		D3DD().SetTexture(0, 0);

	if (texture && (_uvOrigin != Point2::ZERO || _uvScale != Point2(1,1)))
	{
		Matrix4 mat(Matrix4::IDENTITY);
		mat.scale(Point3(_uvScale.x, _uvScale.y, 1.0f));
		mat(2, 0) = _uvOrigin.x;
		mat(2, 1) = _uvOrigin.y;
		D3DD().SetTransform(D3DTS_TEXTURE0, mat);
		D3DD().SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
	}
	else
	{
		D3DD().SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
	}

	if (wrap.x)
		D3DD().SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	else
		D3DD().SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);

	if (wrap.y)
		D3DD().SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	else
		D3DD().SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
}