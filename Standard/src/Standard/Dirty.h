// ---------------------------------------------------------------------------------------------------------
//
// Dirty
// Encapsulates dirty variable concept
//
// ---------------------------------------------------------------------------------------------------------
#pragma once

#include "Standard/api.h"

template<class T>
class Dirty
{
public:
	Dirty() :
	  m_bDirty(true)
	{
	}

	T& operator *() { return m_var; }
	const T& operator *() const { return m_var; }
	T* operator ->() { return &m_var; }
	T& operator =(const T& val) { m_var = val; m_bDirty = false; return m_var; }

	bool dirty() { return m_bDirty; }
	void setDirty(bool b = true) { m_bDirty = b; }

private:
	bool m_bDirty;
	T m_var;
};
