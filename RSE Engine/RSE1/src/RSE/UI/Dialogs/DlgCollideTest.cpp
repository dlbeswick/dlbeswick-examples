// ---------------------------------------------------------------------------------------------------------
// 
// DlgCollideTest
// 
// ---------------------------------------------------------------------------------------------------------
#include "pch.h"

#include <Standard/Collide.h>

#include "AppBase.h"
#include "DlgCollideTest.h"
#include "Game/Database2D.h"
#include "Render/Camera.h"
#include "Render/MeshObject.h"
#include "Render/SDeviceD3D.h"
#include "Render/Scene.h"
#include "Terrain/TerrainDB.h"
#include "Physics/PhysicsMgr.h"
#include "Physics/PhysicsObject.h"
#include "Physics/PhysRay.h"
#include "Physics/PhysSphere.h"
#include "UI/DialogMgr.h"
#include "UI/DragHelper3D.h"
#include "UI/Controls/UIMenu.h"
#include "UI/Dialogs/DlgDebug.h"
#include "Standard/DXInput.h"

//REGISTER_RTTI_NAME(DlgCollideTest, "Collision Test");
IMPLEMENT_RTTI(DlgCollideTest);


// constructor
DlgCollideTest::DlgCollideTest()
{
	m_dragger = addChild(new DragHelper3D);
}

// setupMenu
void DlgCollideTest::setupMenu(class UIMenu& menu)
{
	PtrGC<UIMenu> objects = menu.addExpander(new MenuItemText("Objects"));
	
	RegistrantsList classes;
	registrantsOfClass(PhysicsObject::static_rtti(), classes);
	for (RegistrantsList::const_iterator i = classes.begin(); i != classes.end(); ++i)
	{
		const Registrant& r = **i;
		objects->addItem(new TMenuItemData<const Registrant*>(r.name, &r, delegate(&DlgCollideTest::onCreate)));
	}
}

void DlgCollideTest::onCreate(const TMenuItemData<const Registrant*>& d)
{
	PhysicsObject* o = (PhysicsObject*)Registrar<Base>::newObject(d.data()->rtti);
	
	Point3 origin, dir;
	level().scene().camera().screenToRay(DlgStandard::dlgDebug().m_pMenu->screenPos(), origin, dir);
		
	o->setPos(origin + 10.0f * dir);
	o->setGravityScale(0);

	level().collisionGroup().add(o);
}

void DlgCollideTest::navigation()
{
	if (!m_dragger->dragging())
		fpsNavigation();
}

static PhysicsObject *o0,*o1;

// update
void DlgCollideTest::update(float delta)
{
	Super::update(delta);
	//o0->setVel(Point3(10, 0, 0));
	//o1->setVel(Point3(-10, 0, 0));
}


// activate
void DlgCollideTest::activate()
{
	Super::activate();

	//SceneMgr().getCurrent().getCamera().setPos(Point3(0, 0, 0));
	level().scene().camera().setPos(Point3(0, -10, 0));

	o0 = new PhysSphere(0.05f);
	o0->setPos(Point3(-10, 0.1f, 0));
	o0->setGravityScale(0);
	o0->setMass(1.0f);
	level().collisionGroup().add(o0);

	o1 = new PhysSphere;
	o1->setPos(Point3(10, 0.1f, 0));
	o1->setGravityScale(0);
	o1->setMass(1);
	level().collisionGroup().add(o1);

	o0->setVel(Point3(5, 0, 0));
	o1->setVel(Point3(-5, 0, 0));
}

bool DlgCollideTest::onMouseUp(const Point2& p, int button)
{
	if (button != 0)
		return Super::onMouseUp(p, button);

	Point3 origin, dir;
	level().scene().camera().screenToRay(p, origin, dir);

	PtrGC<PhysRay> ray = new PhysRay(dir, origin);

	PtrGC<PhysicsObject> hit = level().physicsMgr().findFirstCollision(ray).collider;

	if (m_selected)
		m_selected->setColour(RGBA(1, 1, 1));

	if (hit)
	{
		m_selected = hit;
		hit->setColour(RGBA(0, 1, 0));
	}
	else
		m_selected = 0;

	m_dragger->setTarget(m_selected);

	ray.destroy();
	//level().physicsMgr().add(ray);

	return Super::onMouseUp(p, button);
}
