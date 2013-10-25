// ------------------------------------------------------------------------------------------------
//
// Level
// Object encapsulating everything about a game level
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/Game/Object.h"
#include "RSE/Game/Database2D.h"
#include "RSE/Render/Scene.h"
#include "Standard/Base.h"
#include "Standard/SmartPtr.h"
#include "Standard/Timer.h"
class ParticleRenderer;
class PhysicsMgr;
class CollisionGroup;

class RSE_API Level : public Base, public PtrGCHost
{
public:
	Level();
	virtual ~Level();

	typedef PendingListNullRemover<PtrGC<Object> > ObjectList;

	const PtrGC<Object>& objectRoot() const	{ return m_objectRoot; }
	const PtrGC<ParticleRenderer>& particleRoot()	{ return m_particleRoot; }
	Database2D& database2D()			{ return m_database2D; }
	Scene& scene()						{ return m_scene; }
	const Scene& scene() const			{ return m_scene; }

	// render
	class RenderContext* renderContext() const { return m_context; }

	// autolists
	template <class T>
		void registerAutoList() { registerAutoList(T::static_rtti()); }
	void registerAutoList(const RTTI& what);

	template <class T>
		const ObjectList& autoList() { return autoList(T::static_rtti()); }
	const ObjectList& autoList(const RTTI& what);

	void onObjectUpdate(Object* caller); // call when objects update themselves

	// physics
	PhysicsMgr& physicsMgr()			{ return *m_physicsMgr; }
	CollisionGroup& collisionGroup()	{ return *m_collisionGroup; }

	virtual void update();
	virtual void draw();

	virtual void move(const PtrGC<Object>& obj, const Point3& newLoc);
	virtual void clear() { m_objectRoot.destroy(); m_objectRoot = new Object; }

	virtual Point3 gravity() const;
	virtual void setGravity(const Point3& v);

	// special particle functions
	virtual void processParticle(class ParticleEmitter& e, class Particle& p) {};

	Timer& time() { return m_timer; }

	PtrGC<Object> objectAtScreenPoint(const Point2& p) const;

protected:
	friend class DeferredPainter;
	void addDeferredPainter(class D3DPainter& painter);

	virtual void drawScene();
	void standardDrawNode(Object* node);

	Point3			m_gravity;
	std::vector<class D3DPainter*> m_deferredPainters;

	class RenderContext*	m_context;

private:
	typedef stdext::hash_map<const RTTI*, ObjectList > AutoListHash;

	AutoListHash			m_currentAutoList;
	AutoListHash			m_lastAutoList;
	Counter					m_updateLists;

	PtrGC<Object>			m_objectRoot;
	PtrGC<ParticleRenderer>	m_particleRoot;
	Database2D				m_database2D;
	Scene					m_scene;

	PhysicsMgr*				m_physicsMgr;
	EmbeddedPtr<CollisionGroup> m_collisionGroup;

	Timer					m_timer;
};