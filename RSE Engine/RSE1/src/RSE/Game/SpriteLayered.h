#ifndef RSE1_SPRITELAYERED_H
#define RSE1_SPRITELAYERED_H

#include "Sprite.h"

class RSE_API SpriteLayered : public Sprite
{
	USE_RTTI(SpriteLayered, Sprite);
public:
	SpriteLayered(TextureAnimationSet& animationSet);

	class TextureAnimatorLayered& animator() const;

protected:
	virtual class ITextureAnimator* createAnimator() const;
};

#endif
