// ------------------------------------------------------------------------------------------------
//
// PancakeSprite
//
// ------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "PancakeSprite.h"
#include "PancakeLevel.h"
#include "RSE/Game/TextureAnimatorLayered.h"
#include "RSE/Render/D3DPainter.h"
#include "Standard/Collide.h"

IMPLEMENT_RTTI(PancakeSprite);

PancakeSprite::PancakeSprite(TextureAnimationSet& animationSet) :
	SpriteLayered(animationSet),
	_bShadow(true)
{
}

PancakeLevel& PancakeSprite::level() const 
{ 
	return Super::level().cast<PancakeLevel>(); 
}

void PancakeSprite::draw()
{
	const TextureAnimationFrame& f = animator().frame();
	
	Quad q;
	generateWorldVerts(q, drawOffset);

	// draw shadow
	// don't draw shadow if below world min
	if (_bShadow && worldPos().z >= level().boundMin().z)
	{
		Quad shadowQ;

		if (level().projectedShadows())
		{
			Plane floor(Point3(0, 0, 1), Point3(0, 0, level().boundMin().z));

			for (int i = 0; i < 4; ++i)
			{
				Collide::rayPlane(level().sunPosition(), (q[i] - level().sunPosition()).normal(), floor, shadowQ[i]);
			}
		}
		else
		{
			generateLocalVerts(shadowQ);
			
			for (int i = 0; i < 4; ++i)
			{
				shadowQ[i] *= worldRot();

				float shadowOffset = size().z / 2;

				shadowQ[i] += Point3(0, 0, shadowOffset);
				std::swap(shadowQ[i].y, shadowQ[i].z);
				shadowQ[i].y *= level().fakeShadowScaleY();
				shadowQ[i].x += level().fakeShadowSkew().x * shadowQ[i].y;
				shadowQ[i].y += level().fakeShadowSkew().y * shadowQ[i].x;
				shadowQ[i] += Point3(worldPos().x, worldPos().y, level().boundMin().z);
			}
		}

		drawSprite(f, shadowQ, level().shadowColour());
	}

	drawSprite(f, q, RGBA(1,1,1));

	// don't call super, children are drawn in pancakelevel
}

void PancakeSprite::setShadow(bool b) 
{ 
	_bShadow = b; 
}

