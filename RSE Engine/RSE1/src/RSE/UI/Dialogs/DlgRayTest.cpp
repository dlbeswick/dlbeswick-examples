// ---------------------------------------------------------------------------------------------------------
// 
// DlgRayTest
// 
// ---------------------------------------------------------------------------------------------------------
#include "pch.h"

#include <Standard/Collide.h>

#include "DlgRayTest.h"
#include "PathResource.h"
#include "Game/Database2D.h"
#include "Render/Camera.h"
#include "Render/MeshObject.h"
#include "Render/D3DPainter.h"
#include "Render/Scene.h"
#include "Render/SDeviceD3D.h"
#include "Terrain/TerrainDB.h"
#include "UI/Controls/UIMenu.h"
#include "UI/DialogMgr.h"
#include "Standard/DXInput.h"
#include "Standard/FileMgr.h"

//REGISTER_RTTI_NAME(DlgRayTest, "Ray Test (deprecated)");
IMPLEMENT_RTTI(DlgRayTest);


static Tri TRIANGLE(Point3(0, 55, 0), Point3(0, 55, 10), Point3(10, 55, 0));
static Point3 LINE[2] = { Point3(2, -500, 5), Point3(2, 500, 5) };

// Ray Tri test
struct RayTriTest : public DlgRayTest::State
{
	RayTriTest() : m_bHit(false), m_rot(1, Point3(0, 0, 0))
	{}

	virtual void draw(Level& level)
	{
		D3D().zbuffer(true);

		RGBA col;
		if (m_bHit)
		{
			col = RGBA(1.0f, 0.0f, 0.0f, 1.0f);
		}
		else
		{
			col = RGBA(0.0f, 1.0f, 0.0f, 1.0f);
		}

		D3DPaint().setFill(col);
		D3DPaint().tri(m_tri);

		D3DPaint().setFill(RGBA(1.0f, 0.0f, 0.0f, 0.5f));
		Tri t(m_hit + Point3(-5, 0, -5), m_hit + Point3(5, 0, 5), m_hit + Point3(-5, 0, 5)); 
		D3DPaint().tri(t);

		D3DPaint().setFill(RGBA(1.0f, 0.0f, 0.0f, 1.0f));
		D3DPaint().line(m_line[0], m_line[1]);

		Point3 normal = m_tri.calcNormal();

		D3DPaint().line(Point3(0, 0, 0), normal * 10);
	}

	virtual void update(Level& level, float delta)
	{
		// rotate poly on input
		float inc = 0.05f;
		Point3 amt(0, 0, 0);
		if (Input().key(VK_LEFT))
		{
			amt.x -= inc;
		}
		if (Input().key(VK_RIGHT))
		{
			amt.x += inc;
		}
		if (Input().key(VK_UP))
		{
			amt.y -= inc;
		}
		if (Input().key(VK_DOWN))
		{
			amt.y += inc;
		}

		Quat q1;
		q1.angleAxis(amt.x, Point3(1, 0, 0));
		Quat q2;
		q2.angleAxis(amt.y, Point3(0, 1, 0));
		m_rot.multiply(q1);
		m_rot.multiply(q2);

		m_rot.normalise();

		Point3 origin, dir;
		level.scene().camera().screenToRay(DialogMgr().pointerPos(), origin, dir);
		m_line[0] = origin;
		m_line[1] = origin + dir * 1000;

		m_tri = TRIANGLE;
		for (int i = 0; i < 3; i++)
		{
			m_rot.multiply(m_tri[i]);
		}

		m_bHit = Collide::rayPolygon(m_tri, m_line[0], dir, m_hit);
	}

	bool m_bHit;
	Tri m_tri;
	Point3 m_line[2];
	Point3 m_hit;
	Quat m_rot;
};

// SpherePlaneSweepTest
struct SpherePlaneSweepTest : public DlgRayTest::State
{
//	SpherePlaneSweepTest() : m_bHit(false), m_start(12, 25, 0), m_vel(-0.5, 1, 0), m_plane(Point3(0, -1, 0), -37), m_speed(100)
	SpherePlaneSweepTest() : m_bHit(false), m_start(0, 37 - 2.747f, 0), m_vel(0, 1.941f, 0), m_plane(Point3(0, -1, 0), -37), m_speed(1.941f)
	{
		m_vel.normalise();
	}

