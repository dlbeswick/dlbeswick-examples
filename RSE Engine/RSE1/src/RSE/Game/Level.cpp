// ------------------------------------------------------------------------------------------------
//
// Level
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"

#include "Level.h"
#include "AppBase.h"
#include "Physics/PhysicsMgr.h"
#include "Render/ParticleRenderer.h"
#include "Render/RenderContext.h"
#include "Render/SDeviceD3D.h"
#include "Render/D3DPainter.h"
#include "Sound/ISoundProvider.h"
#include "Terrain/TerrainDB.h"
#include "ObjectQuadtree.h"
#include <Standard/Collide.h>
#include <Standard/Config.h>
#include <Standard/Profiler.h>
#include <mm3dnow.h>

// Level
Level::Level()
{
	m_physicsMgr = new PhysicsMgr(*this);
	m_objectRoot = new Object;
	m_objectRoot->setLevel(this);
	m_particleRoot = new ParticleRenderer;
	m_particleRoot->setParent(m_objectRoot);
	m_gravity = AppBase().options().get("World", "Gravity", Point3(0, 0, -9.8f));
	m_collisionGroup = m_physicsMgr->addCollisionGroup();
	AppBase().sound().setListener(&scene().camera());

	D3D().alpha(true);
	D3DD().SetTransform(D3DTS_WORLD, Matrix4::IDENTITY);

	D3DLIGHT9 l;
	zero(l);
	l.Type = D3DLIGHT_DIRECTIONAL;
	l.Diffuse = RGBA(1,1,1,1);
	l.Specular = RGBA(1,1,1,1);
	l.Direction.y = 1;
	D3DD().SetLight(0, &l);
	D3DD().LightEnable(0, TRUE);

	m_context = new RenderContext;
}

Level::~Level()
{
	delete m_physicsMgr;
	delete m_context;
	m_objectRoot.destroy();
	AppBase().removeLevel(*this);
}

// update
void Level::update()
{
	m_timer.endFrame();
	float delta = (float)time().frame();

	Base::update(delta);

	// autolists
	// reset lists
	for (AutoListHash::iterator i = m_currentAutoList.begin(); i != m_currentAutoList.end(); ++i)
		i->second = ObjectList();

	if (delta)
	{
		m_physicsMgr->update(delta);
		m_objectRoot->update(delta);

		ProfileValue("Top-Level Nodes", m_objectRoot->children().size() - 1);
		ProfileValue("Particle Nodes", m_particleRoot->children().size());
	}

	// autolists
	m_lastAutoList = m_currentAutoList;
}

// draw
void Level::draw()
{
	scene().clear();
	drawScene();

	if (AppBase().options().get<bool>("Debug", "ShowPhysicsDebug"))
		m_physicsMgr->drawDebug();

	if (AppBase().options().get<bool>("Debug", "TerrainQuadtree"))
		database2D().terrainDB().debugShow(scene());

	if (AppBase().options().get<bool>("Debug", "ObjectQuadtree"))
		database2D().objectDB().draw();

	for (uint i = 0; i < m_deferredPainters.size(); ++i)
	{
		D3DPainter* p = m_deferredPainters[i];
		p->draw();
		delete p;
	}

	m_deferredPainters.resize(0);
}

void Level::drawScene()
{
	D3D().setContext(*m_context);
	scene().camera().draw();

	for (Object::ObjectList::const_iterator i = m_objectRoot->children().begin(); i < m_objectRoot->children().end(); ++i)
	{
		standardDrawNode(i->ptr());
	}
}

void Level::standardDrawNode(Object* node)
{
	node->draw();

	for (Object::ObjectList::const_iterator i = node->children().begin(); i < node->children().end(); ++i)
	{
		standardDrawNode(i->ptr());
	}
}

// move
void Level::move(const PtrGC<Object>& obj, const Point3& newLoc)
{
	ObjectQuadtree& db = database2D().objectDB();
	if (db.isAdded(obj))
		db.remove(obj);

	obj->setPos(newLoc);

	db.add(obj);
}

void Level::registerAutoList(const RTTI& what)
{
	m_currentAutoList[&what] = ObjectList();
}

const Level::ObjectList& Level::autoList(const RTTI& what)
{
	AutoListHash::iterator i = m_lastAutoList.find(&what);
	if (i != m_lastAutoList.end())
		return i->second;
	else
		return m_lastAutoList[0];
}

void Level::onObjectUpdate(Object* caller)
{
	Profile("Level::onObjectUpdate");

	const RTTI* r = &caller->rtti();
	while (r->base() != r)
	{
		AutoListHash::iterator i = m_currentAutoList.find(r);

		if (i != m_currentAutoList.end())
			i->second.add(caller);

		r = r->base();
	}
}

void Level::addDeferredPainter(D3DPainter& painter)
{
	m_deferredPainters.push_back(&painter);
}

PtrGC<Object> Level::objectAtScreenPoint(const Point2& p) const
{
	struct Recurse
	{
		Recurse()
		{
			closestDist = FLT_MAX;
		}

		void go(const Point2& p, Object* node, const Level& l)
		{
			static const Point3 extents[8] = {
				Point3(-0.5f, -0.5f, -0.5f),
				Point3(0.5f, -0.5f, -0.5f),
				Point3(0.5f, 0.5f, -0.5f),
				Point3(-0.5f, 0.5f, -0.5f),
				Point3(-0.5f, -0.5f, 0.5f),
				Point3(0.5f, -0.5f, 0.5f),
				Point3(0.5f, 0.5f, 0.5f),
				Point3(-0.5f, 0.5f, 0.5f)
			};

			for (Object::ObjectList::const_iterator i = node->children().begin(); i < node->children().end(); ++i)
			{
				go(p, i->ptr(), l);
			}

			if (node->extent() == Point3::ZERO)
				return;

			Point2 min(Point2::MAX), max(-Point2::MAX);

			for (uint i = 0; i < 8; ++i)
			{
				Point2 vertex = l.scene().camera().worldToScreen(node->worldPos() + extents[i].rComponentMul(node->extent()));
				min.x = std::min(min.x, vertex.x);
				min.y = std::min(min.y, vertex.y);
				max.x = std::max(max.x, vertex.x);
				max.y = std::max(max.y, vertex.y);
			}

			/*D3DPaint().reset();
			D3DPaint().quad2D(min.x, min.y, max.x, max.y);
			D3DPaint().draw();*/

			if (min != Point2::ZERO && max != Point2::ZERO)
			{
				if (Collide::pointInBox2D(p, min, max))
				{
					float dist = (l.scene().camera().worldPos() - node->worldPos()).length();
					if (dist < closestDist)
					{
						best = node;
						closestDist = dist;
					}
				}
			}
		}

		PtrGC<Object> best;
		float closestDist;
	};

	Recurse r;
	r.go(p, (Object*)m_objectRoot, *this);

	return r.best;
}

Point3 Level::gravity() const 
{ 
	return m_gravity; 
}

void Level::setGravity(const Point3& v)
{ 
	m_gravity = v;
}
