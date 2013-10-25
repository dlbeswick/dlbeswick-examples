// ------------------------------------------------------------------------------------------------
//
// ManAIControl
//
// ------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "ManAIControl.h"
#include "RSEApp.h"
#include "AI/Goals/AvoidDisabled.h"
#include "AI/Goals/Home.h"
#include "AI/Goals/Taunt.h"
#include "AI/Goals/PanicGore.h"
#include "AI/Goals/WeaponAttack.h"
#include "AI/Goals/BaddyHome.h"
#include "AI/Goals/MoveFromOOB.h"
#include "AI/Goals/JumpAttack.h"
#include "AI/Goals/Sequencer.h"
#include "AI/Goals/Flee.h"
#include "AI/Goals/Selector.h"
#include "Game/Man.h"
#include "Game/ManAIControl.h"
#include "Game/PancakeLevel.h"
#include "Game/Weapon.h"
#include "RSE/Game/TextureAnimatorLayered.h"
#include <Standard/Config.h>
#include <Standard/Profiler.h>
#include <Standard/Range.h>

IMPLEMENT_RTTI(ManAIControl);

ManAIControl::ManAIControl()
{
	setConfig(App().configAI());
	m_rootGoal = new Goals::Selector;
	m_rootGoal->setAI(this);
}

ManAIControl::~ManAIControl()
{
	delete m_rootGoal;
}

void ManAIControl::update(float delta)
{
	Super::update(delta);

	if (obj()->dead())
	{
		if (m_rootGoal)
			m_rootGoal->stop();
		obj()->controller().destroy();
		return;
	}

	Profile("AI");

	if (m_rootGoal)
	{
		m_rootGoal->update(delta);
		m_rootGoal->activeUpdate(delta);
	}
}

void ManAIControl::add(AIGoal* g)
{
	m_rootGoal->add(g);
}

void ManAIControl::reselectGoal()
{
	m_rootGoal->reselectGoal();
}

// AIBaddy

REGISTER_RTTI(AIBaddy);

AIBaddy::AIBaddy()
{
}

void AIBaddy::activated(PancakeLevel& level)
{
	PancakeLevel& l = level;

	add(new(FLT_MAX) Goals::MoveFromOOB);
	add(new(get("Cocky", 0.5f), 0.25f) Goals::Taunt(get<std::string>("TauntAnim"), l.bound().x * 0.35f));
	add(new(get("Scared", 0.0f)) Goals::PanicGore);
	add(new(get("AvoidDisabled", 0.0f)) Goals::AvoidDisabled(get("AvoidDisabledRange", 100.0f)));
	addJumpGoal();

	if (App().options().get<bool>("Pancake Bros", "AIAttacks", true))
	{
		if (get<Range<float> >("RangedAttackFreq") != Range<float>(0,0))
		{
			add(new Goals::WeaponAttackRanged(get<Range<float> >("RangedAttackFreq"), get("LeadSkill", 1.0f)));
		}
		else
		{
			if (get<Range<float> >("MeleeAttackFreq") != Range<float>(0,0))
				add(new Goals::WeaponAttack(get<Range<float> >("MeleeAttackFreq")));
		}

		if (get("RangedAttackAfterTargetChargesAmt", -1) != -1)
		{
			add(new Goals::WeaponAttackRangedOnCharge(
					get<Range<float> >("RangedAttackOnChargingTargetFreq"),
					get("RangedAttackAfterTargetChargesAmt", -1),
					get("LeadSkill", 1.0f)
				)
			);
		}
	}
}

void AIBaddy::addJumpGoal()
{
	add(new(get("Jump", 0.0f)) Goals::JumpAttack);
}

// Red Guard
class AIRedGuard : public AIBaddy
{
	USE_RTTI(AIRedGuard, AIBaddy);
public:
	virtual void activated(PancakeLevel& level)
	{
		Super::activated(level);

		if (App().options().get<bool>("Pancake Bros", "AIAttacks", true))
			add(new Goals::WeaponAttack(get("AttackFreq", Range<float>(1.0f, 3.0f))()));

		add(new Goals::BaddyHome);
	}
};

REGISTER_RTTI(AIRedGuard);

// Cat
class AICat : public AIBaddy
{
	USE_RTTI(AICat, AIBaddy);
public:
	virtual void activated(PancakeLevel& level)
	{
		Super::activated(level);

		add(new Goals::BaddyHome);
	}

	virtual void addJumpGoal()
	{
		Goals::Sequencer::GoalList goals;
		goals.push_back(Goals::Sequencer::GoalData(new(0.1f) Goals::Home, 0.0f));
		goals.push_back(Goals::Sequencer::GoalData(new(1.0f) Goals::JumpAttack(0.0f), 2.5f));
		goals.push_back(Goals::Sequencer::GoalData(new(0.1f) Goals::Flee(Range<float>(0.5f, 1.0f)), 0.0f));
		add(new(get("Jump", 0.0f)) Goals::Sequencer(goals));
	}
};

REGISTER_RTTI(AICat);
