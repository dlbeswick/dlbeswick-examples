// ------------------------------------------------------------------------------------------------
//
// ParticleEmitter
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "ParticleSystem.h"
#include "ParticleFactory.h"
#include "ParticleEmitter.h"
#include "ParticleEmitterInline.h"

#include "TextureAnimator.h"
#include "TextureAnimationTypes.h"
#include "TextureAnimatorInline.h"
#include "Render/SDeviceD3D.h"
#include "Render/VertexBuffer.h"
#include "Render/ParticleRenderer.h"
#include "UI/Controls/UIPropertyEditor.h"
#include <Standard/Config.h>
#include <Standard/Interpolation.h>
#include <Standard/Profiler.h>
#include <Standard/Rand.h>

#define METADATA ParticleEmitter

IMPLEMENT_RTTI(ParticleEmitter);

STREAM
	STREAMVAR(impactSpawnSystemName);
	STREAMVAR(animationName);
	STREAMVAR(maxParticles);
	STREAMVAR(lifetimeMin);
	STREAMVAR(lifetimeMax);
	STREAMVAR(animScaleMin);
	STREAMVAR(animScaleMax);
	STREAMVAR(drag);
}

EDITSTREAM
	EDITSTREAMNAME("Desc", description, "A descripton of the emitter (for reference).");
	EDITSTREAMNAME("Continuous?", bContinuous, "If true, the emitter will continuously spawn new particles. Otherwise, it will destroy itself once spawning the max number of particles.");
	EDITSTREAMNAME("Spawn Freq Min", spawnFrequencyMin, "Minimum number of particles spawned per second.");
	EDITSTREAMNAME("Spawn Freq Max", spawnFrequencyMax, "Maximum number of particles spawned per second.");
	EDITSTREAMNAME("Size Min", sizeMin, "Minimum size of each particle.");
	EDITSTREAMNAME("Size Max", sizeMax, "Maximum size of each particle.");
	EDITSTREAMNAME("Size Grow", sizeGrow, "Amount per second that each particle scales over time.");
	EDITSTREAMNAME("Uniform Scale?", bUniformScale, "Convenience. If true, the X value of scale factors are used for all axes.");
	EDITSTREAMNAME("Kill After Anim?", bKillOnAnimEnd, "If true, the particle is killed once the animation completes, otherwise the animation will loop. The particle may still be killed before the animation completes if the maximum lifetime is shorter than the animation length.");
	EDITSTREAMNAME("Velocity Min", velocityMin, "Minimum velocity (launch speed) of each particle.");
	EDITSTREAMNAME("Velocity Max", velocityMax, "Maximum velocity (launch speed) of each particle.");
	EDITSTREAMNAME("Acceleration Min", accelerationMin, "Minimum acceleration (change in velocity per second) of each particle.");
	EDITSTREAMNAME("Acceleration Max", accelerationMax, "Maximum acceleration (change in velocity per second) of each particle.");
	EDITSTREAMNAME("Use Gravity?", bGravity, "If true, the world gravity is applied to the acceleration in addition to whatever value set by the user.");
	EDITSTREAMNAME("Start Colour", startColour, "Start diffuse colour of the particle. Blends to end colour over the particle's lifetime.");
	EDITSTREAMNAME("End Colour", endColour, "End diffuse colour of the particle. Blends from start colour over the particle's lifetime.");
	EDITSTREAMNAME("Alpha Sorting?", bSort, "If true then particles within the same system are sorted by depth value. Avoids artifacts when using textures with alpha. Can be very slow for large systems, use with care.");
}

EDITABLE
	EDITPROP(property("Max/Total Particles", maxParticles, "If continuous, maximum number of particles that can be shown for this emitter before particles are reused. If non-continuous, the number of particles spawned.", 0, 1000, true));
	EDITPROP(property("Lifetime Min", lifetimeMin, "Minimum lifetime of each particle.", 0, 10));
	EDITPROP(property("Lifetime Max", lifetimeMax, "Maximum lifetime of each particle.", 0, 10));
	EDITPROP(property("Anim Scale Min", animScaleMin, "Minimum scale factor applied to animation length (2 = twice as long)", 0, 5));
	EDITPROP(property("Anim Scale Max", animScaleMax, "Maximum scale factor applied to animation length (2 = twice as long)", 0, 5));
	EDITPROP(property("Drag", drag, "A 'damping factor' that slows the particle in flight.", 0, 1, true));
}

ParticleEmitter::ParticleEmitter(bool bStart) :
	m_totalSpawned(0),
	maxParticles(100),
	bContinuous(true),
	lifetimeMin(10),
	lifetimeMax(10),
	spawnFrequencyMin(10),
	spawnFrequencyMax(10),
	sizeMin(0, 0, 0),
	sizeMax(0, 0, 0),
	sizeGrow(0, 0, 0),
	bUniformScale(true),
	animScaleMin(1),
	animScaleMax(1),
	bKillOnAnimEnd(true),
	velocityMin(Point3(-50, -50, 50)),
	velocityMax(Point3(50, 50, 100)),
	accelerationMin(Point3(0, 0, 0)),
	accelerationMax(Point3(0, 0, 0)),
	bGravity(false),
	drag(0),
	startColour(1, 1, 1, 1),
	endColour(1, 1, 1, 1),
	bSort(false),
	m_lastWorldPos(Point3::MAX)
{
	m_animator = 0;
	m_nextSpawn = 0;

	if (bStart)
		start();
	else
		m_bActive = false;
}

