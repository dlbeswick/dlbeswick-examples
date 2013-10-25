// ------------------------------------------------------------------------------------------------
//
// AIGoal
// likelihood - normalised weighting given to final suitability score
// repetitionWeight - weighting that influences repetition of the goal. 
//		1 = no effect. 0.5 = only considered half as often as other goals
//
// ------------------------------------------------------------------------------------------------
#ifndef PBROS_AIGOAL_H
#define PBROS_AIGOAL_H

#include "Game/Man.h"

namespace Goals
{
	class Selector;
}

class AIGoal : public Base
{
	USE_RTTI(AIGoal, Base);
public:
	void* operator new (size_t s) { return operator new(s, 1.0f); }
	void operator delete (void* p) { return ::operator delete(p); }
	void* operator new (size_t s, float likelihood, float repetitionWeight = 1.0f)
	{
		AIGoal* a = (AIGoal*)::operator new(s);
		a->likelihood = likelihood;
		a->repetitionWeight = repetitionWeight;
		return a;
	}
	void operator delete (void* p, float, float) { return ::operator delete(p); }

	AIGoal() :
		lastActiveTime(0),
		parent(0)
	{}

	virtual void setAI(class ManAIControl* _ai)
	{
		ai = _ai;
	}

	virtual float suitability() { return -FLT_MAX; }
	virtual bool completed() { return false; }
	virtual void start() {}
	virtual void stop() {}
	virtual void activeUpdate(float delta) {}

	void move(const Point3& vec, Man::ControlBlock& c);
	void move(const Point3& vec);
	virtual void finish();

	const PtrGC<class Man>& obj() const;

	class ManAIControl* ai;
	class Goals::Selector* parent;
	float likelihood;
	float repetitionWeight;
	float lastActiveTime;
	float lastScore;

	friend class DlgDebugAI;
};

#endif
