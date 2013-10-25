// ------------------------------------------------------------------------------------------------
//
// DlgDebugPancake
//
// ------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "DlgDebugPancake.h"
#include "DlgDebugAI.h"
#include "Game/AI/AI.h"
#include "Game/ManAIControl.h"
#include "Game/Man.h"
#include "Game/Controller.h"
#include "RSE/UI/Controls/UIMenu.h"

DlgDebugPancake::DlgDebugPancake()
{
}

void DlgDebugPancake::construct()
{
	Super::construct();

	m_dlgDebugAI = new DlgDebugAI;
	addChild(m_dlgDebugAI);
}

bool DlgDebugPancake::mouseDown(const Point2& pos, int button)
{
	return Super::mouseDown(pos, button);
}

void DlgDebugPancake::fillContextMenu(UIMenu& menu)
{
	Super::fillContextMenu(menu);

	if (!m_hoverObject)
		return;

	// tbd: cast to IControllable
	PtrGC<Man> man = Cast<Man>(m_hoverObject);
	if (!man)
		man = Cast<Man>(m_hoverObject->parent());

	if (man)
	{
		PtrGC<AI> ai = Cast<AI>(man->controller());
		if (ai)
		{
			TMenuItemData<PtrGC<Man> >* item = new TMenuItemData<PtrGC<Man> >("AI Debug (" + m_hoverObject->name() + ")", man, delegate(&DlgDebugPancake::onAIDebug));
			menu.addItem(item);
		}
	}
}

void DlgDebugPancake::onAIDebug(const PtrGC<Man>& man)
{
	if (man)
	{
		PtrGC<AI> ai = Cast<AI>(man->controller());
		if (ai)
			m_dlgDebugAI->set(Cast<ManAIControl>(ai->methodOfType(ManAIControl::static_rtti())));
	}
}
