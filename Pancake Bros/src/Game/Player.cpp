// ------------------------------------------------------------------------------------------------
//
// Player
//
// ------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "PancakeLevel.h"
#include "Player.h"
#include "Man.h"
#include "RSEApp.h"
#include "RSE/Physics/PhysSphere.h"
#include "Standard/Config.h"
#include "Standard/DXInput.h"

IMPLEMENT_RTTI(Player);

class ManPlayerControl : public ControlMethod<Man>
{
	USE_RTTI(ManPlayerControl, ControlMethodBase);
public:
	ManPlayerControl()
	{
		keys[0] = Input().keyFromId(App().options().get<std::string>("Player 1", "Left"));
		keys[1] = Input().keyFromId(App().options().get<std::string>("Player 1", "Right"));
		keys[2] = Input().keyFromId(App().options().get<std::string>("Player 1", "Up"));
		keys[3] = Input().keyFromId(App().options().get<std::string>("Player 1", "Down"));
		keys[4] = Input().keyFromId(App().options().get<std::string>("Player 1", "Jump"));
		keys[5] = Input().keyFromId(App().options().get<std::string>("Player 1", "Attack"));
		bWasJump = false;
		bWasFire = false;
		bJumpRequest = false; // retire once have pre-jump anim
	}

	float keyToFloat(int key)
	{
		if (key)
			return 1.0f;
		else
			return 0.0f;
	}

	virtual void update(float delta)
	{
		if (!m_obj)
			return;

		Man::ControlBlock c;
		c.bSecondaryFire = false;
		c.bJump = false;
		c.left = keyToFloat(Input().key(keys[0]));
		c.right = keyToFloat(Input().key(keys[1]));
		c.up = keyToFloat(Input().key(keys[2]));
		c.down = keyToFloat(Input().key(keys[3]));

		// check for jump/fire/headattack
		float time = (float)m_obj->level().time();
		bool bJumpKey = Input().key(keys[4]);
		bool bFireKey = Input().key(keys[5]);
		if (bFireKey && !bWasFire)
			lastFireTime = (float)time;
		if (bJumpKey && !bWasJump)
		{
			lastJumpTime = (float)time;
			bJumpRequest = true;
		}

		c.bFire = bFireKey;

		float jumpLag = std::max(0.025f, delta);

		if (bJumpRequest && time - lastJumpTime >= jumpLag)
		{
			bJumpRequest = false;
			c.bJump = true;
		}

		if (bJumpKey)
		{
			if (bFireKey)
			{
				c.bSecondaryFire = true;
				if (abs(lastFireTime - lastJumpTime) < jumpLag)
				{
					c.bFire = false;
					c.bJump = false;
				}
			}
		}

		bWasJump = bJumpKey;
		bWasFire = bFireKey;

		// apply input
		m_obj->input(c);
		if (c.left && !c.right)
		{
			m_obj->setFacing(true);
		}
		else if (c.right && !c.left)
		{
			m_obj->setFacing(false);
		}
		else if (c.right && c.left)
		{
			m_obj->setFacing(m_obj->physics()->vel().x < 0);
		}
	}

	int keys[6];
	float lastJumpTime;
	float lastFireTime;
	bool bWasJump;
	bool bWasFire;
	bool bJumpRequest;
};

IMPLEMENT_RTTI(ManPlayerControl);


Player::Player()
{
	addMethod(Man::static_rtti(), new ManPlayerControl);
}