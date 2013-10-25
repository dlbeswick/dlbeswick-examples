// ------------------------------------------------------------------------------------------------
//
// PancakeSprite
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/Game/SpriteLayered.h"

class PancakeSprite : public SpriteLayered
{
	USE_RTTI(PancakeSprite, SpriteLayered);
public:
	PancakeSprite(TextureAnimationSet& animationSet);

	virtual void draw();
	class PancakeLevel& level() const;
	virtual void setShadow(bool b);

protected:
	bool _bShadow;
};