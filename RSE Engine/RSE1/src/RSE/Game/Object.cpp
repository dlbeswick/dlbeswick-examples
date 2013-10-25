// ---------------------------------------------------------------------------------------------------------
// 
// Object
// 
// ---------------------------------------------------------------------------------------------------------
#include "pch.h"

#include "Object.h"
#include "Game/Level.h"
#include "Game/ObjectMgr.h"
#include <mm3dnow.h>

IMPLEMENT_RTTI(Object);

ExceptionObject::ExceptionObject(Object& who) : 
	ExceptionStreamRTTI(who)
{
	addContext(who.name());
}

////

std::string Object::_unnamed = "Unnamed";

Object::Object() :
	  m_pos(0,0,0),
	  m_pivot(0,0,0),
	  m_rot(1,0,0,0),
	  m_scale(1,1,1),
	  m_bVisible(true),
	  m_pParent(0),
	  m_level(0),
	  _constructedObject(false)
{
	m_worldPos = Point3(0,0,0);
	m_worldRot = Quat(1,0,0,0);
	m_worldScale = Point3(1,1,1);
}

Object::~Object()
{
	// destroy children
	m_children.flush();
	for (ObjectList::iterator i = m_children.begin(); i != m_children.end(); ++i)
		i->destroy();
}

// worldXForm
const Matrix4& Object::worldXForm() const
{
	if (!xformIsDirty())
		return *m_worldXForm;

	Matrix4 m;
	Matrix4& world = *m_worldXForm;
	world.identity();

	// pivot, scale then rotation
	world.translation(-pivot());

	m.identity();
	m.scale(scale());
	world = world * m;

	rot().toMatrix(m);
	world = world * m;

	// translation
	m.identity();
	m.translation(pos());
	world = world * m;

	if (m_pParent)
		world = world * m_pParent->worldBaseXForm();

	m_worldXForm.setDirty(false);
	return world;
}

// worldBaseXForm
const Matrix4& Object::worldBaseXForm() const
{
	if (!xformIsDirty())
		return *m_worldBaseXForm;

	Matrix4 m;
	Matrix4& world = *m_worldBaseXForm;
	world.identity();

	// rotation
	world.scale(scale());
	rot().toMatrix(m);
	world = world * m;

	// translation
	m.identity();
	m.translation(pos());
	world = world * m;

	if (m_pParent)
		world = world * m_pParent->worldBaseXForm();

	m_worldBaseXForm.setDirty(false);
	return world;
}


// worldPos
const Point3& Object::worldPos() const
{
	if (m_worldPos.dirty())
	{
		m_worldPos = Point3(0,0,0) * worldXForm();
		m_worldPos.setDirty(false);
	}

	return *m_worldPos;
}

// worldRot
const Quat& Object::worldRot() const
{
	if (m_worldRot.dirty())
	{
		if (m_pParent)
		{
			m_worldRot = m_pParent->worldRot();
			*m_worldRot *= m_rot;
		}
		else
		{
			m_worldRot = m_rot;
		}

		m_worldRot.setDirty(false);
	}

	return *m_worldRot;
}

// worldScale
const Point3& Object::worldScale() const
{
	if (m_worldScale.dirty())
	{
		if (m_pParent)
		{
			m_worldScale = m_pParent->worldScale();
			(*m_worldScale).componentMul(m_scale);
		}
		else
		{
			m_worldScale = m_scale;
		}

		m_worldScale.setDirty(false);
	}

	return *m_worldScale;
}

void Object::setParent(Level& l)
{
	setParent(l.objectRoot());
}

