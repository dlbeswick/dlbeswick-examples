// ------------------------------------------------------------------------------------------------
//
// DlgGameHUD
//
// ------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "DlgGameHUD.h"
#include "DlgGame.h"
#include "RSEApp.h"
#include "Game/Game.h"
#include "Game/Man.h"
#include "Game/Head.h"
#include "Game/Pancake.h"
#include "Game/PancakeLevel.h"
#include "UI/Controls/UIPancake.h"
#include "UI/Controls/UIStateInfo.h"
#include "UI/Controls/UIStatusBar.h"
#include "RSE/Game/SpriteLayered.h"
#include "RSE/Game/TextureAnimatorLayered.h"
#include "RSE/UI/DialogMgr.h"
#include "RSE/UI/Controls/UITextBox.h"
#include "RSE/Render/Camera.h"
#include "RSE/Render/Scene.h"
#include "RSE/UI/DialogFrame.h"
#include "Standard/Config.h"
#include "Standard/Help.h"
#include "Standard/Interpolation.h"
#include "Standard/Rand.h"

IMPLEMENT_RTTI(DlgGameHUD);

UISTYLESTREAM(DlgGameHUD)
	STREAMVAR(pancakesStart);
	STREAMVAR(pancakeOffset);
	STREAMVAR(pancakeScaleOnStack);
	STREAMVAR(pancakesPerStack);
	STREAMVAR(pancakeStackOffset);
	STREAMVAR(pancakeGiveFreq);
	STREAMVAR(barSize);
	STREAMVAR(barPos);
UISTYLESTREAMEND

DlgGameHUD::DlgGameHUD(const std::string& styleName)
{
	setConfigCategory("DlgGameHUD" + styleName);
}

void DlgGameHUD::construct()
{
	Super::construct();

	m_bar = new UIStatusBar;
	m_bar->setStyle("Health Bar");
	addChild(m_bar);
	m_bar->setSize(style().barSize);
	m_bar->setPos(style().barPos);

	setFrame(DialogFrameNaked::static_rtti());
	setSize(parent()->size());

	setColour(RGBA(0,0,0,0));

	setInputEnabled(false);

	m_stacks.push_back(Pancakes());

	m_info = 0;
}

void DlgGameHUD::setTarget(const PtrGC<PlayerMan>& m)
{
	m_target = m;
	
	if (!m_info)
	{
		m_info = new UIStateInfo(m_target);
		addChild(m_info);
	}

	if (m)
	{
		m_bar->setValue(&m->refHealth());
		m_bar->setMax(m->maxHealth());
	}
	else
	{
		m_bar->setValue(0);
	}
}

void DlgGameHUD::update(float delta)
{
	Super::update(delta);

	if (!m_target)
		setTarget(0);
}

void DlgGameHUD::addPancake(const PtrGC<Head>& head)
{
	if (!head)
		return;

	Point3 min = head->worldPos();
	Point3 max = head->worldPos();
	min.x -= head->size().x * 0.5f;
	min.z -= head->size().z * 0.5f;
	max.x += head->size().x * 0.5f;
	max.z += head->size().z * 0.5f;
	Point2 sMin(head->level().scene().camera().worldToScreen(min));
	Point2 sMax(head->level().scene().camera().worldToScreen(max));
	Point2 size(sMax - sMin);
	size.x = abs(size.x);	
	size.y = abs(size.y);

	Point2 nextPos;
	
	if ((int)m_stacks.back().size() < style().pancakesPerStack())
		nextPos = Point2(style().pancakeOffset().x, style().pancakeOffset().y * (float)m_stacks.back().size());
	else
		m_stacks.push_back(Pancakes());
	
	if (m_stacks.back().empty())
		m_currentStackPos = style().pancakesStart + Point2(style().pancakeStackOffset().x * ((float)m_stacks.size()-1), style().pancakeStackOffset().y);

	nextPos += m_currentStackPos;

	UIPancake* newPancake = new UIPancake(m_info, new Pancake(*head), 0.75f, head->sprite().animator().set(), (sMax + sMin) * 0.5f, size, nextPos, size * style().pancakeScaleOnStack());
	addChild(newPancake);
	m_stacks.back().push_back(newPancake);
}

void DlgGameHUD::onLevelOver()
{
	counters.set(1.0f / style().pancakeGiveFreq, delegate(&DlgGameHUD::givePancake));
}

void DlgGameHUD::givePancake()
{
	if (m_stacks.back().empty())
		m_stacks.erase(m_stacks.end() - 1);

	if (m_stacks.empty())
	{
		counters.set(App().configLevel().get("Options", "NextLevelWait", 3.0f), delegate(&DlgGameHUD::onPancakesDone));
		return;
	}

	m_stacks.back().back()->give(m_target);
	m_stacks.back().erase(m_stacks.back().end() - 1);
	
	counters.set(1.0f / style().pancakeGiveFreq, delegate(&DlgGameHUD::givePancake));
}

void DlgGameHUD::onPancakesDone()
{
	App().game().nextLevel();
}
