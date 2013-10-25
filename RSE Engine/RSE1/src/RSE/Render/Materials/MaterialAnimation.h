// ------------------------------------------------------------------------------------------------
//
// MaterialAnimation
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "MaterialTexture.h"
#include <Standard/PtrD3D.h>

class RSE_API MaterialAnimation : public MaterialTextureBase
{
	USE_RTTI(MaterialAnimation, MaterialTextureBase);
	USE_STREAMING
public:
	MaterialAnimation(
		const std::string& _set = "",
		const std::string& _sequence = "",
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
	class TextureAnimator& animator();
	const class TextureAnimator& animator() const;

protected:
	std::string setName;
	std::string sequenceName;

	virtual void onPostRead();

private:
	mutable const class TextureAnimator* m_animator;
};