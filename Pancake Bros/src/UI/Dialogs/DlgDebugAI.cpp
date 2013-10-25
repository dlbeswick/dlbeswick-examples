// ------------------------------------------------------------------------------------------------
//
// DlgDebugAI
//
// ------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "DlgDebugAI.h"
#include "Game/ManAIControl.h"
#include "Game/AI/Goals/Selector.h"
#include "RSE/Game/Level.h"
#include "RSE/Render/Scene.h"
#include "RSE/Render/Camera.h"
#include "RSE/UI/UILayout.h"
#include "RSE/UI/Controls/UIListView.h"

IMPLEMENT_RTTI(DlgDebugAI);

DlgDebugAI::DlgDebugAI()
{
}

void DlgDebugAI::construct()
{
	Super::construct();

	UILayoutFill* layout = new UILayoutFill;
	addChild(layout);

	m_info = layout->addChild(new UIListView);
	setVisible(false);
	setFont("smallarial");

	setSize(Point2(0.5f, 0.5f));
	setColour(RGBA(1,1,1,0.5f));
}

void DlgDebugAI::set(const PtrGC<ManAIControl>& ai)
{
	m_info->clear();
	m_info->setColumns(4);

	m_ai = ai;

	if (!m_ai)
	{
		m_info->add(new ListText("No AI"));
		setVisible(false);
		return;
	}

	setVisible(true);
}

void DlgDebugAI::update(float delta)
{
	Super::update(delta);

	if (!m_ai)
	{
		set(0);
		return;
	}

	ListText* text;
	RGBA colour;

	Goals::Selector& ai = m_ai->rootGoal();

	m_info->clear();
	for (uint i = 0; i < ai.m_goals.size(); ++i)
	{
		AIGoal& g = *ai.m_goals[i];

		if (&g == ai.m_curGoal)
			colour = RGBA(0, 1, 0, 1);
		else if (g.lastScore > 0)
			colour = RGBA(1, 1, 1, 1);
		else
			colour = RGBA(1, 0, 0, 1);

		text = new ListText(g.rtti().className());
		text->setColour(colour);
		m_info->add(text);

		if (g.likelihood == FLT_MAX)
			text = new ListText("L: MAX");
		else
			text = new ListText("L: " + ftostr(g.likelihood));

		text->setColour(colour);
		m_info->add(text);

		text = new ListText("R: " + ftostr(g.repetitionWeight));
		text->setColour(colour);
		m_info->add(text);

		text = new ListText("Score: " + ftostr(g.lastScore));
		text->setColour(colour);
		m_info->add(text);
	}

	/*if (m_ai->obj())
		setPos(m_ai->obj()->level().scene().camera().worldToScreen(m_ai->obj()->worldPos()));*/
}