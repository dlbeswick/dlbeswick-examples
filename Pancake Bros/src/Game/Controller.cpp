// ------------------------------------------------------------------------------------------------
//
// Controller
//
// ------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "Controller.h"
#include "PancakeLevel.h"

IMPLEMENT_RTTI(ControlMethodBase);

IMPLEMENT_RTTI(IControllable);

IControllable::~IControllable()
{
	if (m_controller)
		m_controller->m_obj = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_RTTI(Controller);

void Controller::update(float delta)
{
	if (!m_obj)
		m_method = 0;

	if (m_method)
	{
		m_method->update(delta);
	}
}

bool Controller::possess(IControllable* o)
{
	// Controller must have a parent before possession.
	assert(parent());
	// not meaningful to possess controllers other than ourselves
	//if (!o || (o->rtti().isA(Controller::static_rtti()) && o != this))
	//	return false;

	if (!o)
	{
		m_obj->m_controller = 0;
		m_obj = 0;
		return true;
	}

	for (MethodMap::iterator i = m_methods.begin(); i != m_methods.end(); ++i)
	{
		if (o->rtti().isA(*i->first))
		{
			if (i->second->activate(o, level().cast<PancakeLevel>()))
			{
				m_method = i->second;
				m_obj = o;
				m_obj->m_controller = this;
				return true;
			}
		}
	}

	dlog << rtti().className() << " couldn't possess object of type " << o->rtti().className() << dlog.endl;
	return false;
}

PtrGC<ControlMethodBase> Controller::methodOfType(const RTTI& rtti)
{
	MethodMap::iterator i = m_methods.begin();

	for (; i != m_methods.end(); ++i)
	{
		if (i->second->rtti().isA(rtti))
			break;
	}

	if (i == m_methods.end())
		return 0;

	return i->second;
}
