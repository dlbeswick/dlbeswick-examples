// ------------------------------------------------------------------------------------------------
//
// Particle
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/Game/Level.h"


class RSE_API Particle
{
public:
	Particle() :
		bActive(false)
	{
	}

	bool	bActive;
	
	Point3	pos;
	Point3	vel;
	Point3	size;
	Point3	acceleration;
	float	animScale;
	float	activeTime;
	float	lifeTime;

	__forceinline void update(float delta, class ParticleEmitter& e);
	__forceinline void draw(char*& buf, class ParticleEmitter& e);
};