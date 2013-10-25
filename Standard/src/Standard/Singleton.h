#pragma once

#include "Standard/api.h"
#include "SmartPtr.h"

// ---------------------------------------------------------------------------------------------------------
//
// Singleton
//
// ---------------------------------------------------------------------------------------------------------

template <class T>
class Singleton
{
public:
	Singleton()
	{
		m_pInstance = (T *)this;
	}

	static T &instance() { return *m_pInstance; }

private:
	static T* m_pInstance;
};


template <class T> T* Singleton<T>::m_pInstance;

// accessor helper
#define SINGLETON(x) inline S##x &x() { return S##x::instance(); }
#define CSINGLETON(x) inline C##x &x() { return C##x::instance(); }

// ---------------------------------------------------------------------------------------------------------
//
// SingletonMgr
// Help for basic manager-style singletons
//
// ---------------------------------------------------------------------------------------------------------

template <class T, class type>
class SingletonMgr : public Singleton<T>
{
public:
	SingletonMgr()
	{
	}

	~SingletonMgr()
	{
		m_pCurrent = 0;
	}

	// change to operator = and operator ->
	void setCurrent(type* var) { m_pCurrent = var; }
	type& getCurrent() { return *m_pCurrent; }

private:
	static SmartPtr<type> m_pCurrent;
};


template <class T, class type> SmartPtr<type> SingletonMgr<T, type>::m_pCurrent;
