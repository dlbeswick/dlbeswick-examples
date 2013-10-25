// ------------------------------------------------------------------------------------------------
//
// UIPancake
//
// ------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "UIPancake.h"
#include "Game/Man.h"
#include "Game/Pancake.h"
#include "Game/PancakeLevel.h"
#include "UI/Controls/UIStateInfo.h"
#include "RSE/Render/Camera.h"
#include "RSE/Render/Scene.h"
#include "RSE/Render/TextureAnimationTypes.h"
#include "RSE/Render/TextureAnimator.h"
#include "RSE/Render/Materials/MaterialAnimation.h"
#include "RSE/UI/Controls/UIPic.h"
#include "Standard/Config.h"
#include "Standard/Interpolation.h"

IMPLEMENT_RTTI(UIPancake)

UIPancake::UIPancake(UIStateInfo* info, Pancake* pancake, float duration, TextureAnimationSet& set, const Point2& startPoint, const Point2& startSize, const Point2& destPoint, const Point2& destSize) :
	m_startSize(startSize),
	m_destSize(destSize),
	m_t(0),
	m_duration(duration),
	m_pancake(pancake),
	m_info(info)
{
	m_startPoint = startPoint;
	Point2 midPoint = Interpolation::linear(startPoint, destPoint, 0.5f);
	midPoint.y -= 0.25f;
	m_destPoint = destPoint;

	m_parabola = new Interpolation::Parabola(Point3(startPoint.x, 0, startPoint.y), Point3(midPoint.x, 0, midPoint.y), Point3(destPoint.x, 0, destPoint.y));

	m_picMaterial = new MaterialAnimation(set.name, "headsquashedget");
	m_pic = new UIPic(*m_picMaterial/*material("Window Border")*/);
	addChild(m_pic);
	setPos(startPoint);
	setSize(startSize);
	m_pic->setSize(startSize);

	BEGIN_STATEMACHINE;
	ADD_STATE(Get);
	ADD_STATE(Give);
	ADD_STATE(Given);

	states.go(Get);
}

UIPancake::~UIPancake()
{
	delete m_parabola;
	delete m_picMaterial;
	delete m_pancake;
}

void UIPancake::update(float delta)
{
	Super::update(delta);

	states.update(delta);
}

void UIPancake::give(const PtrGC<Man>& target)
{
	_target = target;
	states.go(Give);
}

////

void UIPancake::StateGet::update(float delta)
{
	o->m_t = clamp(o->m_t + delta, 0.0f, o->m_duration);
	float u = o->m_t / o->m_duration;

	Point3 newPoint = (*o->m_parabola)(u);
	o->setPos(Point2(newPoint.x, newPoint.z));

	o->setSize(Interpolation::exp(o->m_startSize, o->m_destSize, u));

	o->m_pic->setSize(o->size());

	if (o->m_t == o->m_duration)
	{
		o->states.go(o->states.null());
	}
}

////

void UIPancake::StateGive::start()
{
	o->m_t = 0;
	Point2 screenPoint = o->_target->level().scene().camera().worldToScreen(o->_target->worldPos());

	Point2 midPoint = Interpolation::linear(o->m_destPoint, screenPoint, 0.5f);
	midPoint.y -= 0.25f;

	o->m_parabola->recalculate(Point3(o->m_destPoint.x, 0, o->m_destPoint.y), Point3(midPoint.x, 0, midPoint.y), Point3(screenPoint.x, 0, screenPoint.y));

	o->m_picMaterial->animator().play("headsquashedgivelaunch");
}

void UIPancake::StateGive::update(float delta)
{
	if (o->m_t == o->m_duration)
	{
		o->states.go(o->Given);
		return;
	}

	//Point2 screenPoint = target->level().scene().camera().worldToScreen(target->worldPos());
	//o->m_parabola->recalculate(Point3(o->m_destPoint.x, 0, o->m_destPoint.y), Point3(o->m_midPoint.x, 0, o->m_midPoint.y), Point3(screenPoint.x, 0, screenPoint.y));

	o->m_t = clamp(o->m_t + delta, 0.0f, o->m_duration);
	float u = o->m_t / o->m_duration;

	Point3 newPoint = (*o->m_parabola)(u);
	o->setPos(Point2(newPoint.x, newPoint.z));
}

void UIPancake::StateGiven::start()
{
	std::string infoString = o->m_pancake->giveTo(o->_target);
	o->m_info->add(infoString, RGBA(1, 1, 1));
	o->m_picMaterial->animator().play("headsquashedgiven");
}

void UIPancake::StateGiven::update(float delta)
{
	if (o->m_picMaterial->animator().finished())
		o->self().destroy();
}
