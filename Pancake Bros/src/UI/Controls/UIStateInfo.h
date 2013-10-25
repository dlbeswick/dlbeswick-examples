// ------------------------------------------------------------------------------------------------
//
// UIStateInfo
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/UI/UIElement.h"

class UIStateInfo : public UIElement
{
	USE_RTTI(UIStateInfo, UIElement);
public:
	UIStateInfo(const PtrGC<class Object>& target);

	// style
	UISTYLE
		Point2 infoVelocity;
		float infoTime;
	UISTYLEEND

	virtual void add(const std::string& info, const RGBA& colour, float timeScale = 1.0f);
	virtual void update(float delta);

protected:
	PtrGC<class Object> m_target;
};

class UIStateInfoItem : public UIElement
{
public:
	UIStateInfoItem(const std::string& text, const RGBA& colour, float lifeTime, const Point2& velocity);

	virtual void update(float delta);

protected:
	RGBA m_colour;
	float m_totalLifeTime;
	float m_lifeTime;
	Point2 m_velocity;
	class UILabel* m_text;
};