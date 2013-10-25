#include "pch.h"
#include "TextureAnimationTypes.h"
#include "SDeviceD3D.h"

AnimationFrame& TextureAnimationSequence::nullFrame() const
{
	TextureAnimationFrame* f = new TextureAnimationFrame;
	f->texture = D3D().nullTexture();;
	return *f;
}

AnimationSequence* TextureAnimationSet::nullSequence()
{ 
	return new TextureAnimationSequence; 
}

bool TextureAnimationFrame::isNullFrame() const
{
	return texture == D3D().nullTexture();
}
