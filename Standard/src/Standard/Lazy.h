// ------------------------------------------------------------------------------------------------
//
// Lazy
// Encapsulates lazy evaluation concept
//
// ------------------------------------------------------------------------------------------------
#pragma once

#include "Standard/api.h"

// TBD: lazy heap alloc class using pointer?
template <class T>
class Lazy
{
public:
	Lazy() :
	  m_bEval(false)
	{
	}
	
	operator T&() { return m_var; }
	operator const T&() const { return m_var; }
	T* operator ->() { return &m_var; }
	T& operator =(const T& val) { m_var = val; m_bEval = true; return m_var; }
	
	bool evaluated() { return m_bEval; }
	void setEvaluated(bool b = true) { m_bEval = b; }

private:
	bool m_bEval;
	T m_var;
};