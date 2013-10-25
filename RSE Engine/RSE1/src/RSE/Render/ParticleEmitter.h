// ------------------------------------------------------------------------------------------------
//
// ParticleEmitter
// Particle animations must not span textures
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "Particle.h"
#include "RSE/Game/Object.h"
#include "RSE/UI/Controls/EditableProperties.h"
#include <Standard/SortedVec.h>

namespace ParticleSort
{
	class Z 
	{
	public:
		bool operator()(const Particle* lhs, const Particle* rhs) const
		{
			return lhs->pos.z < rhs->pos.z;
		}
	};

	class Life 
	{
	public:
		bool operator()(const Particle* lhs, const Particle* rhs) const
		{
			return lhs->activeTime < rhs->activeTime;
		}
	};
};

class RSE_API ParticleEmitter : public Object
{
	USE_RTTI(ParticleEmitter, Object);
	USE_STREAMING;
	USE_EDITSTREAM;
	USE_EDITABLE;

public:
	ParticleEmitter(bool bStart = true);
	~ParticleEmitter();

	typedef std::vector<Particle> ParticleList;

	std::string	description;
	int			maxParticles;
	bool		bContinuous;
	float		lifetimeMin;
	float		lifetimeMax;
	float		spawnFrequencyMin;
	float		spawnFrequencyMax;
	Point3		sizeMin;
	Point3		sizeMax;
	Point3		sizeGrow;
	bool		bUniformScale;
	float		animScaleMin;
	float		animScaleMax;
	bool		bKillOnAnimEnd;
	Point3		velocityMin;
	Point3		velocityMax;
	Point3		accelerationMin;
	Point3		accelerationMax;
	std::string	animationName;
	std::string	impactSpawnSystemName;
	bool		bGravity;
	float		drag;
	RGBA		startColour;
	RGBA		endColour;
	bool		bSort;

	float											animLength;
	std::vector<const class TextureAnimationFrame*> animFrames;

	__forceinline void inlineUpdate(float delta);
	virtual void update(float delta);

	virtual ParticleEmitter* clone(const PtrGC<Object>& newParent);

	void setAnim(class TextureAnimationSet& set, const std::string& animationName);
	virtual void start();
	virtual void stop();
	__forceinline void deactivateParticle(Particle& p)
	{
		if (!p.bActive)
			return;

		p.bActive = false;
		m_free.push_back(&p);
	}

	virtual void finish();

	virtual void onImpact(class ParticleFactory& factory, Particle& p);

	const ParticleList& particles() { return m_particles; }
	class TextureAnimator* animator() { return m_animator; }

protected:
	friend class ParticleSystem;
	friend class Particle;

	struct FVF
	{
		float pos[3];
		D3DCOLOR c;
		float uv[2];
	};

	void reallocParticles();
	Particle& getFreeParticle();
	Particle& spawnParticle(float delta, float totalDelta);

	ParticleList m_particles;
	std::vector<Particle*> m_sortedZ;
	std::vector<Particle*> m_sortedTime;
	std::vector<Particle*> m_free;
	class TextureAnimator* m_animator;

	float m_nextSpawn;
	float m_animTime;
	bool m_bActive;
	int m_totalSpawned;
	Point3 m_lastWorldPos;
	Quat m_lastWorldRot;
};