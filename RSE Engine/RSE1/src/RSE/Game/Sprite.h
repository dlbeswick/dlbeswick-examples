// ------------------------------------------------------------------------------------------------
//
// Sprite
// A 2D (billboarded) texture-animated entity
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/Game/Object.h"

class TextureAnimator;
class TextureAnimationSet;
class TextureAnimationFrame;
class SpriteFXPlayer;


class RSE_API Sprite : public Object
{
	USE_RTTI(Sprite, Object);
	USE_STREAMING;
public:
	Sprite(TextureAnimationSet& animationSet);
	~Sprite();

	virtual void draw();
	virtual void update(float delta);
	virtual Point3 size() const;
	
	class ITextureAnimator& animator() const;

	virtual void destroyOnAnimFinish();
	virtual void setDrawOffset(const Point3& offset);
	virtual void setDrawScale(float f) { _drawScale = f; }
	virtual float drawScale() const { return _drawScale; }

protected:
	void drawSprite(const class TextureAnimationFrame& f, const Quad& q, const RGBA& colour);
	
	void generateLocalVerts(Quad& q);
	void generateWorldVerts(Quad& q, const Point3& drawOffset);

	virtual class ITextureAnimator* createAnimator() const;

	TextureAnimationSet& _animationSet;
	bool _destroyOnAnimFinish;
	Point3 drawOffset;
	float _drawScale;

private:
	mutable class ITextureAnimator* _animator;
};