	virtual void draw(Level& level)
	{
		Point3 end = m_start;
		RGBA col;

		if (m_bHit)
			col = RGBA(1, 0, 0, 1);
		else
			col = RGBA(0, 1, 0, 1);

		D3D().zbuffer(true);
		for (int i = 0; i <= 25; i++)
		{
			D3DPaint().setFill(col);
			D3DPaint().sphere(end, 1);
			end += m_vel * (m_speed / 25.0f);
		}

		D3DPaint().setFill(RGBA(1, 1, 1, 0.25f));
		D3DPaint().plane(m_plane, 10);
	}

	virtual void update(Level& level, float delta)
	{
		float inc = delta * 10;

		if (Input().key(VK_DOWN))
			m_speed -= inc;
		if (Input().key(VK_UP))
			m_speed += inc;
		if (Input().key(VK_END))
			m_start -= m_vel.normal() * inc;
		if (Input().key(VK_HOME))
			m_start += m_vel.normal() * inc;

		float u;
		m_bHit = Collide::spherePlaneSweep(m_start, m_vel * m_speed, 1, m_plane, u);

		m_hit = m_start + m_vel * m_speed * u;
	}

	bool m_bHit;
	Point3 m_hit;
	Plane m_plane;
	Point3 m_start;
	Point3 m_vel;
	float m_speed;
};

// SphereTriSweepTest
struct SphereTriSweepTest : public DlgRayTest::State
{
	SphereTriSweepTest() : m_bHit(false), m_start(0, 50 - 2.747f, 0), m_vel(0, 1.941f, 0), m_speed(1.941f),
							m_tri(Point3(-25, 50, 25), Point3(25, 50, -25), Point3(-25, 50, -25))
	{
		m_vel.normalise();
	}

	virtual void draw(Level& level)
	{
		Point3 end = m_start;
		RGBA col;

		if (m_bHit)
			col = RGBA(1, 0, 0, 0.25);
		else
			col = RGBA(0, 1, 0, 0.25);

		if (m_bHit)
		{
			D3DPaint().setFill(RGBA(0, 1, 0));
			D3DPaint().tick(m_hit, 5);
		}

		D3D().zbuffer(true);
		for (int i = 0; i <= 25; i++)
		{
			D3DPaint().setFill(col);
			D3DPaint().sphere(end, 1);
			end += m_vel * (m_speed / 25.0f);
		}

		D3DPaint().setFill(RGBA(1, 1, 1, 0.25f));
		D3DPaint().tri(m_tri);
	}

	virtual void update(Level& level, float delta)
	{
		float inc = delta * 10;

		if (Input().key(VK_DOWN))
			m_speed -= inc;
		if (Input().key(VK_UP))
			m_speed += inc;
		if (Input().key(VK_END))
			m_start -= m_vel.normal() * inc;
		if (Input().key(VK_HOME))
			m_start += m_vel.normal() * inc;
		if (Input().key(VK_INSERT))
			m_start -= (m_vel.normal().rCross(Point3(0, 0, 1))) * inc;
		if (Input().key(VK_DELETE))
			m_start += (m_vel.normal().rCross(Point3(0, 0, 1))) * inc;

		float u;
		m_bHit = Collide::sphereTriSweep(m_start, m_vel * m_speed, 1, m_tri, u);

		Plane plane(m_tri.calcNormal(), m_tri[0]);
		m_hit = m_start + m_vel * u;
		m_hit -= plane.distance(m_hit) * plane.n;
	}

	bool m_bHit;
	Point3 m_hit;
	Tri m_tri;
	Point3 m_start;
	Point3 m_vel;
	float m_speed;
};

// TerrainTrisSphereSweepTest
struct TerrainTrisSphereSweepTest : public DlgRayTest::State
{
	TerrainTrisSphereSweepTest() : m_start(0, 0, 0), m_end(0, 0, 0)
	{
		m_pObj = new MeshObject;

		itextstream streamFlat(PathResource("media/terrain/flat.!x").open());
		m_pObj->load(streamFlat);
		m_pObj->setColour(RGBA(0, 1, 0));

		Database2D().terrainDB().create(*m_pObj);
	}

