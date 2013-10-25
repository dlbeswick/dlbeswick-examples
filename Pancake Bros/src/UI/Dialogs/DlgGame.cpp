// ---------------------------------------------------------------------------------------------------------
// 
// DlgGame
// 
// ---------------------------------------------------------------------------------------------------------
#include "stdafx.h"

#include "DlgGame.h"
#include <RSEApp.h>
#include "Game/PancakeLevel.h"
#include "Game/ManAIControl.h"
#include "Game/Game.h"
#include "UI/Dialogs/DlgDebugPancake.h"
#include "UI/Dialogs/DlgGameHUD.h"
#include "RSE/Game/Database2D.h"
#include "RSE/Game/ObjectMgr.h"
#include "RSE/Render/ParticleFactory.h"
#include "RSE/Render/Camera.h"
#include "RSE/Render/SDeviceD3D.h"
#include "RSE/Render/SFont.h"
#include "RSE/Render/MeshObject.h"
#include "RSE/Render/Scene.h"
#include "RSE/Terrain/TerrainDB.h"
#include "RSE/UI/DialogMgr.h"
#include "RSE/UI/Dialogs/DlgMessageBox.h"
#include "RSE/UI/Dialogs/DlgDebugLevel.h"
#include "RSE/UI/Controls/UIMenu.h"
#include "RSE/UI/Controls/UIShuttle.h"
#include "Standard/Collide.h"

IMPLEMENT_RTTI(DlgGame);

DlgGame::DlgGame(const std::string& levelName)
{
	m_level = new PancakeLevel(levelName);
	m_level->construct();
};

DlgGame::~DlgGame()
{
}

void DlgGame::construct()
{
	Super::construct();

	setPointer(0);
	m_hud = clientArea().addChild(new DlgGameHUD("Player1"));
	m_hud->setTarget(App().game().player(0));
}

// onActivate
void DlgGame::onActivate()
{
	enableDebug(false);
	AppBase().addLevel(*m_level);
}

void DlgGame::onDraw()
{
	Super::onDraw();
}

bool DlgGame::mouseDown(const Point2& pos, int button)
{
	enableDebug(true);
	dlgDebug().enableStepping(true);
	return Super::mouseDown(pos, button);
}

void DlgGame::setupMenu(UIMenu& menu)
{
	Super::setupMenu(menu);

	Camera& c = level().scene().camera();
	Point3 origin, dir, hit;
	c.screenToRay(DialogMgr().pointerPos(), origin, dir);

	manSelected = 0;
	for (Object::ObjectList::const_iterator i = level().objectRoot()->children().begin(); i != level().objectRoot()->children().end(); ++i)
	{
		PtrGC<Man> m = Cast<Man>(*i);
		if (!m || !m->controller() || !Cast<ManAIControl>(m->controller()->active()))
			continue;

		if (Collide::rayAABB(AABB(m->pos(), m->extent()), origin, dir, hit))
		{
			manSelected = m;
			break;
		}
	}

	if (manSelected)
		menu.addItem(new MenuItemText("AI Debug", delegate(&DlgGame::onAIDebug)));

	PtrGC<UIMenu> m = menu.addExpander(new MenuItemText("Level " + itostr(App().game().currentLevel())));
	int n = 1;
	while (App().configLevel().exists(Game::levelName(n)))
	{
		m->addItem(new MenuItemText(itostr(n), delegate(&DlgGame::onWave)));
		++n;
	}
}

void DlgGame::onWave(const MenuItemText& m)
{
	App().game().nextLevel(atoi(m.text().c_str()));
}

void DlgGame::onAIDebug(const MenuItemText&)
{
	addChild(new DlgMessageBox("FART"));
}

void DlgGame::update(float delta)
{
	Super::update(delta);
}

Level* DlgGame::createLevel()
{
	return 0;
}

DlgDebug* DlgGame::createDlgDebug() const
{
	return new DlgDebugPancake;
}

void DlgGame::onDestroy()
{
	App().game().onDlgGameDestroy();
}
