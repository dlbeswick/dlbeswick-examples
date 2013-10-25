// ---------------------------------------------------------------------------------------------------------
//
// MultiDelegate
// Multicast delegate
//
// ---------------------------------------------------------------------------------------------------------
#pragma once

#include "Standard/api.h"
#include "PendingList.h"
#include "PtrGC.h"
#include "Base.h"

class Base;

// MultiDelegateBase
template <class ClientType>
class TMultiDelegate
{
public:
	typedef PtrGC<ClientType> PtrType;
	typedef void (ClientType::*FuncType)();

	struct Target
	{
		Target() {}

		Target(const PtrType& _target, FuncType _func) :
			target(_target),
			func(_func)
		{
		}

		bool operator == (const Target& rhs) const { return target == rhs.target; }
		operator bool () const { return target != 0; }

		PtrType target;
		typename TMultiDelegate<ClientType>::FuncType func;
	};

	void clear()
	{
		m_targets.clear();
	}

	void remove(PtrType target)
	{
		for (typename Targets::iterator i = m_targets.begin(); i != m_targets.end(); ++i)
		{
			if (i->target == target)
				i.remove();
		}
	}

	template <class F>
	void add(ClientType& obj, F pFunc)
	{ 
		m_targets.add(Target(&obj, (typename TMultiDelegate<ClientType>::FuncType)pFunc));
	}

	void operator()()
	{
		m_targets.flush(); // possible performance issue
		for (uint i = 0; i < m_targets.size(); ++i)
		{
			Target& t = m_targets[i];
			((t.target.ptr())->*(FuncType)(t.func))();
		}
	}

	void operator()() const
	{
		const_cast<TMultiDelegate*>(this)->operator()();
	}

protected:
	typedef PendingListNullRemover<Target> Targets;
	Targets m_targets;
};

template <class ClientType, class P>
class TTPMultiDelegate : public TMultiDelegate<ClientType>
{
public:
	typedef void (Base::*ParamFuncType)(P);

	TTPMultiDelegate& operator << (const typename TMultiDelegate<ClientType>::Target& t)
	{
		this->m_targets.add(t);
		return *this;
	}

	void operator()(P p)
	{
		this->m_targets.flush(); // possible performance issue
		for (uint i = 0; i < this->m_targets.size(); ++i)
		{
			typename TMultiDelegate<ClientType>::Target& t = this->m_targets[i];
			((t.target.ptr())->*(ParamFuncType)(t.func))(p);
		}
	}

	void operator()(P p) const
	{
		const_cast<TMultiDelegate<ClientType>*>(this)->operator()(p);
	}
};

template <class ClientType, class F>
typename TMultiDelegate<ClientType>::Target makeMultiDelegate(ClientType* pObj, F pFunc) { return typename TMultiDelegate<ClientType>::Target(pObj, (typename TMultiDelegate<ClientType>::FuncType)pFunc); }
