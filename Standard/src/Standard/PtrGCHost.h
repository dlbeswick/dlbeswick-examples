// ------------------------------------------------------------------------------------------------
//
// PtrGCHost
// Base class for objects that can be pointed to by PtrGC objects.
//
// ------------------------------------------------------------------------------------------------
#ifndef STANDARD_PTRGCHOST_H
#define STANDARD_PTRGCHOST_H

#include "Standard/api.h"
#include "Standard/EmbeddedPtr.h"

class PtrGCBase;

namespace PtrGCGlobals
{
	class GarbageWatcher;
}

class STANDARD_API IPtrGCHost : virtual public IEmbeddedPtrHost
{
public:
};

class STANDARD_API PtrGCHost : public EmbeddedPtrHost, virtual public IPtrGCHost
{
public:
	PtrGCBase& self() const;

private:
	virtual const class EmbeddedPtrHost* host() const { return this; }
	virtual class EmbeddedPtrHost* host() { return this; }
	virtual void onDestroy() {}

	friend class PtrGCBase;
	friend class PtrGCGlobals::GarbageWatcher;
};

#endif
