#include "pch.h"
#include "SpriteLayered.h"
#include "TextureAnimatorLayered.h"

IMPLEMENT_RTTI(SpriteLayered);

SpriteLayered::SpriteLayered(TextureAnimationSet& animationSet) :
	Sprite(animationSet)
{
}

TextureAnimatorLayered& SpriteLayered::animator() const
{
	return (TextureAnimatorLayered&)Super::animator();
}

ITextureAnimator* SpriteLayered::createAnimator() const
{
	TextureAnimatorLayered* animator = new TextureAnimatorLayered(this, _animationSet, 2);
	return animator;
}
