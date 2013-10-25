// ------------------------------------------------------------------------------------------------
//
// DlgPhysTest
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"

#include "DlgPhysTest.h"
#include "PathResource.h"
#include "Physics/PhysicsMgr.h"
#include "Physics/PhysSphere.h"
#include "Render/MeshObject.h"
#include "Terrain/TerrainDB.h"
#include "UI/Controls/UIMenu.h"
#include "UI/DialogMgr.h"

//REGISTER_RTTI_NAME(DlgPhysTest, "Physics Test");
IMPLEMENT_RTTI(DlgPhysTest);

// construct
DlgPhysTest::DlgPhysTest()
{
	load("media/terrain/slope.!x");	

	Camera& cam = level().scene().camera();
	cam.setPos(Point3(50, -50, 25));

	m_bPlay = true;
}

// setupMenu
void DlgPhysTest::setupMenu(UIMenu& menu)
{
	menu.addExpander(new MenuItemText("Terrain"))->fillFromDirectory(Path("media/terrain"), "*.!x", delegate(&DlgPhysTest::onLoad));
	menu.addItem(new TMenuItemAssign<bool>("Go", m_bPlay, true));
	menu.addItem(new TMenuItemAssign<bool>("Pause", m_bPlay, false));
	menu.addItem(new MenuItemText("Step", delegate(&DlgPhysTest::onStep)));
	menu.addItem(new MenuItemText("Step Small", delegate(&DlgPhysTest::onStepSmall)));
	menu.addItem(new MenuItemText("Reset", delegate(&DlgPhysTest::onReset)));
}

// onLoad
void DlgPhysTest::onLoad(const MenuItemText& item)
{
	load(item.data());
}

// load
void DlgPhysTest::load(const std::string& terrainPath)
{
	PtrGC<MeshObject> terrain = new MeshObject;
	terrain->setColour(RGBA(0, 1, 0));

	itextstream streamTerrain(PathResource(terrainPath).open());
	terrain->load(streamTerrain);

	level().clear();
	terrain->setParent(level().objectRoot());
	level().database2D().terrainDB().create(*terrain);

	reset();
}

// update
void DlgPhysTest::update(float delta)
{
	Super::update(delta);
	level().update();
}

// onDrawScene
void DlgPhysTest::onDrawScene()
{
	level().draw();
}

// onStep
void DlgPhysTest::onStep(const class MenuItemText&)
{
	level().physicsMgr().update(0.1f);
}

// onStepSmall
void DlgPhysTest::onStepSmall(const class MenuItemText&)
{
	level().physicsMgr().update(0.25f);
}

// onKeyChar
void DlgPhysTest::onKeyChar(int key)
{
	if (key == 'p')
	{
		m_bPlay = !m_bPlay;
	}
	else if (key == '.')
	{
		level().physicsMgr().update(0.01f);
	}
	else if (key == '>')
	{
		level().physicsMgr().update(0.1f);
	}
	else if (key == 'r')
	{
		reset();
	}
}

// reset
void DlgPhysTest::reset()
{
	level().physicsMgr().clear();

	PtrGC<PhysSphere> sphere = new PhysSphere(2);
	sphere->setParent(level());
	sphere->setPos(Point3(50, 0, 25));
	sphere->setMass(8);
	//sphere->setFriction(0.5f);
	level().collisionGroup().add(sphere);
}
