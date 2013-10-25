#ifndef PBROS_GOALS_SELECTOR_H
#define PBROS_GOALS_SELECTOR_H

#include "Game/PancakeLevel.h"
#include "Game/AI/AIGoal.h"

class DlgDebugAI;

namespace Goals
{
	class Selector : public AIGoal
	{
		USE_RTTI(Selector, AIGoal);

		const float FREQ;

	public:
		Selector() :
			FREQ(0.25f),
			m_curGoal(0)
		{
			counters.loop(FREQ, delegate(&Selector::selectGoalIfNeeded));
		}
		
		~Selector()
		{
			freeSTL(m_goals);
		}

		virtual void reselectGoal()
		{
			m_bReselectGoal = true;
			selectGoalIfNeeded();
		}

		virtual void update(float delta)
		{
			Super::update(delta);

			for (uint i = 0; i < m_goals.size(); ++i)
			{
				AIGoal& goal = *m_goals[i];
				goal.update(delta);
			}
		}

		virtual void activeUpdate(float delta)
		{
			Super::activeUpdate(delta);

			if (m_curGoal)
			{
				m_curGoal->activeUpdate(delta);
				if (m_curGoal && m_curGoal->completed())
					reselectGoal();
			}
		}

		virtual void add(AIGoal* g)
		{
			g->setAI(ai);
			g->parent = this;
			m_goals.add(g);
		}

		virtual void stop()
		{
			if (m_curGoal)
				m_curGoal->stop();
		}

		virtual bool completed()
		{
			if (m_curGoal)
				return m_curGoal->completed();
			else
				return false;
		}

	protected:
		typedef PendingList<AIGoal*> GoalList;
		
		virtual bool canConsider(AIGoal& goal, float currentTime)
		{
			if (goal.likelihood == 0)
				return false;

			if (goal.repetitionWeight == 1)
				return true;

			return currentTime - goal.lastActiveTime > FREQ / goal.repetitionWeight;
		}

		AIGoal* bestGoal(float currentTime)
		{
			float bestScore = 0;
			AIGoal* best = 0;

			m_goals.flush();
			for (uint i = 0; i < m_goals.size(); ++i)
			{
				AIGoal& goal = *m_goals[i];

				if (!canConsider(goal, currentTime))
					continue;

				float score = goal.suitability();
				goal.lastScore = score;
				assert(score >= 0 && score <= 1);
				if (!score)
					continue;

				if (goal.likelihood != FLT_MAX)
				{
					score *= goal.likelihood;
				}
				else
				{
					score = FLT_MAX;
				}

				if (score > bestScore)
				{
					best = &goal;
					bestScore = score;
				}
			}

			return best;
		}

		bool needGoal()
		{
			return m_bReselectGoal || !m_curGoal;
		}

		void selectGoalIfNeeded()
		{
			float currentTime = (float)obj()->level().time();

			if (needGoal())
			{
				m_bReselectGoal = false;

				// note time goal ended
				if (m_curGoal)
					m_curGoal->lastActiveTime = currentTime;

				// select new goal
				AIGoal* newGoal = bestGoal(currentTime);
				if (!newGoal || m_curGoal == newGoal)
					return;

				startGoal(*newGoal);
			}
		}

		virtual void startGoal(AIGoal& newGoal)
		{
			if (m_curGoal)
				m_curGoal->stop();

			m_curGoal = &newGoal;

			if (m_curGoal)
			{
				m_curGoal->start();

				// ensure goal isn't immediately complete
				/*if (m_curGoal->completed())
					reselectGoal();*/ // fix: infinite recursion
			}
		}

		GoalList m_goals;
		AIGoal* m_curGoal;
		bool m_bReselectGoal;
		float m_nextTick;

		friend class ::DlgDebugAI;
	};
}

#endif
