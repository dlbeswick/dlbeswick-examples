#include "Standard/pch.h"
#include "PtrGC.h"
#include "Log.h"
#include "Delegate.h" // temp
#include "Base.h" // temp

SPtrGCManager::SPtrGCManager()
{
	resolver = new PtrGCGlobals::Resolver;
	garbage = new PtrGCGlobals::GarbageWatcher;
#if PTRGC_CHECKLEVEL
	dlogref_internal = get_dlog_ref(); // workaround for global order-of-initialization/destruction problem.
#endif
}

SPtrGCManager::~SPtrGCManager()
{
	delete garbage;
	delete resolver;
#if PTRGC_CHECKLEVEL
	if (!debugAssignCheck.empty())
		*dlogref_internal << "PTRGC: " << (int)debugAssignCheck.size() << " ptrs unfreed" << dlog.endl;
	else
		*dlogref_internal << "PtrGC cleanup ok" << dlog.endl;
#endif
}

SPtrGCManager ptrGCManager;

/*static void PtrGCUnitTest()
{
	PtrGC<PtrGCHost> hi;
	hi->self();

	PtrGC<Base> ho;
	Delegate d; // temp
	d = Delegate(ho.ptr(), &Base::construct);
}*/
