// ------------------------------------------------------------------------------------------------
//
// TransitionManager
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "TransitionManager.h"
#include "UITransition.h"

TransitionManager Transitions;

void TransitionManager::add(UITransition* transition, bool clearAll)
{
	assert(transition);

	if (clearAll)
	{
		m_transitions.flush();
		for (uint i = 0; i < m_transitions.size(); ++i)
		{
			m_transitions[i].destroy();
		}
	}
	else
	{
		// destroy transitions that are using the same source or destination as the incoming transition
		m_transitions.flush();
		for (uint i = 0; i < m_transitions.size(); ++i)
		{
			UITransition& t = *m_transitions[i];
			if (t.srcElement())
			{
				if (t.srcElement() == transition->srcElement() ||
					t.srcElement() == transition->dstElement())
				{
					t.self().destroy();
				}
			}
			if (t.dstElement())
			{
				if (t.dstElement() == transition->srcElement() ||
					t.dstElement() == transition->dstElement())
				{
					t.self().destroy();
				}
			}
		}
	}

	UIElement::dlgRoot()->addChild(transition);
	m_transitions.add(transition);
}
