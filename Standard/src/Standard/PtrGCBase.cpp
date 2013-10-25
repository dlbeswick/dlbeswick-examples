// ------------------------------------------------------------------------------------------------
//
// PtrGCBase
//
// ------------------------------------------------------------------------------------------------
#include "Standard/pch.h"
#include "PtrGCBase.h"
#include "PtrGCHost.h"
#include "CriticalSectionBlock.h"
#include "binstream.h"
#include "textstream.h"
#include "streamops.h"
#include "Log.h"

using namespace PtrGCGlobals;

PtrGCBase::PtrGCBase(const PtrGCHost* ptr)
{
	assign(ptr);
}

PtrGCBase::PtrGCBase(const IPtrGCHost* ptr) :
	EmbeddedPtrBase(ptr)
{
}

void PtrGCBase::destroy() const
{
	if (_ptr && *_ptr)
	{
		((PtrGCHost*)*_ptr)->onDestroy();
		ptrGCManager.garbage->add((PtrGCHost*)*_ptr);
	}
}

PtrGCBase& PtrGCHost::self() const
{
	return *(PtrGCBase*)(this->_self);
}

Resolver::~Resolver()
{
}

void Resolver::registerMapping(void* oldPointer, void* newPointer)
{
	assert(0);
	if (!oldPointer)
		return;

	// TODO
	/*
	// record the mapping
	assert(m_pending.find(oldPointer) == m_pending.end());
	m_resolveMap.insert(std::pair<void*, void*>(oldPointer, newPointer));

	// resolve any pointers that had no mapping at the time of their streaming
	std::pair<PendingMap::iterator, PendingMap::iterator> pendingI = m_pending.equal_range(oldPointer);
	for (PendingMap::iterator i = pendingI.first; i != pendingI.second; ++i)
	{
		PtrGCBase& ptr = *i->second;
		ptr.m_ptr = newPointer;
		ptrGCManager.refMap->map().insert(std::pair<void*, PtrGCBase*>(ptr.m_ptr, &ptr));
	}
	if (pendingI.first != pendingI.second)
		m_pending.erase(pendingI.first, pendingI.second);
	*/
}

void Resolver::resolve(void* oldPointer, PtrGCBase& ptr)
{
	assert(0);
	if (!oldPointer)
		return;

	// TODO
	/*
	ResolveMap::iterator i = m_resolveMap.find(oldPointer);
	if (i != m_resolveMap.end())
	{
		ptr.m_ptr = i->second;
		ptrGCManager.refMap->map().insert(std::pair<void*, PtrGCBase*>(ptr.m_ptr, &ptr));
	}
	else
	{
		// couldn't find a mapping so far, add us to the 'pending resolve' list
		m_pending.insert(std::pair<void*, PtrGCBase*>(oldPointer, &ptr));
	}
	*/
}

// called with garbage collect
void Resolver::reset()
{
	assert(m_pending.empty());
	m_resolveMap.clear();
}

GarbageWatcher::GarbageWatcher() :
	m_deferredGC(true),
	m_critical(new CriticalSection)
{
}

void GarbageWatcher::collect()
{
	Critical(*m_critical);

	static bool bCollecting = false;

	if (bCollecting)
	{
		assert(0);
		dlog << "Re-entrant call to garbage collection." << dlog.endl;
	}

	bCollecting = true;

	cleanupGarbage();
	ptrGCManager.resolver->reset();

	bCollecting = false;
}

void GarbageWatcher::collectAll()
{
	Critical(*m_critical);
	do
	{
		collect();
	} while (m_garbage.size() > 0);
}

GarbageWatcher::~GarbageWatcher()
{
	{
		Critical(*m_critical);
		collectAll();
	}

	delete m_critical;
}

void GarbageWatcher::add(PtrGCHost* ptr)
{
	Critical(*m_critical);
	m_garbage.push_back(ptr);
	*(ptr->_masterRef) = 0;

	if (!m_deferredGC)
		collect();
}

void GarbageWatcher::setDeferredGC(bool b)
{
	Critical(*m_critical);
	m_deferredGC = b;
}

void GarbageWatcher::cleanupGarbage()
{
	Garbage destructList = m_garbage;

	// 'destruct' cannot happen while iterating through garbage as more garbage may be added during collection
	m_garbage.clear();

	for (Garbage::iterator i = destructList.begin(); i != destructList.end(); ++i)
	{
		delete *i;
	}
}