ParticleEmitter::~ParticleEmitter()
{
	if (m_animator)
		delete m_animator;
}

void ParticleEmitter::setAnim(class TextureAnimationSet& set, const std::string& animationName)
{
	if (!m_animator)
		m_animator = new TextureAnimator(set);
	else
		m_animator->useSet(set);

	m_animator->play(animationName, true);
	this->animationName = animationName;
	m_animTime = m_animator->duration();

	animLength = m_animator->duration();
	animFrames.reserve(m_animator->frames());
	animFrames.resize(m_animator->frames());
	for (uint i = 0; i < animFrames.size(); ++i)
	{
		animFrames[i] = (TextureAnimationFrame*)&m_animator->frame(i);
	}
}

void ParticleEmitter::start()
{
	m_nextSpawn = 0;
	m_bActive = true;
}

void ParticleEmitter::stop()
{
	m_bActive = false;
}

Particle& ParticleEmitter::getFreeParticle()
{
	Particle* val = 0;

	// reallocate if no free particles are available and we've not yet reached our maximum
	if (m_free.empty() && (int)m_particles.size() < maxParticles)
		reallocParticles();

	// try and find a free particle
	if (!m_free.empty())
	{
		val = m_free.back();
		m_free.pop_back();
	}
	else
	{
		// recycle particles once limit is reached
		if (!m_sortedTime.empty())
		{
			val = m_sortedTime.back();
			m_sortedTime.pop_back();
		}
		else
		{
			val = &m_particles.back();
		}
	}

	assert(val);
	return *val;
}

Particle& ParticleEmitter::spawnParticle(float delta, float totalDelta)
{
	Particle& p = getFreeParticle();
	Point3 curScale = worldScale();

	++m_totalSpawned;

	p.bActive = true;
	p.activeTime = 0;
	
	p.pos = Interpolation::linear(m_lastWorldPos, worldPos(), delta / totalDelta);
	Quat rot = Interpolation::linear(m_lastWorldRot, worldRot(), delta / totalDelta);
	
	p.size.x = Rand(sizeMin.x, sizeMax.x);
	if (!bUniformScale)
	{
		p.size.y = Rand(sizeMin.y, sizeMax.y);
		p.size.z = Rand(sizeMin.z, sizeMax.z);
	}
	else
	{
		p.size.y = p.size.z = p.size.x;
	}
	p.size.componentMul(curScale);
	
	p.animScale = Rand(animScaleMin, animScaleMax);
	p.vel.x = Rand(velocityMin.x, velocityMax.x);
	p.vel.y = Rand(velocityMin.y, velocityMax.y);
	p.vel.z = Rand(velocityMin.z, velocityMax.z);
	p.vel.componentMul(curScale);
	p.vel *= rot;
	p.acceleration.x = Rand(accelerationMin.x, accelerationMax.x);
	p.acceleration.y = Rand(accelerationMin.y, accelerationMax.y);
	p.acceleration.z = Rand(accelerationMin.z, accelerationMax.z);
	p.acceleration.componentMul(curScale);
	p.acceleration *= rot;
	
	p.lifeTime = Rand(lifetimeMin, lifetimeMax);
	if (bKillOnAnimEnd)
		p.lifeTime = std::min(p.lifeTime, m_animator->duration() * p.animScale);

	return p;
}

void ParticleEmitter::reallocParticles()
{
	if (m_particles.empty())
	{
		m_particles.resize(std::min(maxParticles, 50));
	}
	else
	{
		m_particles.resize(std::min(maxParticles, (int)m_particles.size() * 2));
	}

	// re-calculate free stores and sorted lists
	if (bSort)
	{
		m_sortedZ.resize(0);
		m_sortedTime.resize(0);
	}

	m_free.resize(0);

	if (bSort)
	{
		m_sortedZ.reserve(m_particles.size());
		m_sortedTime.reserve(m_particles.size());
	}

	m_free.reserve(m_particles.size());

	for (uint i = 0; i < m_particles.size(); ++i)
	{
		Particle& p = m_particles[i];
		
		if (!p.bActive)
		{
			m_free.push_back(&p);
		}
		else
		{
			/*if (bSort)
			{
				// update sorted lists
				m_sortedZ.push_back(&p);
				m_sortedTime.push_back(&p);
			}*/
		}
	}

	/*if (bSort)
	{
		std::sort(m_sortedZ.begin(), m_sortedZ.end(), ParticleSort::Z());
		std::sort(m_sortedTime.begin(), m_sortedTime.end(), ParticleSort::Life());
	}*/
}

void ParticleEmitter::onImpact(ParticleFactory& factory, Particle& p)
{
	deactivateParticle(p);
	if (!impactSpawnSystemName.empty())
	{
		factory(level(), impactSpawnSystemName, p.pos);
	}
}

void ParticleEmitter::finish()
{
	bContinuous = false;
	maxParticles = particles().size();
	m_totalSpawned = maxParticles;
}

ParticleEmitter* ParticleEmitter::clone(const PtrGC<Object>& newParent)
{
	ParticleEmitter* clone = new ParticleEmitter(*this);
	clone->m_animator = 0;
	clone->setAnim(m_animator->set(), animationName);
	clone->setParent(newParent);
	clone->bContinuous = false;
	clone->start();
	
	return clone;
}

void ParticleEmitter::update(float delta)
{
	Super::update(delta);

	inlineUpdate(delta);
}
