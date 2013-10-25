// ------------------------------------------------------------------------------------------------
//
// MaterialTexture
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "MaterialTexture.h"
#include "PathResource.h"
#include "Render/SDeviceD3D.h"
#include <Standard/TGA.h>

#define METADATA MaterialTexture

REGISTER_RTTI(MaterialTexture)

STREAM
	STREAMVAR(textureResource);
	STREAMVAR(texturePath);
}

#undef METADATA 

MaterialTexture::MaterialTexture(
	const PtrD3D<IDirect3DTexture9>& _texture,
	const Point2& _uvOrigin,
	const Point2& _uvScale,
	const Point2i& _wrap,
	const RGBA& _diffuse,
	const RGBA& _specular,
	const RGBA& _ambient,
	const RGBA& _emissive,
	float _power
	) :
	MaterialTextureBase(
		_uvOrigin,
		_uvScale,
		_wrap,
		_diffuse,
		_specular,
		_ambient,
		_emissive,
		_power
		),
	texture(_texture),
	_sourceDimensions(0, 0)
{
}

void MaterialTexture::apply() const
{
	Super::apply(texture, uvOrigin, uvScale);
}

const Point2i& MaterialTexture::sourceDimensions() const
{
	return _sourceDimensions;
}

void MaterialTexture::onPostRead()
{
	Super::onPostRead();

	// tbd: store image size from resource load
	if (!textureResource.empty())
	{
		texture = D3D().loadTextureFromResource(textureResource, "TGA");
	}
	else
	{
		// tbd: support all image formats
		/*TGA tga;
		tga.read(texturePath, ibinstream(PathResource(texturePath).open(std::ios::in | std::ios::binary)));
		
		_sourceDimensions = tga.size();

		texture = D3D().loadTexture(tga);*/
		texture = D3D().loadTexture(PathResource(texturePath));
	}
}