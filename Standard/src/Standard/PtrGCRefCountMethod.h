// ---------------------------------------------------------------------------------------------------------
//
// PtrGCRefCountMethod
//
// PtrGC implementation using reference counted smart pointers.
//
// PtrGCs can only hold objects that are derived from class PtrGCHost.
//
// ---------------------------------------------------------------------------------------------------------
#ifndef STANDARD_PTRGCREFCOUNTMETHOD_H
#define STANDARD_PTRGCREFCOUNTMETHOD_H

#include "Standard/api.h"
#include <new>
#include "Standard/EmbeddedPtr.h"
#include "Standard/PtrGCBase.h"

// PtrGC Class
template <class T>
class PtrGC : public TEmbeddedPtr<T, PtrGCBase>
{
public:
	typedef TEmbeddedPtr<T, PtrGCBase> Super;

	// construct/copy
	PtrGC()
	{
	}

	PtrGC(const PtrGC& ptr) :
		Super(ptr)
	{
	}

	PtrGC(const T* ptr) :
		Super(ptr)
	{
	}

	PtrGC& operator = (const PtrGC& p)
	{
		Super::operator=(p);
		return *this;
	}

	template<class AnyT>
	PtrGC<AnyT> downcast() const
	{
		return PtrGC<AnyT>((AnyT*)this->ptr());
	}

	template <class PT>
	operator PtrGC<PT>() const
	{
		PT* c = this->ptr(); // compile error here if conversion is unsafe, use downcast function if necessary
		return PtrGC<PT>(c);
	}
};

#endif
