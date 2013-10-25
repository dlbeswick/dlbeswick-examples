// ------------------------------------------------------------------------------------------------
//
// Controller
// A controller enables players or AIs to control an object using a common interface, and represents
// an entity that can control a range of objects.
// One controller should be defined per "entity" (player, ai, etc).
// ControlMethods should then be added to the controller: one per type of object that the entity can control.
// When the controller possesses an object, the correct ControlMethod class for the object is selected. 
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/Game/Object.h"
#include "Standard/Registrar.h"
#include "Standard/Base.h"

class Object;
class PancakeLevel;

// ControlMethodBase
class ControlMethodBase : public Base, public PtrGCHost
{
	USE_RTTI(ControlMethodBase, Base);
public:
	virtual bool activate(class IControllable* o, PancakeLevel& level) = 0;

protected:
	virtual void activated(class PancakeLevel& level) {}
};

// ControlMethod
// Derive from this. When method is activated, m_obj is automatically set to your object type
template <class ObjectType>
class ControlMethod : public ControlMethodBase
{
public:
	virtual bool activate(class IControllable* o, PancakeLevel& level)
	{
		m_obj = Cast<ObjectType>(o);
		if (m_obj)
			activated(level);

		return m_obj != 0;
	}
	const PtrGC<ObjectType>& obj() { return m_obj; }

protected:
	PtrGC<ObjectType> m_obj;
};

// IControllable
class IControllable
{
	USE_RTTI(IControllable, IControllable);
public:
	virtual ~IControllable();

	PtrGC<class Controller>& controller() { return m_controller; };

private:
	friend class Controller;
	PtrGC<class Controller> m_controller;
};

// Controller
class Controller : public Object
{
	USE_RTTI(Controller, Object);

public:
	Controller() :
		m_method(0)
	{
	}
	
	~Controller()
	{
		for (MethodMap::iterator i = m_methods.begin(); i != m_methods.end(); ++i)
			i->second.destroy();
	}

	virtual void update(float delta);
	virtual bool possess(IControllable* o);
	virtual PtrGC<ControlMethodBase> methodOfType(const RTTI& rtti);

	PtrGC<ControlMethodBase> active() const { return m_method; }

protected:
	void addMethod(const RTTI& type, ControlMethodBase* m) { m_methods[&type] = m; }
	IControllable* m_obj;
	PtrGC<ControlMethodBase> m_method;

private:
	friend class IControllable;

	typedef stdext::hash_map<const RTTI*, PtrGC<ControlMethodBase> > MethodMap;
	MethodMap m_methods;
};
