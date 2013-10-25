// ---------------------------------------------------------------------------------------------------------
// 
// ObjectMgr
// 
// ---------------------------------------------------------------------------------------------------------
#pragma once


#include "Object.h"
#include "Standard/PendingList.h"


class RSE_API ObjectMgr
{
public:
	ObjectMgr(class Level& level) :
	  m_level(level)
	{
	}

	void draw();
	void update(float delta);

	void add(const PtrGC<Object>& pObj);
	void remove(const PtrGC<Object>& pObj);
	void clear()							{ m_objects.clear(); }

protected:
private:
	typedef PendingListNullRemover<PtrGC<Object> > ObjectList;
	ObjectList		m_objects;
	class Level&	m_level;
};