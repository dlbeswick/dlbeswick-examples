// ------------------------------------------------------------------------------------------------
//
// StateMachine
//
// ------------------------------------------------------------------------------------------------
#pragma once

#include "Standard/api.h"
#include "Log.h"
#include "Base.h"

// don't care about this warning
#if IS_MSVC
	#pragma warning(disable:4355)
#endif

// default state type.
// derive from this to make new states.
struct STANDARD_API StateBase : public Base
{
	virtual void start() {}
	virtual void update(float delta) { Base::update(delta); }
	virtual void end() {}

	const char* name;
};

template <class T>
struct TState : public StateBase
{
	typedef T ObjType;

	T* o;
};

class STANDARD_API StateMachineBase
{
public:
	StateMachineBase();
	~StateMachineBase();

	void go(StateBase* state, bool force = false);
	void update(float delta);

	StateBase* null() const { return m_nullState; }

protected:
	typedef std::vector<StateBase*> States;

	StateBase* m_curState;
	StateBase* m_nullState;
	States m_states;
};

template <class StateClass>
class StateMachine : public StateMachineBase
{
public:
	typedef typename StateClass::ObjType OwnerClass;

	StateMachine() :
	  m_owner(0)
	{
	}

	void setOwner(OwnerClass* owner)
	{
		m_owner = owner;
		m_nullState = new StateClass;
		add((StateClass*)m_nullState, "0");
	}

	void add(StateClass* state, const char* name)
	{
		assert(m_owner);

		state->o = m_owner;
		state->name = name;
		m_states.push_back(state);
	}

	StateClass* operator* () const { return (StateClass*)m_curState; }
	StateClass* operator-> () const { return (StateClass*)m_curState; }

protected:
	OwnerClass* m_owner;
};

#define DECLARE_STATECLASS(x) typedef TState<x> State;
#define EXTEND_STATECLASS(x) struct State : public TState<x>
#define DECLARE_STATEMACHINE StateMachine<State> states;

#define EXTEND_STATE(x, Base) \
	struct State##x; \
	State##x* x; \
	virtual State##x* ConstructState##x() { return new State##x; } \
	struct State##x : public Base

#define STATE(x) EXTEND_STATE(x, State)

#define BEGIN_STATEMACHINE states.setOwner(this);
#define ADD_STATE(x) x = ConstructState##x(); states.add(x, #x);