void Object::setParent(const PtrGC<Object>& p)
{
	bool wasConstructed = _constructedObject;

	if (p == this)
		EXCEPTIONSTREAM(ExceptionObject(*this), "An attempt was made to make the object a parent of itself.");

	if (!wasConstructed)
	{
		if (!p)
			EXCEPTIONSTREAM(ExceptionObject(*this), "Object can't be constructed unless a parent is supplied. Try supplying a level's object root.");

		constructObject(*p);
		_constructedObject = true;
	}

	if (m_pParent)
	{
		m_pParent->removeChild(this);
	}

	m_pParent = p;

	if (p)
	{
		setLevel(&p->level());
		m_pParent->addChild(this);
	}
	else
	{
		setLevel(0);
	}

	xformDirty();

	if (!wasConstructed)
	{
		constructed();
	}
}

void Object::update(float delta)
{
	Base::update(delta);

	if (m_level)
		m_level->onObjectUpdate(this);

	m_children.flush();

	for (ObjectList::iterator i = m_children.begin(); i != m_children.end(); ++i)
	{
		(*i)->update(delta);
	}
}

void Object::draw()
{
}

void Object::setLevel(Level* l)
{ 
	if (m_level == l)
		return;

	levelChanging(l);

	m_level = l;
	for (ObjectList::iterator i = m_children.begin(); i != m_children.end(); ++i)
	{
		(*i)->setLevel(l);
	}
}

const Point3& Object::pos() const
{ 
	return m_pos; 
}

const Quat& Object::rot() const 
{  
	return m_rot; 
}

const Point3& Object::pivot() const 
{ 
	return m_pivot; 
}

const Point3& Object::scale() const 
{ 
	return m_scale; 
}

void Object::setPos(const Point3& pos) 
{ 
	if (pos != m_pos) 
		xformDirty(); 

	m_pos = pos; 
}

void Object::setRot(const Quat& rot) 
{ 
	if (rot != m_rot) 
		xformDirty(); 
	
	m_worldRot.setDirty(); 
	m_rot = rot; 
}

void Object::setScale(const Point3& scale) 
{ 
	if (scale != m_scale) 
		xformDirty(); 
	
	m_worldScale.setDirty(); 
	m_scale = scale; 
}

void Object::setPivot(const Point3& pivot) 
{ 
	if (pivot != m_pivot) 
		xformDirty(); 
	
	m_pivot = pivot; 
}

bool Object::hasLevel() const
{
	return m_level != 0;
}

Level& Object::level() const		
{ 
	return *m_level; 
}

const std::string& Object::name() const
{ 
	if (m_name.empty())
		return _unnamed;
	else
		return m_name; 
}

void Object::setName(const std::string& n)
{ 
	m_name = n; 
}

bool Object::visible() const
{ 
	return m_bVisible; 
}

void Object::setVisible(bool b)
{ 
	m_bVisible = b; 
}

const PtrGC<Object>& Object::parent() const
{ 
	return m_pParent; 
}

const Object::ObjectList& Object::children() const
{ 
	return m_children; 
}

Point3 Object::extent() const
{ 
	return Point3(0, 0, 0); 
}

Point3 Object::geomCenter() const
{ 
	return Point3(0, 0, 0); 
}

void Object::addChild(const PtrGC<Object>& p)
{ 
	m_children.add(p); 
}

void Object::removeChild(const PtrGC<Object>& p)
{ 
	m_children.remove(p); 
}

void Object::xformDirty() const
{
	onXFormDirty();

	m_worldXForm.setDirty();
	m_worldBaseXForm.setDirty();
	m_worldPos.setDirty();
	m_worldRot.setDirty();
	m_worldScale.setDirty();

	for (ObjectList::const_iterator i = m_children.begin(); i != m_children.end(); ++i)
	{
		(*i)->xformDirty();
	}
}

bool Object::xformIsDirty() const
{
	return m_worldXForm.dirty() || m_worldBaseXForm.dirty();
}

void Object::constructObject(Object& parent)
{
	Super::construct();
}

void Object::constructed()
{
}

void Object::levelChanging(Level* l)
{
}

// Objects call construct when setParent is invoked, so construct is not necessary after newObject.
bool Object::needPostNewObjectConstructCall() const
{
	return false;
}
