// ------------------------------------------------------------------------------------------------
//
// Sprite
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "Sprite.h"
#include "AppBase.h"
#include <Game/Level.h>
#include <Render/D3DPainter.h>
#include <Render/SFont.h>
#include <Render/FontElement.h>
#include <Render/TextureAnimator.h>
#include <Render/TextureAnimationManager.h>

IMPLEMENT_RTTI(Sprite);

const Point2 PIXELS_PER_UNIT(1.0f, 1.0f);

#define METADATA Sprite
STREAM
	STREAMVAR(drawOffset);
}

Sprite::Sprite(TextureAnimationSet& animationSet) :
	_animationSet(animationSet),
	_animator(0),
	_destroyOnAnimFinish(false),
	_drawScale(1),
	drawOffset(0,0,0)
{
}

Sprite::~Sprite()
{
	delete _animator;
}

void Sprite::draw()
{
	const TextureAnimationFrame& f = animator().frame();
	
	Quad q;
	generateWorldVerts(q, drawOffset);
	drawSprite(f, q, RGBA(1,1,1));

	Super::draw();
}

void Sprite::update(float delta)
{
	if (_destroyOnAnimFinish && animator().finished())
	{
		PtrGC<Object>(this).destroy();
		return;
	}
	animator().update(delta);
	
	Object::update(delta);
}

void Sprite::generateLocalVerts(Quad& q)
{
	const TextureAnimationFrame& f = animator().frame();

	Point3 newSize(f.size().x, 0, f.size().y);
	PtrD3D<IDirect3DTexture9> tex = f.texture;

	newSize *= _drawScale * 0.5f;

	q[0] = Point3(-newSize.x, 0, newSize.z);
	q[1] = Point3(newSize.x, 0, newSize.z);
	q[2] = Point3(newSize.x, 0, -newSize.z);
	q[3] = Point3(-newSize.x, 0, -newSize.z);
}

void Sprite::generateWorldVerts(Quad& q, const Point3& drawOffset)
{
	generateLocalVerts(q);

	Quat rot = worldRot();
	Point3 pos = worldPos();

	for (int i = 0; i < 4; ++i)
		q[i] = q[i] * rot + pos + drawOffset;
}

void Sprite::drawSprite(const TextureAnimationFrame& f, const Quad& q, const RGBA& colour)
{
	D3DPaint().reset();

	if (!f.isNullFrame())
	{
		D3DPaint().setFill(colour, f.texture);
		D3DPaint().setNormal(Point3(0, -1, 0));
		D3DPaint().uv0 = f.uvMin;
		D3DPaint().uv1 = f.uvMax;
		D3DPaint().quad(q);
	}
	else
	{
		Font().get("worldarial")->worldWrite(level().scene().camera(), D3DPaint(), animator().curSequenceName(), worldPos());
	}

	D3DPaint().draw();
}

Point3 Sprite::size() const
{
	return Point3(animator().frame().size().x, animator().frame().size().x, animator().frame().size().y) * 512.0f;
}

ITextureAnimator& Sprite::animator() const
{
	if (!_animator)
		_animator = createAnimator();

	return *_animator;
}

ITextureAnimator* Sprite::createAnimator() const
{
	return new SpriteFXAnimator((Sprite*)this, _animationSet);
}

void Sprite::destroyOnAnimFinish() 
{ 
	_destroyOnAnimFinish = true;
}

void Sprite::setDrawOffset(const Point3& offset) 
{ 
	drawOffset = offset; 
}
