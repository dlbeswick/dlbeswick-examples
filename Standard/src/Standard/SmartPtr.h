// ---------------------------------------------------------------------------------------------------------
//
// Ptr
//
// Smart pointer class
// When smart pointers are cast to pointers and then assigned to other smart pointers, terrible things
// happen.
//
// No two smart pointers should be created from the same standard pointer.
// The debug build contains code to assert if two smart pointers are created from the same pointer.
//
// ---------------------------------------------------------------------------------------------------------
#pragma once


#include "Standard/api.h"
// debug smart checks
#ifdef USE_PTR_SMARTCHECKS
#include <assert.h>
#include <set>

#if !COMPILING_DLL
	#define IMPLEMENT_PTR_SMARTCHECKS \
	namespace PtrDebug \
	{ \
		std::set<char*> ptrRecord; \
	}
#else
	#define IMPLEMENT_PTR_SMARTCHECKS \
	namespace PtrDebug \
	{ \
		_declspec(dllexport) std::set<char*> ptrRecord; \
	}
#endif

namespace PtrDebug
{
#if COMPILING_DLL
	extern _declspec(dllexport) std::set<char*> ptrRecord;
#elif IMPORTING_DLL
	extern _declspec(dllimport) std::set<char*> ptrRecord;
#else
	extern std::set<char*> ptrRecord;
#endif
}
#endif



template <class T>
class SmartPtr
{
public:
	typedef T* P;

	// construct/copy
	SmartPtr() : m_pRefCount(0), m_ptr(0)
	{
	}

	template<class AnyT>
	SmartPtr(const SmartPtr<AnyT>& ptr) : m_pRefCount(0), m_ptr(0)
	{
		assign(ptr);
	}

	SmartPtr(const P pPtr) : m_pRefCount(0), m_ptr(0)
	{
		assign(pPtr);
	}

	SmartPtr(const SmartPtr<T>& ptr)
	{
		// prevent self-assignment
		if (&ptr != this)
		{
			m_pRefCount = 0;
			m_ptr = 0;
			assign(ptr);
		}
	}

	// special constructor for upcast, no typical use
	SmartPtr(const P pPtr, int* pRefCount) : m_pRefCount(pRefCount), m_ptr(pPtr)
	{
		incRef();
	}

	SmartPtr<T>& operator = (const P p)
	{
		assign(p);
		return *this;
	}

	template<class AnyT>
	SmartPtr<T>& operator = (const SmartPtr<AnyT>& ptr)
	{
		assign(ptr);
		return *this;
	}

	// If this duplicated assignment operator is not present, the compiler will generate a default constructor
	// when assigning two Ptrs of the same type. VC 6 Compiler bug?
	SmartPtr<T>& operator = (const SmartPtr<T>& ptr)
	{
		assign(ptr);
		return *this;
	}

	// comparison
	bool operator == (const SmartPtr<T>& rhs) const	{ return m_ptr == rhs.m_ptr; }
	bool operator != (const SmartPtr<T>& rhs) const	{ return m_ptr != rhs.m_ptr; }
	bool operator < (const SmartPtr<T>& rhs) const	{ return m_ptr < rhs.m_ptr; }
	bool operator > (const SmartPtr<T>& rhs) const	{ return m_ptr > rhs.m_ptr; }
	bool operator == (const P& rhs) const	{ return m_ptr == rhs; }
	bool operator != (const P& rhs) const	{ return m_ptr != rhs; }
	bool operator < (const P& rhs) const	{ return m_ptr < rhs; }
	bool operator > (const P& rhs) const	{ return m_ptr > rhs; }

	// destruct
	~SmartPtr()
	{
#ifdef USE_PTR_SMARTCHECKS
		if (m_pRefCount)
		{
			assert(*m_pRefCount >= 0);
		}
#endif

		decRef();
	}

	// access
	T& operator*() const { return *m_ptr; }
	P operator ->()	const { return m_ptr; }
	operator bool() const { return m_ptr != 0; }

	// downcast
	template<class AnyT>
	SmartPtr<AnyT> downcast() const
	{
		return SmartPtr<AnyT>((AnyT*)m_ptr, m_pRefCount);
	}

	template<class AnyT>
	SmartPtr<AnyT> cast() const
	{
		return SmartPtr<AnyT>(m_ptr, m_pRefCount);
	}

	// query
	int* refCount() const { return m_pRefCount; }
	T*const ptr() const { return m_ptr; }
	T* ptr() { return m_ptr; }

protected:
	void incRef()
	{
		if (m_ptr && m_pRefCount)
			(*m_pRefCount)++;
	}

	void decRef()
	{
		if (m_ptr && m_pRefCount)
			(*m_pRefCount)--;

		validate();
	}

	void assign(const P p)
	{
		decRef();

		if (p)
		{
#ifdef USE_PTR_SMARTCHECKS
			// ASSERT: pointer has already been assigned to another smart pointer
			assert(PtrDebug::ptrRecord.insert((char*)p).second == true);
#endif

			m_pRefCount = new int;
			*m_pRefCount = 0;

			m_ptr = p;
			incRef();
		}
		else
		{
			m_ptr = 0;
			m_pRefCount = 0;
		}
	}

	// template function, so a smart pointer can be cast to a smart pointer of base type
	template<class AnyT>
	void assign(const SmartPtr<AnyT>& p)
	{
		decRef();

		m_pRefCount = p.refCount();
		m_ptr = p.ptr();

		incRef();
	}

	void validate()
	{
		if (m_pRefCount && *m_pRefCount == 0)
		{
			if (m_ptr)
				delete m_ptr;

			if (m_pRefCount)
				delete m_pRefCount;

#ifdef USE_PTR_SMARTCHECKS
			PtrDebug::ptrRecord.erase((char*)m_ptr);
#endif
		}
	}

private:
	P		m_ptr;
	int*	m_pRefCount;
};
