// ---------------------------------------------------------------------------------------------------------
// 
// PtrD3D
//
// Smart pointer class for managed Direct3D resources
//
// PtrD3D objects share a pointer to a second pointer. The pointer-to-pointer is allocated only once, when a
// raw pointer is assigned to a PtrD3D. So, similarly to the Ptr class, no one raw pointer should be assigned
// to two different PtrD3D objects.
//
// No two smart D3D pointers should be created from the same raw pointer.
// The debug build contains code to assert if two smart pointers are created from the same pointer.
// 
// ---------------------------------------------------------------------------------------------------------
#pragma once

#include "Standard/api.h"
// include SmartPtr.h for debug smart checking
#include "SmartPtr.h"

template <class T>
class PtrD3D
{
public:
	typedef T* P;

	// construct/copy
	PtrD3D() : m_pRefCount(0), m_pPtrD3D(0)
	{
	}

	template<class PT>
	PtrD3D(const PtrD3D<PT>& ptr) : m_pRefCount(0), m_pPtrD3D(0)
	{
		assign(ptr);
	}

	PtrD3D(const P pPtrD3D) : m_pRefCount(0), m_pPtrD3D(0)
	{
		assign(pPtrD3D);
	}

	PtrD3D(const PtrD3D<T>& ptr) : m_pRefCount(0), m_pPtrD3D(0)
	{
		assign(ptr);
	}

	PtrD3D<T>& operator = (const P pPtrD3D)
	{
		assign(pPtrD3D);
		return *this;
	}

	template<class PT>
	PtrD3D<T>& operator = (const PtrD3D<PT>& ptr)
	{
		assign(ptr);
		return *this;
	}

	// If this duplicated assignment operator is not present, the compiler will generate a default constructor
	// when assigning two PtrD3Ds of the same type. VC 6 Compiler bug?
	PtrD3D<T>& operator = (const PtrD3D<T>& ptr)
	{
		assign(ptr);
		return *this;
	}

	// comparison
	bool operator == (const PtrD3D<T>& rhs) const	{ return m_pPtrD3D == rhs.m_pPtrD3D; }
	bool operator != (const PtrD3D<T>& rhs) const	{ return m_pPtrD3D != rhs.m_pPtrD3D; }
	bool operator < (const PtrD3D<T>& rhs) const	{ return m_pPtrD3D < rhs.m_pPtrD3D; }
	bool operator > (const PtrD3D<T>& rhs) const	{ return m_pPtrD3D > rhs.m_pPtrD3D; }

	// destruct
	~PtrD3D()
	{
		if (m_pRefCount)
		{
			assert(*m_pRefCount >= 0);
		}

		decRef();
	}

	// cast
	T& operator*() const { return **m_pPtrD3D; }
	P operator ->()	const { return *m_pPtrD3D; }
	operator bool() const { return m_pPtrD3D != 0; }

	// query
	int* refCount() const { return m_pRefCount; }
	T*const& ptr() const { return *m_pPtrD3D; }
	T*& ptr() { return *m_pPtrD3D; }

	// remap - will make all pointers referencing the old object reference the new object
	// use with care
	void remap(T* p)
	{
		PtrD3D<T> ptr = p;
		remap(ptr);
	}

	void remap(const PtrD3D<T>& ptr)
	{
		if (!m_pPtrD3D)
		{
			assign(ptr);
			return;
		}

#ifdef USE_PTR_SMARTCHECKS
		PtrDebug::ptrRecord.erase((char*)*m_pPtrD3D);
#endif
		if (ptr.m_pPtrD3D)
			*m_pPtrD3D = *ptr.m_pPtrD3D;
		else
			*m_pPtrD3D = 0;

		// unify the references to the refcount and d3d ptr between both smart pointers.
		delete ptr.m_pRefCount;
		delete ptr.m_pPtrD3D;

		// it's convenient to be able to pass const pointers to this func, so const cast even though it's misleading.
		const_cast<PtrD3D<T>&>(ptr).m_pRefCount = m_pRefCount;
		const_cast<PtrD3D<T>&>(ptr).m_pPtrD3D = m_pPtrD3D;
		(*m_pRefCount)++;
#ifdef USE_PTR_SMARTCHECKS
		PtrDebug::ptrRecord.insert((char*)*m_pPtrD3D);
#endif
	}

	void release()
	{
		if (*m_pPtrD3D)
			(*m_pPtrD3D)->Release();

		remap(0);
	}

protected:
	void incRef()
	{
		if (m_pPtrD3D && m_pRefCount)
			(*m_pRefCount)++;
	}

	void decRef()
	{
		if (m_pPtrD3D && m_pRefCount)
			(*m_pRefCount)--;

		validate();
	}

	void assign(const P pPtrD3D)
	{
		decRef();

		if (pPtrD3D)
		{
#ifdef USE_PTR_SMARTCHECKS
			// ASSERT: pointer has already been assigned to another smart pointer
			assert(PtrDebug::ptrRecord.insert((char*)pPtrD3D).second == true);
#endif

			m_pRefCount = new int;
			*m_pRefCount = 0;
	
			m_pPtrD3D = new T*;
			*m_pPtrD3D = pPtrD3D;
			incRef();
		}
		else
		{
			m_pPtrD3D = 0;
			m_pRefCount = 0;
		}
	}

	// template function - so a smart pointer can be cast to a smart pointer of base type
	template<class PT>
	void assign(const PtrD3D<PT>& ptr)
	{
		if (&ptr == this)
			return;

		decRef();

		m_pRefCount = ptr.refCount();
		m_pPtrD3D = (PT**)ptr.m_pPtrD3D;

		incRef();
	}

	void validate()
	{
		if (m_pRefCount && *m_pRefCount == 0)
		{
			if (m_pPtrD3D)
			{
				if (*m_pPtrD3D)
				{
					(*m_pPtrD3D)->Release();
				}

				delete m_pPtrD3D;
			}

			if (m_pRefCount)
				delete m_pRefCount;

#ifdef USE_PTR_SMARTCHECKS
			PtrDebug::ptrRecord.erase((char*)*m_pPtrD3D);
#endif
		}
	}

private:
	P*		m_pPtrD3D;
	int*	m_pRefCount;
};
