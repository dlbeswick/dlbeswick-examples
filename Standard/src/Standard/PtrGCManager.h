#ifndef STANDARD_PTRGCMANAGER_H
#define STANDARD_PTRGCMANAGER_H

#define PTRGC_CHECKLEVEL _DEBUG

#if PTRGC_CHECKLEVEL
#include <Standard/Log.h>
#endif

namespace PtrGCGlobals
{
	class Resolver;
	class GarbageWatcher;
}

class STANDARD_API SPtrGCManager
{
public:
	SPtrGCManager();
	~SPtrGCManager();

	PtrGCGlobals::Resolver* resolver;
	PtrGCGlobals::GarbageWatcher* garbage;
#if PTRGC_CHECKLEVEL
	stdext::hash_set<void*> debugAssignCheck;
	SmartPtr<dlogstreamstdout> dlogref_internal;
#endif
};

extern STANDARD_API SPtrGCManager ptrGCManager;

#endif
