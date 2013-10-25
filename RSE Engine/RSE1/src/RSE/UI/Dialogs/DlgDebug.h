// ------------------------------------------------------------------------------------------------
//
// DlgDebug
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/UI/Dialog.h"


class RSE_API DlgDebug : public Dialog
{
	USE_RTTI(DlgDebug, Dialog);
public:
	DlgDebug();

	TPDelegate<class UIMenu&> setupMenu;

	void enableStepping(bool b);

	virtual void update(float delta);

	virtual void keyUp(int key);

	void shuttlePlay();
	void shuttlePause();
	void shuttleForward();
	void shuttleLast();

	void onDialog(const class MenuItemText&);
	void onDebug(const class MenuItemText& i);
	void onExit(const class MenuItemText&);
	void onProfiler(const class MenuItemText&);

	PtrGC<class UIShuttle> m_shuttle;
	PtrGC<class UITextBox> m_stepInfo;

	PtrGC<class UIMenuText> m_pMenu;
	PtrGC<class UIMenuText> m_pDebug;
	PtrGC<class UIMenuText> m_pDialog;

	// fix: move this up to DlgDebugLevel
	class Level* level;
	int debugKey;

protected:
	virtual void construct();
	virtual void onActivate();
	virtual void fillContextMenu(UIMenu& menu);
};