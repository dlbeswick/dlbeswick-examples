// ------------------------------------------------------------------------------------------------
//
// TransitionManager
//
// ------------------------------------------------------------------------------------------------
#ifndef RSE_TRANSITIONMANAGER_H
#define RSE_TRANSITIONMANAGER_H

#include "Standard/PendingList.h"
#include "Standard/PtrGC.h"
class UITransition;

class RSE_API TransitionManager
{
public:
	void add(UITransition* transition, bool clearAll = true);

protected:
	typedef PendingListNullRemover<PtrGC<UITransition> > Transitions;
	Transitions m_transitions;
};

extern TransitionManager Transitions;

#endif
