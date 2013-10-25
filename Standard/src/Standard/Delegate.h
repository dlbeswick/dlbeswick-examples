// ---------------------------------------------------------------------------------------------------------
//
// Delegate
//
// ---------------------------------------------------------------------------------------------------------
#pragma once

#include "Standard/api.h"

// use this macro to create delegate objects
#define delegate(x) makeDelegate(this, x)

// DelegateBase
class Base;

template <class T>
class DelegateBase
{
public:
	typedef void (T::*BaseFuncType)();

	DelegateBase() :
		m_pOwner(0),
		m_pFunc(0)
	{}

	void clear()
	{
		m_pOwner = 0;
		m_pFunc = 0;
	}

	BaseFuncType func() { return m_pFunc; }
	bool valid() const { return m_pOwner && m_pFunc; }
	operator bool () const { return valid(); }

	bool operator == (const DelegateBase& rhs) const { return m_pOwner == rhs.m_pOwner && m_pFunc == rhs.m_pFunc; }
	bool operator != (const DelegateBase& rhs) const { return m_pOwner != rhs.m_pOwner || m_pFunc != rhs.m_pFunc; }

protected:
	T* m_pOwner;
	BaseFuncType m_pFunc;
};

// Delegate
template <class T>
class TDelegate : public DelegateBase<T>
{
public:
	typedef void (T::*FuncType)();

	TDelegate() {}

	TDelegate(T *pOwner, FuncType pFunc)
	{
		this->m_pOwner = pOwner;
		this->m_pFunc = (typename DelegateBase<T>::BaseFuncType)pFunc;
	}

	void operator()()
	{
		if (this->valid()) (DelegateBase<T>::m_pOwner->*(FuncType)DelegateBase<T>::func())();
	}
};

typedef TDelegate<class Base> Delegate;

template <class D>
Delegate makeDelegate(Base* pObj, void (D::*pFunc)()) { return Delegate(pObj, (void(Base::*)())pFunc); }

// TPDelegate
// PDelegate, delete with parameter
template <class P, class T = class Base>
class TPDelegate : public DelegateBase<T>
{
public:
	typedef void (T::*FuncType)(P);

	TPDelegate() {}

	TPDelegate(T *pOwner, FuncType pFunc)
	{
		this->m_pOwner = pOwner;
		this->m_pFunc = (typename DelegateBase<T>::BaseFuncType)pFunc;
	}

	void operator()(P a)
	{
		if (DelegateBase<T>::valid()) (DelegateBase<T>::m_pOwner->*(FuncType)DelegateBase<T>::func())(a);
	}
};

// call this to create a delegate in a derived class
template <class D, class P>
TPDelegate<P, Base> makeDelegate(Base* pObj, void (D::*pFunc)(P)) { return TPDelegate<P, Base>(pObj, (void(Base::*)(P))pFunc); }

