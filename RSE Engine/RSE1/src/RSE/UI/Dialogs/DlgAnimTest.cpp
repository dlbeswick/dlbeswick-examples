// ---------------------------------------------------------------------------------------------------------
// 
// DlgAnimTest
// 
// ---------------------------------------------------------------------------------------------------------
#include "pch.h"

#include "DlgAnimTest.h"
#include "PathResource.h"
#include "Render/Scene.h"
#include "Render/Camera.h"
#include "Render/D3DPainter.h"
#include "Render/FontElement.h"
#include "Render/SFont.h"
#include "UI/Controls/UIMenu.h"
#include "UI/DialogMgr.h"

//REGISTER_RTTI_NAME(DlgAnimTest, "Anim Test");
IMPLEMENT_RTTI(DlgAnimTest);


// constructor
DlgAnimTest::DlgAnimTest()
{
	itextstream is(PathResource("media/man2/man2_firsttest.!x").open());
	m_mesh.load(is);

	m_bStopped = true;
	m_bLooping = true;
}


// setupMenu
void DlgAnimTest::setupMenu(UIMenu& menu)
{
	m_regressionMenu = menu.addExpander(new MenuItemText("Regression")).downcast<UIMenuText>();
	m_regressionMenu->fillFromDirectory(Path("media\\animationtest"), "*.!x", delegate(&DlgAnimTest::onRegression));
	menu.addItem(new TMenuItemAssign<bool>("Stop", m_bStopped, true));
	menu.addItem(new MenuItemText("Frame", delegate(&DlgAnimTest::onFrame)));
	menu.addItem(new TMenuItemAssign<bool>("Play", m_bStopped, false));
	menu.addItem(new MenuItemText("Rewind", delegate(&DlgAnimTest::onRewind)));
	menu.addItem(new TMenuItemToggle<bool>("Loop", m_bLooping, true, false));
}


// onDrawScene
void DlgAnimTest::onDrawScene()
{
	m_mesh.draw();
}


// onDraw
void DlgAnimTest::onDraw()
{
	Animation& anim = m_mesh.animation();

	setFont("arial");
	font().write(D3DPaint(), std::string("frame: ") + anim.timeToFrame(anim.time()), 0, 0.75f - Font()->height());
	D3DPaint().draw();
}


// onActivate
void DlgAnimTest::onActivate()
{
	level().scene().camera().setPos(Point3(0, -250, 25));
}


// update
void DlgAnimTest::update(float delta)
{
	Super::update(delta);

	m_mesh.animation().setLooping(m_bLooping);

	if (!m_bStopped)
	{
		m_mesh.animation().update(delta);
	}
}

// onRegression
void DlgAnimTest::onRegression(const MenuItemText&)
{
	itextstream is(PathResource(m_regressionMenu->selectedItem()->data()).open());
	m_mesh = SkelMesh();
	m_mesh.load(is);
}

// onRewind
void DlgAnimTest::onRewind(const MenuItemText&)
{
	m_mesh.animation().setTime(0);
	m_mesh.animation().update(0);
}

// onFrame
void DlgAnimTest::onFrame(const MenuItemText&)
{
	m_mesh.animation().update(m_mesh.animation().frameToTime(1));
}
