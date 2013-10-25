// ------------------------------------------------------------------------------------------------
//
// UIStateInfo
//
// ------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "UIStateInfo.h"
#include "RSE/Game/Object.h"
#include "RSE/Game/Level.h"
#include "RSE/Render/Camera.h"
#include "RSE/Render/Scene.h"
#include "RSE/UI/Controls/UITextBox.h"
#include "Standard/Interpolation.h"

IMPLEMENT_RTTI(UIStateInfo)

UISTYLESTREAM(UIStateInfo)
	STREAMVAR(infoVelocity);
	STREAMVAR(infoTime);
UISTYLESTREAMEND

UIStateInfo::UIStateInfo(const PtrGC<class Object>& target)
{
	m_target = target;
}

void UIStateInfo::add(const std::string& info, const RGBA& colour, float timeScale)
{
	addChild(new UIStateInfoItem(info, colour, style().infoTime * timeScale, style().infoVelocity / timeScale));
}

void UIStateInfo::update(float delta)
{
	Super::update(delta);

	if (!m_target)
		return;

	setPos(m_target->level().scene().camera().worldToScreen(m_target->worldPos()));
}

////

UIStateInfoItem::UIStateInfoItem(const std::string& text, const RGBA& colour, float lifeTime, const Point2& velocity) :
	m_lifeTime(lifeTime),
	m_totalLifeTime(lifeTime),
	m_velocity(velocity),
	m_colour(colour)
{
	setInheritClipping(false);
	m_text = addChild(new UILabel(text));
	m_text->setPos(Point2(-m_text->size().x * 0.5f, 0.0f));
	m_text->setTextColour(m_colour);
	m_text->setInheritClipping(false);
	fitToChildren();
}

void UIStateInfoItem::update(float delta)
{
	Super::update(delta);

	m_lifeTime -= delta;
	if (m_lifeTime < 0)
	{
		self().destroy();
		return;
	}

	setPos(pos() + m_velocity * delta);
	m_text->setTextColour(RGBA(m_colour.r, m_colour.g, m_colour.b, Interpolation::exp(m_colour.a, 0.0f, 1.0f - (m_lifeTime / m_totalLifeTime))));
}