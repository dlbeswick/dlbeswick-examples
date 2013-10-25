// ------------------------------------------------------------------------------------------------
//
// MaterialAnimation
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "MaterialAnimation.h"
#include "AppBase.h"
#include "Render/SDeviceD3D.h"
#include "Render/TextureAnimationManager.h"
#include "Render/TextureAnimator.h"
#include <Standard/Mapping.h>

#define METADATA MaterialAnimation

REGISTER_RTTI(MaterialAnimation)

STREAM
	STREAMVARNAME(set, setName);
	STREAMVARNAME(sequence, sequenceName);
}

#undef METADATA

MaterialAnimation::MaterialAnimation(
	const std::string& _set,
	const std::string& _sequence,
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
	m_animator(0)
{
	if (!_set.empty())
	{
		animator().useSet(AppBase().textureAnimation().set(_set));
		animator().play(_sequence, true);
	}
}

void MaterialAnimation::apply() const
{
	const TextureAnimationFrame& f = animator().frameAt(AppBase().timer());
	
	// apply texture transformation such that (0,0) is start of animation frame, and (1,1) is end
	Point2 scale = f.uvMax - f.uvMin;
	scale.componentMul(uvScale);
	Super::apply(
		f.texture, 
		uvOrigin + f.uvMin, 
		scale
		);
}

void MaterialAnimation::onPostRead()
{
	Super::onPostRead();

	animator().useSet(AppBase().textureAnimation().set(setName));
	animator().play(sequenceName, true);
}

TextureAnimator& MaterialAnimation::animator()
{
	if (!m_animator)
		m_animator = new TextureAnimator(AppBase().textureAnimation().set(""));

	return (TextureAnimator&)*m_animator;
}

const TextureAnimator& MaterialAnimation::animator() const
{
	if (!m_animator)
		m_animator = new TextureAnimator(AppBase().textureAnimation().set(""));

	return *m_animator;
}
