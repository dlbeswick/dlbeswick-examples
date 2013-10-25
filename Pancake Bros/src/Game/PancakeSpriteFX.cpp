// ------------------------------------------------------------------------------------------------
//
// PancakeSpriteFX
//
// ------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "RSEApp.h"
#include "Man.h"
#include "Game/PancakeLevel.h"
#include "Game/PancakeSprite.h"
#include "Game/Wave.h"
#include "RSE/Game/SpriteFX.h"
#include "RSE/UI/Controls/UIPropertyEditor.h"
#include "Standard/Config.h"
#include "Standard/Log.h"

std::vector<std::string> allCharacters()
{
	std::vector<std::string> result;

	for (Config::Categories::const_iterator i = App().configGame().categories().begin(); i != App().configGame().categories().end(); ++i)
	{
		if (i->category->name.find("character ") != std::string::npos)
			result.push_back(i->category->name.substr(std::string("character ").size()));
	}

	return result;
}

////

class SpriteFXShootProjectile : public SpriteFXPos
{
	USE_RTTI(SpriteFXShootProjectile, SpriteFXPos);
public:
	virtual void start(SpriteFXPlayer& mgr)
	{
		Man* m = Cast<Man>(mgr.sprite()->parent().ptr());
		if (m)
		{
			m->onProjectileLaunchRequest(m_pos);
		}
	}

	virtual const char* help() {
		return "Acts as a marker in time and position for the launch of a projectile (rotation ignored).";
	};
};

REGISTER_RTTI_NAME(SpriteFXShootProjectile, "Shoot Projectile");

////

class SpriteFXMeleeHit : public SpriteFX
{
	USE_RTTI(SpriteFXMeleeHit, SpriteFX);
public:
	virtual void start(SpriteFXPlayer& mgr)
	{
		Man* m = Cast<Man>(mgr.sprite()->parent().ptr());
		if (m)
		{
			m->onMeleeHitRequest();
		}
	}

	virtual const char* help() {
		return "Acts as a marker in time and position for the launch of a projectile (rotation ignored).";
	};
};

REGISTER_RTTI_NAME(SpriteFXMeleeHit, "Melee Damage");

////

class SpriteFXSpawnSquashedHead : public SpriteFXPos
{
	USE_RTTI(SpriteFXSpawnSquashedHead, SpriteFXPos);
public:
	virtual void start(SpriteFXPlayer& mgr)
	{
		Man* m = Cast<Man>(mgr.sprite()->parent().ptr());
		if (m)
		{
			m->onHeadSquashRequest(m_pos);
		}
	}

	virtual const char* help() {
		return "Spawns a squashed head for the character.";
	};
};

REGISTER_RTTI_NAME(SpriteFXSpawnSquashedHead, "Spawn Squashed Head");

////

// Spawn Character
class SpriteFXSpawnCharacter : public SpriteFXPos
{
	USE_RTTI(SpriteFXSpawnCharacter, SpriteFXPos);
	USE_STREAMING;
	USE_EDITSTREAM;
	USE_EDITABLE;

public:
	SpriteFXSpawnCharacter() :
	  zOrderBehind(true)
	{}	  

	std::string characterType;
	bool zOrderBehind;

	virtual const char* name() { return "Spawn Character"; }
	virtual const char* help() {
		return "Spawn a character";
	};

	virtual void start(SpriteFXPlayer& mgr)
	{
		if (Cast<PancakeLevel>(&mgr.sprite()->level()))
		{
			Point3 pos = mgr.sprite()->worldPos() + Point3(m_pos.x, 0, m_pos.y);
			if (zOrderBehind)
				pos.y += 1.0f;
			else
				pos.y -= 1.0f;

			((PancakeLevel&)mgr.sprite()->level()).spawnCharacter(characterType, -1, pos);
		}
	}

	virtual void stop(SpriteFXPlayer& mgr) {}
};

REGISTER_RTTI_NAME(SpriteFXSpawnCharacter, "Spawn Character");

#define METADATA SpriteFXSpawnCharacter
STREAM
	STREAMVAR(characterType);
}

EDITSTREAM
	EDITSTREAMVAR(zOrderBehind, "If true, then the new character is placed further back than the sprite that spawns him.");
}

EDITABLE
	EDITPROP(property("Type", characterType, "Type of character to spawn (taken from game.ini)", allCharacters()));
}

////

// Spawn a wave of baddies.
class SpriteFXSpawnBaddyWave : public SpriteFX
{
	USE_RTTI(SpriteFXSpawnBaddyWave, SpriteFX);
	USE_STREAMING;
	USE_EDITSTREAM;
	USE_EDITABLE;

public:
	SpriteFXSpawnBaddyWave() :
	  amount(1),
	  frequency(0)
	{}	  

	std::string characterType;
	int amount;
	float frequency;
	bool pauseLevelWave;

	virtual const char* name() { return "Spawn Baddy Wave"; }
	virtual const char* help() {
		return "Spawn a wave of baddies. If an action of this kind is already executing, then it's stopped.";
	};

	virtual void start(SpriteFXPlayer& mgr)
	{
		if (!Cast<PancakeLevel>(&mgr.sprite()->level()))
			return;

		PancakeLevel& level = (PancakeLevel&)mgr.sprite()->level();

		Wave::WaveOps waveOps;

		waveOps.push_back(new WaveOpFreq(frequency));

		for (int i = 0; i < amount; ++i)
			waveOps.push_back(new WaveOpSpawn(characterType));

		level.setTemporaryWave(new Wave(waveOps), pauseLevelWave);
	}

	virtual void stop(SpriteFXPlayer& mgr)
	{
		if (!Cast<PancakeLevel>(&mgr.sprite()->level()))
			return;

		PancakeLevel& level = (PancakeLevel&)mgr.sprite()->level();

		level.setTemporaryWave(0, false);
	}
};

REGISTER_RTTI_NAME(SpriteFXSpawnBaddyWave, "Spawn Baddy Wave");

#undef METADATA
#define METADATA SpriteFXSpawnBaddyWave
STREAM
	STREAMVAR(characterType);
}

EDITSTREAM
	EDITSTREAMVAR(amount, "The number of baddies to spawn.");
	EDITSTREAMVAR(frequency, "How often in seconds that baddies are spawned.");
	EDITSTREAMVAR(pauseLevelWave, "If true, then the current level's wave is paused until this temporary wave finishes.");
}

EDITABLE
	std::vector<std::string> allCharacters;
	for (Config::Categories::const_iterator i = App().configGame().categories().begin(); i != App().configGame().categories().end(); ++i)
	{
		if (i->category->name.find("character ") != std::string::npos)
			allCharacters.push_back(i->category->name.substr(std::string("character ").size()));
	}

	EDITPROP(property("Type", characterType, "Type of character to spawn (taken from game.ini)", allCharacters));
}
