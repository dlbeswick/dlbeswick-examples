// ------------------------------------------------------------------------------------------------
//
// Head
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "Physical.h"


class Head : public Physical
{
	USE_RTTI(Head, Physical);

public:
	Head(const Point3& worldPos, float _pancakeValue, TextureAnimationSet& animationSet, bool squashed, Weapon* squashedWeapon = 0);

	virtual void update(float delta);
	virtual void onSquash(Weapon& w);
	virtual bool canBeSquashed() const { return !_squashed; }

	float pancakeValue;

protected:
	virtual void constructed();
	virtual void createPhysics();
	void onCollide(class CollisionInfo& info);
	virtual void givePancakeToMan(Man& m);

	Point3 _worldSpawnPos;
	Weapon* _squashedWeapon;
	bool _squashed;
	void stopBleed();
};