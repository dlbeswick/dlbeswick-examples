// ------------------------------------------------------------------------------------------------
//
// AIGoal
//
// ------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "AIGoal.h"
#include "AI.h"
#include "Game/Man.h"
#include "Game/ManAIControl.h"
#include "Goals/Selector.h"

IMPLEMENT_RTTI(AIGoal);

void AIGoal::move(const Point3& vec, Man::ControlBlock& c)
{
	zero(c);
	c.right = std::max(0.0f, vec * Point3(1, 0, 0));
	c.left = std::max(0.0f, vec * Point3(-1, 0, 0));
	c.up = std::max(0.0f, vec * Point3(0, 1, 0));
	c.down = std::max(0.0f, vec * Point3(0, -1, 0));
	ai->obj()->setFacing(c.left != 0);
}

void AIGoal::move(const Point3& vec)
{
	Man::ControlBlock c;
	move(vec, c);
	ai->obj()->input(c);
}

void AIGoal::finish()
{
	assert(parent);
	if (parent)
		parent->reselectGoal();
}

const PtrGC<class Man>& AIGoal::obj() const
{
	return ai->obj();
}
