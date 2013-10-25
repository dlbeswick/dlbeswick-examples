// ------------------------------------------------------------------------------------------------
//
// UITransition
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/UI/UIElement.h"

class TransitionEffect;


class RSE_API UITransition : public UIElement
{
	USE_RTTI(UITransition, UIElement);
public:
	UITransition(TransitionEffect* src, TransitionEffect* dst, bool simultaneous = true, bool destroy = true);
	~UITransition();

	virtual void update(float delta);
	virtual void draw();

	UIElement* srcElement() const;
	UIElement* dstElement() const;

protected:
	TransitionEffect* m_src;
	TransitionEffect* m_dst;
	bool m_simultaneous;
};

class RSE_API TransitionEffect : public Base
{
	USE_RTTI(TransitionEffect, Base);
public:
	enum ETransition
	{
		TRANSITION_IN,
		TRANSITION_OUT
	};

	TransitionEffect(const PtrGC<UIElement>& src, float time);
	~TransitionEffect();

	virtual void init();

	virtual void draw() {};
	virtual void update(float delta);

	virtual bool finished();
	virtual float alpha();
	virtual float duration() { return m_time; }

	ETransition state;
	bool destroyOnFinish;

protected:
	friend class UITransition;

	UITransition* m_owner;
	PtrGC<UIElement> m_src;
	float m_time;
	float m_elapsed;
	bool m_oldInputEnabled;
};

class RSE_API TransitionRenderToTexture : public TransitionEffect
{
	USE_RTTI(TransitionRenderToTexture, TransitionEffect);
public:
	TransitionRenderToTexture(const PtrGC<UIElement>& src, float time, int texSize = 512);
	~TransitionRenderToTexture();

	virtual void init();
	virtual void draw();

	virtual bool finished() { return Super::finished() || !m_tex; }

protected:
	virtual void onRendered() {}

	// if true, renders the src/dst element at each frame
	virtual bool needsPerFrameRenders() { return true; }

	PtrD3D<IDirect3DTexture9> m_tex;
	PtrD3D<IDirect3DSurface9> m_depth;
	bool m_firstDraw;
};