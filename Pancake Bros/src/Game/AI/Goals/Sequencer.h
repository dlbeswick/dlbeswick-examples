#pragma once
#include "Selector.h"

namespace Goals
{
	class Sequencer : public Selector
	{
		USE_RTTI(Sequencer, Selector);
	public:
		struct GoalData
		{
			GoalData(AIGoal* _goal, float _sleep) :
				goal(_goal),
				sleep(_sleep),
				startedTime(-FLT_MAX * 0.5f),
				idx(-1)
				{}

			bool valid(float curTime, int currentIdx)
			{
				return curTime - startedTime >= sleep && currentIdx == idx;
			}

			AIGoal* goal;
			float sleep;
			float startedTime;
			int idx;
		};

		typedef std::vector<GoalData> GoalList;

		Sequencer(GoalList& list) :
			goals(list),
			currentIdx(0)
		{
			assert(!list.empty());

		}

		GoalData* find(AIGoal& g)
		{
			for (uint i = 0; i < goals.size(); ++i)
			{
				if (goals[i].goal == &g)
					return &goals[i];
			}

			return 0;
		}

		virtual void setAI(ManAIControl* _ai)
		{
			Super::setAI(_ai);

			for (uint i = 0; i < goals.size(); ++i)
			{
				goals[i].idx = i;
				add(goals[i].goal);
			}
		}

		~Sequencer()
		{
		}

		virtual void startGoal(AIGoal& newGoal)
		{
			Super::startGoal(newGoal);

			GoalData& g = *find(newGoal);
			g.startedTime = ai->obj()->level().time();
			currentIdx = (currentIdx + 1) % goals.size();
		}

		virtual void reselectGoal()
		{
			Super::reselectGoal();
		}

		virtual bool canConsider(AIGoal& goal, float currentTime)
		{
			GoalData& d = *find(goal);
			if (!d.valid(currentTime, currentIdx))
				return false;

			return Super::canConsider(goal, currentTime);
		}

		virtual float suitability()
		{
			GoalData& g = goals[currentIdx];

			if (g.valid(ai->obj()->level().time(), currentIdx))
				return g.goal->suitability();
			else
				return 0;
		}

		GoalList goals;
		uint currentIdx;
	};
}