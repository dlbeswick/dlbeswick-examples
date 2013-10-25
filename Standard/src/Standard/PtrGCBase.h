// ------------------------------------------------------------------------------------------------
//
// PtrGCBase
// Base class from which PtrGC objects
// ------------------------------------------------------------------------------------------------
#ifndef STANDARD_PTRGCBASE_H
#define STANDARD_PTRGCBASE_H

#include "Standard/api.h"
#include "Standard/EmbeddedPtr.h"
#include "Standard/PtrGCManager.h"

class PtrGCHost;
class IPtrGCHost;
class CriticalSection;

class STANDARD_API PtrGCBase : public EmbeddedPtrBase
{
public:
	PtrGCBase(const PtrGCHost* ptr);
	PtrGCBase(const IPtrGCHost* ptr);

	// destroy object and zero all references
	void destroy() const;

protected:
	PtrGCBase()
	{}
};

// Stream Resolver
namespace PtrGCGlobals
{
	class STANDARD_API Resolver
	{
	public:
		~Resolver();

		void registerMapping(void* oldPointer, void* newPointer);
		void resolve(void* oldPointer, PtrGCBase& ptr);
		
		// called with garbage collect
		void reset();

	private:
		typedef stdext::hash_map<void*, void*> ResolveMap;
		typedef stdext::hash_multimap<void*, PtrGCBase*> PendingMap;

		ResolveMap m_resolveMap;
		PendingMap m_pending;
	};
}

// garbage collection
namespace PtrGCGlobals
{
	class STANDARD_API GarbageWatcher
	{
	public:
		GarbageWatcher();
		~GarbageWatcher();

		void collect();
		void collectAll();

		void add(PtrGCHost* ptr);

		void setDeferredGC(bool b);

	private:
		typedef std::vector<PtrGCHost*> Garbage;

		void cleanupGarbage();

		Garbage m_garbage;
		bool m_deferredGC;
		CriticalSection* m_critical;
	};
}

#endif
