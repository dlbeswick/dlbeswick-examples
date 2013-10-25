// ------------------------------------------------------------------------------------------------
//
// UIShuttle
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/UI/UIElement.h"

class UIButton;

class RSE_API IShuttleNotify : public Base, virtual public IPtrGCHost
{
public:
	virtual bool shuttleIsPlaying() = 0;
};

class RSE_API UIShuttle : public UIElement
{
	USE_RTTI(UIShuttle, UIElement);
public:
	UIShuttle(bool defaultPlaying = false, const PtrGC<IShuttleNotify>& notify = 0);

	Delegate onFirst;
	Delegate onBack;
	Delegate onPlay;
	Delegate onPause;
	Delegate onForward;
	Delegate onLast;

	virtual void setSize(const Point2& reqSize);
	void update(float delta);

protected:
	void internalOnFirst();
	void internalOnBack();
	void internalOnPlay();
	void internalOnPause();
	void internalOnForward();
	void internalOnLast();

	void onButton();

	void updateButtons(bool playing);

	UIButton* m_first;
	UIButton* m_back;
	UIButton* m_play;
	UIButton* m_pause;
	UIButton* m_forward;
	UIButton* m_last;

	PtrGC<IShuttleNotify> m_notify;
};