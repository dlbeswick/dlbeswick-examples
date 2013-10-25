// ---------------------------------------------------------------------------------------------------------
// 
// ObjectMgr
// 
// ---------------------------------------------------------------------------------------------------------
#include "pch.h"

#include "ObjectMgr.h"
#include "Object.h"
#include "Render/SDeviceD3D.h"
#include "Exception/Video.h"


// update
void ObjectMgr::update(float delta)
{
	m_objects.flush();

	for (ObjectList::iterator i = m_objects.begin(); i < m_objects.end(); ++i)
	{
		(*i)->update(delta);
	}
}

// draw
void ObjectMgr::draw()
{
	DX_ENSURE(D3DD().SetRenderState(D3DRS_LIGHTING, true));

	D3D().zbuffer(true);

	for (ObjectList::iterator i = m_objects.begin(); i < m_objects.end(); ++i)
	{
		(*i)->draw();
	}
}

// add
void ObjectMgr::add(const PtrGC<Object>& pObj)
{
	pObj->setLevel(&m_level);

	m_objects.add(pObj);
}

// remove
void ObjectMgr::remove(const PtrGC<Object>& pObj)
{
	m_objects.remove(pObj);
}
