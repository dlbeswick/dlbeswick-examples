// ------------------------------------------------------------------------------------------------
//
// Parallax
//
// ------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "Parallax.h"
#include "RSE/AppBase.h"
#include "RSE/Render/D3DPainter.h"
#include "RSE/Render/SDeviceD3D.h"
#include "RSE/Render/TextureAnimator.h"
#include "RSE/Render/TextureAnimationManager.h"

IMPLEMENT_RTTI(Parallax)
REGISTER_RTTI(ParallaxLayer)

Parallax::Parallax(const std::string& category)
{
	setConfig(AppBase().config("parallax.ini"), category);

	_set = &AppBase().textureAnimation().set(get<std::string>("animationSet"));

	get("layers", _layers);

	float y = 0;

	for (uint i = 0; i < _layers.size(); ++i)
	{
		float height = _layers[i]->animation(*_set).frame(0).size().y * 512.0f;

		_layers[i]->start = y;
		_layers[i]->end = y + height;

		y += height;
	}

	_paint = new D3DPainter;
}

void Parallax::update(float delta)
{
	for (uint i = 0; i < _layers.size(); ++i)
	{
		_layers[i]->update(delta, *_set);
	}
}

void Parallax::draw()
{
	D3D().reset();
	D3D().xformPixel();
	D3D().texFilter(false, false);
	D3D().zbuffer(false);

/*		D3DLIGHT9 l;
		zero(l);
		l.Type = D3DLIGHT_DIRECTIONAL;
		l.Diffuse = RGBA(1,1,1,1);
		l.Specular = RGBA(1,1,1,1);
		l.Direction.z = 1;
		l.Ambient = RGBA(1,1,1,1);
		D3DD().SetLight(0, &l);
		D3DD().LightEnable( 0, TRUE);*/

	for (uint i = 0; i < _layers.size(); ++i)
	{
		_layers[i]->draw(*_paint, *_set);
	}
}

////

ParallaxLayer::ParallaxLayer() :
	_time(0.0f)
{
}

void ParallaxLayer::streamVars(StreamVars& v)
{
	STREAMVAR(pixelsPerSec);
	STREAMVAR(sequence);
}

void ParallaxLayer::onPostRead()
{
}

void ParallaxLayer::update(float delta, TextureAnimationSet& set)
{
	animation(set).update(delta);
	_time += delta;
}

TextureAnimator& ParallaxLayer::animation(TextureAnimationSet& set)
{
	if (!_animation)
	{
		_animation = new TextureAnimator(set);
		_animation->play(sequence, true);
	}

	return *_animation;
}

void ParallaxLayer::draw(class D3DPainter& painter, TextureAnimationSet& set)
{
	const TextureAnimationFrame& frame = animation(set).frame();

	painter.setFill(RGBA(1,1,1), frame.texture, true);
	painter.uv0 = frame.uvMin;
	painter.uv1 = frame.uvMax;

	float spriteWidth = frame.pixels().x;

	float x = -spriteWidth + floor(fmod(_time * pixelsPerSec, spriteWidth));
	for (; x < (float)D3D().screenSize().x; x += spriteWidth)
	{
		painter.quad2D(x, start, x + spriteWidth, end);
	}

	painter.draw();
}
