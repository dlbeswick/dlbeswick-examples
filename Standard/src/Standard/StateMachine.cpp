// ------------------------------------------------------------------------------------------------
//
// StateMachine
//
// ------------------------------------------------------------------------------------------------
#include "Standard/pch.h"
#include "StateMachine.h"
#include "STLHelp.h"

StateMachineBase::StateMachineBase() : 
	m_curState(0),
	m_nullState(0)
{
}

StateMachineBase::~StateMachineBase()
{
	freeSTL(m_states);
}

void StateMachineBase::go(StateBase* state, bool force)
{
	assert(!m_states.empty());
	assert(m_nullState);

	if (!state)
		state = m_nullState;

	if (m_curState != state || force)
	{
		if (m_curState)
			m_curState->end();

		m_curState = state;

		if (m_curState)
			m_curState->start();
	}
}

void StateMachineBase::update(float delta)
{
	if (m_curState)
		m_curState->update(delta);
}
