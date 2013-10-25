// ------------------------------------------------------------------------------------------------
//
// ManAIControl
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "Controller.h"
#include <Game/Man.h>
#include <Standard/PendingList.h>

class AIGoal;
namespace Goals
{
	class Selector;
}

class ManAIControl : public ControlMethod<Man>
{
	USE_RTTI(ManAIControl, ControlMethodBase);
public:
	ManAIControl();
	~ManAIControl();

	virtual void update(float delta);
	virtual void add(AIGoal* g);

	virtual void reselectGoal();

	Goals::Selector& rootGoal() const { return *m_rootGoal; }

private:
	Goals::Selector* m_rootGoal;
};

// AIBaddy
class AIBaddy : public ManAIControl
{
	USE_RTTI(AIBaddy, ManAIControl);
public:
	AIBaddy();

protected:
	virtual void activated(PancakeLevel& level);
	virtual void addJumpGoal();
};

