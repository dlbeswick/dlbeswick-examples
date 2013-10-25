// ---------------------------------------------------------------------------------------------------------
// 
// DlgStandard
// 
// ---------------------------------------------------------------------------------------------------------
#include "pch.h"

#include "DlgStandard.h"
#include <Standard/Config.h>
#include "UI/DialogFrame.h"
#include "UI/Dialogs/DlgDebug.h"
#include "UI/Dialogs/DlgMessageBox.h"

IMPLEMENT_RTTI(DlgStandard);

// constructor
DlgStandard::DlgStandard() :
	m_dlgDebug(0)
{
}

void DlgStandard::construct()
{
	Super::construct();

	setFrame(DialogFrameNaked::static_rtti());
	setColour(RGBA(0,0,1.0f));
	setDebugKey(VK_ESCAPE);

	enableDebug(false);
}

DlgStandard::~DlgStandard()
{
}

// enableDebug
void DlgStandard::enableDebug(bool bEnable)
{
	if (bEnable)
	{
		if (!m_dlgDebug)
		{
			m_dlgDebug = createDlgDebug();
			UIElement::addChild(m_dlgDebug);
			m_dlgDebug->setFrame(DialogFrameNaked::static_rtti());
			m_dlgDebug->setColour(RGBA(0,0,0,0));
			m_dlgDebug->setupMenu = delegate(&DlgStandard::setupMenu);
			setDebugKey(m_debugKey);
		}

		m_dlgDebug->bringToFront();
		m_dlgDebug->setVisible(true);
	}
	else
	{
		if (m_dlgDebug)
			m_dlgDebug->setVisible(false);
	}
}

void DlgStandard::setDebugKey(int key)
{
	m_debugKey = key;

	if (m_dlgDebug)
		m_dlgDebug->debugKey = key;
}

void DlgStandard::keyUp(int key)
{
	if (key == m_debugKey)
		enableDebug(true);
}

void DlgStandard::toggleDebug()
{ 
	enableDebug(!m_dlgDebug->visible());
}

DlgDebug* DlgStandard::createDlgDebug() const
{
	return new DlgDebug;
}