	virtual void draw(Level& level)
	{
		D3D().zbuffer(true);

		level.database2D().terrainDB().mesh().draw();

		D3DPaint().setFill(RGBA(0.5f, 0, 0));
		for (TerrainDB::PolyRefList::iterator i = m_list.begin(); i != m_list.end(); ++i)
		{
			D3DPaint().tri(**i);
		}

		level.database2D().terrainDB().debugShow(level.scene());

		D3DPaint().setFill(RGBA(1, 1, 1));
		D3DPaint().sphere(m_start, 2);
		D3DPaint().sphere(m_end, 2);
	}

	virtual void update(Level& level, float delta)
	{
		if (Input().mousePressed(0))
		{
			Point3 origin, dir;
			level.scene().camera().screenToRay(DialogMgr().pointerPos(), origin, dir);

			Point3 hit;
			if (Collide::rayPlane(origin, dir, Plane(Point3(0, 0, 1), 0), hit))
				m_end = hit;
		}

		m_list.clear();
		Database2D().terrainDB().polysSphereSweep(m_start, m_end - m_start, 2, m_list);
	}

	virtual Point3 camPos() { return Point3(0, 0, 500); }
	virtual Quat camRot() { return QuatAngleAxis(PI / 2, Point3(1, 0, 0)); }

	Point3 m_start;
	Point3 m_end;
	TerrainDB::PolyRefList m_list;
	SmartPtr<MeshObject> m_pObj;
};

// SimpleSlideTest
struct SimpleSlideTest : public DlgRayTest::State
{
	SimpleSlideTest() : m_start(-10, 25, 0), m_vel(0.5, 1, 0), m_plane(Point3(0, -1, 0), -50), m_speed(50)
	{
		m_vel.normalise();
	}

	virtual void draw(Level& level)
	{
		D3D().zbuffer(true);
		Point3 end = m_start;
		
		D3DPaint().setFill(RGBA(1,1,1));
		for (int i = 0; i <= 25; i++)
		{
			D3DPaint().sphere(end, 1);
			end += m_vel * (m_speed / 25.0f);
		}
		D3DPaint().setFill(RGBA(1,0,0));
		D3DPaint().sphere(m_adjusted, 1);

		D3DPaint().setFill(RGBA(1,1,1,0.5f));
		D3DPaint().plane(m_plane, 20);
	}

	virtual void update(Level& level, float delta)
	{
		float inc = delta * 10;
		if (Input().key(VK_DOWN))
			m_speed -= inc;

		if (Input().key(VK_UP))
			m_speed += inc;

		Point3 end = m_start + m_vel * m_speed;
		float u;
		bool bHit = Collide::spherePlaneSweep(m_start, m_vel * m_speed, 1, m_plane, u);
		if (bHit)
		{
			m_adjusted = end + (m_plane.n * (-m_plane.distance(end) + 1));
		}
	}

	Plane m_plane;
	Point3 m_start;
	Point3 m_vel;
	Point3 m_adjusted;
	float m_speed;
};

// constructor
DlgRayTest::DlgRayTest()
{
	m_state = new RayTriTest;
}

// setupMenu
void DlgRayTest::setupMenu(class UIMenu& menu)
{
	menu.addItem(new TMenuItemAssign<SmartPtr<State> >("Ray/Tri", m_state, m_state/*that's raytritest*/));
	menu.addItem(new TMenuItemAssign<SmartPtr<State> >("Sphere/Plane Sweep", m_state, new SpherePlaneSweepTest));
	menu.addItem(new TMenuItemAssign<SmartPtr<State> >("Sphere/Tri Sweep", m_state, new SphereTriSweepTest));
	menu.addItem(new TMenuItemAssign<SmartPtr<State> >("Terrain Tri List Sphere Sweep", m_state, new TerrainTrisSphereSweepTest));
	menu.addItem(new TMenuItemAssign<SmartPtr<State> >("Simple Slide", m_state, new SimpleSlideTest));
}


// onDrawScene
void DlgRayTest::onDrawScene()
{
	if (m_state)
		m_state->draw(level());
}


// update
void DlgRayTest::update(float delta)
{
	Super::update(delta);
	if (m_state)
	{
		m_state->update(level(), delta);

		if (m_state->camPos() != Point3(0, 0, 0))
		{
			Camera& cam = level().scene().camera();
            cam.setPos(m_state->camPos());
			cam.setRot(m_state->camRot());
		}
	}
}


// onStart
void DlgRayTest::onStart()
{
	//SceneMgr().getCurrent().getCamera().setPos(Point3(0, 0, 0));
	level().scene().camera().setPos(Point3(0, -50, 0));
}
