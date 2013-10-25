// ------------------------------------------------------------------------------------------------
//
// DlgDebug
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "AppBase.h"
#include "DlgDebug.h"
#include "Game/Level.h"
#include <Standard/Config.h>
#include "Standard/DXInput.h"
#include "Standard/ThreadStepper.h"
#include "UI/DialogMgr.h"
#include "UI/Controls/UIMenu.h"
#include "UI/Controls/UIShuttle.h"
#include "UI/Controls/UITextBox.h"
#include "UI/Dialogs/DlgStandard.h"

IMPLEMENT_RTTI(DlgDebug);

DlgDebug::DlgDebug()
{
}

void DlgDebug::construct()
{
	Super::construct();

	setFont("smallarial");

	UITextBox* box = addChild(new UITextBox("Debug Dialog (Press Esc to toggle)"));
	box->setInputEnabled(false);
	box->setAlign(UITextBox::CENTER);
	box->setSize(Point2(1, 0.05f));
	box->setColour(RGBA(0,0.5f,0));

	m_pMenu = addChild(new UIMenuText);
	m_pMenu->setVisible(false);

	m_shuttle = addChild(new UIShuttle(true));
	m_shuttle->onPlay = delegate(&DlgDebug::shuttlePlay);
	m_shuttle->onPause = delegate(&DlgDebug::shuttlePause);
	m_shuttle->onForward = delegate(&DlgDebug::shuttleForward);
	m_shuttle->onLast = delegate(&DlgDebug::shuttleLast);
	m_shuttle->setSize(Point2(0.2f, 0.05f));
	m_shuttle->setPos(Point2(0, 0.75f - m_shuttle->size().y));

	m_stepInfo = addChild(new UITextBox);
	m_stepInfo->setInputEnabled(false);
	m_stepInfo->setSize(Point2(0.4f, 0.025f));
	m_stepInfo->setPos(Point2(0, 0.75f - m_shuttle->size().y - m_stepInfo->size().y));
	m_stepInfo->setColour(RGBA(0,0,0,0.5f));

	setContextMenu(new UIMenuText);
}

void DlgDebug::onActivate()
{
	Super::onActivate();
	// tbd: what should probably happen here is that a lack of focus is noticed and new focus should be found.
	// that should happen at a most basic class level.
	setFocus();
}

void DlgDebug::fillContextMenu(UIMenu& menu)
{
	menu.setTitle("System Menu");

	setupMenu(menu);

	// make debug menu
	PtrGC<UIMenu> expander;
	expander = menu.addExpander(new MenuItemText("Debug"));

	const Config::Categories& cats = AppBase().options().categories();
	Config::Categories::const_iterator debugIt = std::find(cats.begin(), cats.end(), "Debug");
	if (debugIt != cats.end())
	{
		const Config::Vars& vars = debugIt->vars;
		for (Config::Vars::const_iterator i = vars.begin(); i != vars.end(); i++)
		{
			MenuItemText* m = new MenuItemText((*i)->name, delegate(&DlgDebug::onDebug));
			m->setChecked(AppBase().options().get<bool>("Debug", (*i)->name));
			expander->addItem(m);
		}
	}

	// dialogs
	expander = menu.addExpander(new MenuItemText("Dialog"));

	for (RegistrantsName::const_iterator i = registrants().begin(); i != registrants().end(); ++i)
	{
		const Registrant& r = *i->second;
		if (r.rtti.isA(DlgStandard::static_rtti()))
			expander->addItem(new MenuItemText(r.name, r.rtti.className(), delegate(&DlgDebug::onDialog)));
	}

	// profiler
	menu.addItem(new MenuItemText("Profiler", delegate(&DlgDebug::onProfiler)));

	// exit
	menu.addItem(new MenuItemText("Quit Game", delegate(&DlgDebug::onExit)));
}

// onDialog
void DlgDebug::onDialog(const MenuItemText& i)
{
	DialogMgr().removeAll();
	DialogMgr().add(i.data());
}

// onDebug
void DlgDebug::onDebug(const MenuItemText&)
{
	const MenuItemText &selected = *Cast<MenuItemText>(contextMenu()->selectedItem());

	if (!selected.text().empty())
		AppBase().options().set("Debug", selected.text(), !AppBase().options().get<bool>("Debug", selected.text()));
}

// onExit
void DlgDebug::onExit(const MenuItemText&)
{
	AppBase().exit();
}

void DlgDebug::onProfiler(const MenuItemText&)
{
	DialogMgr().add("DlgProfile");
}

void DlgDebug::shuttlePlay()
{
	//level->time().setScale(1.0);
	level->time().setFrameClamp(0.2);
	AppBase().mainLoopStep().resume();
}

void DlgDebug::shuttlePause()
{
	//level->time().setScale(0.0);
	level->time().setFrameClamp(0.01);
	AppBase().mainLoopStep().pause();
}

void DlgDebug::shuttleForward()
{
	/*if (Input().key(VK_CONTROL))
		level->time().advance(0.1f);
	else
		level->time().advance(0.01f);*/
	AppBase().mainLoopStep().step();
}

void DlgDebug::shuttleLast()
{
	AppBase().mainLoopStep().stepToStart();
}

void DlgDebug::keyUp(int key)
{
	Super::keyUp(key);

	if (key == debugKey)
		setVisible(false);
}

void DlgDebug::update(float delta)
{
	Super::update(delta);

	if (m_stepInfo->visible())
	{
		if (AppBase().mainLoopStep().current())
			m_stepInfo->setText(AppBase().mainLoopStep().current());
		else
			m_stepInfo->setText("Frame Stepper");
	}
}

void DlgDebug::enableStepping(bool b)
{
	m_shuttle->setVisible(b);
	m_stepInfo->setVisible(b);
}
