#ifndef _PANCAKE_BROS_PARALLAX_H
#define _PANCAKE_BROS_PARALLAX_H

#include "Standard/Base.h"
#include "Standard/SmartPtr.h"

class AnimationSet;
class TextureAnimator;
class TextureAnimationSet;

class ParallaxLayer : public Base
{
	USE_RTTI(ParallaxLayer, Base);
public:
	ParallaxLayer();

	virtual void update(float delta, TextureAnimationSet& set);
	virtual void draw(class D3DPainter& painter, TextureAnimationSet& set);

	virtual TextureAnimator& animation(TextureAnimationSet& set);

	float start;
	float end;

protected:
	virtual void onPostRead();
	virtual void streamVars(StreamVars& v);

	float pixelsPerSec;
	std::string sequence;

	float _time;

private:
	SmartPtr<TextureAnimator> _animation;
};

class Parallax : public Base
{
	USE_RTTI(Parallax, Base);
public:
	Parallax(const std::string& category);

	virtual void update(float delta);
	virtual void draw();

protected:
	class TextureAnimationSet* _set;

	typedef std::vector<SmartPtr<ParallaxLayer> > Layers;
	Layers _layers;
	SmartPtr<class D3DPainter> _paint;
};


#endif