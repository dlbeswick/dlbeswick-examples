// ---------------------------------------------------------------------------------------------------------
// 
// DlgFatal
// 
// ---------------------------------------------------------------------------------------------------------
#include "pch.h"

#include "DlgFatal.h"
#include "AppBase.h"
#include "Render/D3DPainter.h"
#include "Render/SFont.h"
#include "Render/FontElement.h"
#include "Render/SDeviceD3D.h"
#include "UI/DialogFrame.h"
#include "UI/DialogMgr.h"
#include "UI/Controls/UITextBox.h"
#include "UI/Controls/UIButton.h"
#include "Standard/DXInput.h"

IMPLEMENT_RTTI(DlgFatal);

// constructor
DlgFatal::DlgFatal(const std::vector<std::string>& callstack, const std::string& message)
{
	Font().set("arial");

	m_pTex = D3D().loadTextureFromResource("Skull", RT_BITMAP);

	m_pBox = new UITextBox;
	m_pBox->setSize(Point2(0.85f, 0.375f));
	m_pBox->setPos(Point2(0.075f, 0.1875f));
	m_pBox->setInputEnabled(false);
	m_pBox->setAlign(UITextBox::CENTER);
	m_pBox->setVAlign(UITextBox::VCENTER);
	m_pBox->setWrap(true);
	m_pBox->setFont("smallarial");
	m_pBox->setText(join(callstack, " <- ") + message);

	m_pButton = new UIButton;
	m_pButton->onClick = delegate(&DlgFatal::onExit);
	m_pButton->setText("Exit And Cry");
	m_pButton->setPos(Point2(m_pBox->pos().x + ((m_pBox->size().x - m_pButton->size().x) * 0.5f), 0.75f - m_pButton->size().y - 0.025f));
}

void DlgFatal::construct()
{
	Super::construct();

	addChild(m_pButton);
	addChild(m_pBox);

	DialogMgr().pointerLock(false);

	setFrame(DialogFrameNaked::static_rtti());
}


// onDraw
void DlgFatal::onDraw()
{
	D3DPaint().setFill(RGBA(0,0,0));
	D3DPaint().quad2D(0, 0, 1, 1);
	D3DPaint().setFill(RGBA(0,0,0), m_pTex);
	D3DPaint().quad2D(0.1275f, 0, 0.8725f, 0.75f);
}

// onPostDraw
void DlgFatal::onPostDraw()
{
	setFont("bigarial");
	font().write(D3DPaint(), "FATAL", 0.179f, 0);
	D3DPaint().draw();
}

// onExit
void DlgFatal::onExit()
{
	AppBase().exit();
}
