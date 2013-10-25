// ------------------------------------------------------------------------------------------------
//
// MaterialTextureBase
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "MaterialSolid.h"
#include <Standard/PtrD3D.h>

class RSE_API MaterialTextureBase : public MaterialSolid
{
	USE_RTTI(MaterialTextureBase, MaterialSolid);
	USE_STREAMING
public:
	Point2 uvOrigin;
	Point2 uvScale;
	Point2i wrap;

	MaterialTextureBase(
		const Point2& _uvOrigin = Point2(0, 0),
		const Point2& _uvScale = Point2(1, 1),
		const Point2i& _wrap = Point2i(0, 0),
		const RGBA& _diffuse = RGBA(1,1,1),
		const RGBA& _specular = RGBA(1,1,1),
		const RGBA& _ambient = RGBA(0,0,0,0),
		const RGBA& _emissive = RGBA(0,0,0,0),
		float _power = 1
		);

protected:
	virtual void apply(const PtrD3D<IDirect3DTexture9>& texture, const Point2& _uvOrigin, const Point2& _uvScale) const;
};