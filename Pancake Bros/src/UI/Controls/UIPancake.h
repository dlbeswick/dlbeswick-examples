// ------------------------------------------------------------------------------------------------
//
// UIPancake
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/UI/UIElement.h"
#include "Standard/StateMachine.h"

class Pancake;
class Man;
class TextureAnimationSet;
namespace Interpolation
{
	template <class T> class TParabola;
};

class UIPancake : public UIElement
{
	USE_RTTI(UIPancake, UIElement);

public:
	UIPancake(class UIStateInfo* info, Pancake* pancake, float duration, TextureAnimationSet& set, const Point2& startPoint, const Point2& startSize, const Point2& destPoint, const Point2& destSize);
	~UIPancake();

	virtual void update(float delta);

	virtual void give(const PtrGC<Man>& target);

protected:
	DECLARE_STATECLASS(UIPancake);
	DECLARE_STATEMACHINE;

	STATE(Get)
	{
		void update(float delta);
	};

	STATE(Give)
	{
		void start();
		void update(float delta);
	};

	STATE(Given)
	{
		void start();
		void update(float delta);
	};

	class UIPic* m_pic;
	class MaterialAnimation* m_picMaterial;
	Interpolation::TParabola<float>* m_parabola;
	Point2 m_startSize;
	Point2 m_destSize;
	PtrGC<Man> _target;
	float m_t;
	float m_duration;
	Point2 m_startPoint;
	Point2 m_destPoint;
	Pancake* m_pancake;
	class UIStateInfo* m_info;
};