// ------------------------------------------------------------------------------------------------
//
// MaterialTexture
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "MaterialTextureBase.h"
#include <Standard/PtrD3D.h>

class RSE_API MaterialTexture : public MaterialTextureBase
{
	USE_RTTI(MaterialTexture, MaterialTextureBase);
	USE_STREAMING;

public:
	PtrD3D<IDirect3DTexture9> texture;

	MaterialTexture(
		const PtrD3D<IDirect3DTexture9>& _texture = 0,
		const Point2& _uvOrigin = Point2(0, 0),
		const Point2& _uvScale = Point2(1, 1),
		const Point2i& _wrap = Point2i(0, 0),
		const RGBA& _diffuse = RGBA(1,1,1),
		const RGBA& _specular = RGBA(1,1,1),
		const RGBA& _ambient = RGBA(0,0,0,0),
		const RGBA& _emissive = RGBA(0,0,0,0),
		float _power = 1
		);

	virtual void apply() const;

	const Point2i& sourceDimensions() const;

protected:
	std::string texturePath;
	std::string textureResource;
	Point2i		_sourceDimensions;

	virtual void onPostRead();
private:
};