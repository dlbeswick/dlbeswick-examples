// ------------------------------------------------------------------------------------------------
//
// ParticleSystem
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "ParticleSystem.h"
#include "ParticleEmitter.h"
#include "Particle.h"
#include "ParticleEmitterInline.h"
#include "ParticleSystemInline.h"
#include "AppBase.h"
#include "Physics/PhysicsMgr.h"
#include "Render/SDeviceD3D.h"
#include "Sound/ISoundProvider.h"
#include "Sound/ISample.h"
#include "Sound/ISampleInstance.h"
#include "Standard/Config.h"
#include "UI/Controls/UIPropertyEditor.h"


IMPLEMENT_RTTI(ParticleSystem);

#define METADATA ParticleSystem

STREAM
	STREAMVAR(m_spawnSound);
}

EDITABLE
	EDITPROP(propertySound("Spawn Sound", m_spawnSound, "Sound to play when particle system is spawned."));
}


ParticleSystem::ParticleSystem() :
	m_updated(false)
{
	bNeverDestroy = false;
}

ParticleSystem::ParticleSystem(class Config& config, class TextureAnimationSet& animationSet, const std::string& name, bool bStart) :
	m_bFinishing(false)
{
	bNeverDestroy = false;

	Streamable::read(config, name);

	setScale(AppBase().options().get<Point3>("Particles", "GlobalScale", Point3(1, 1, 1)));

	for (uint i = 0; ; ++i)
	{
		std::string varName = "Emitter";
		varName = varName + i;
		if (config.exists(name, varName))
		{
			ParticleEmitter* p = new ParticleEmitter(bStart);
			p->setParent(this);

			itextstream s(config.get<std::string>(name, varName));
			s >> *p;

			p->m_animator = 0;
			p->setAnim(animationSet, p->animationName);
		}
		else
			break;
	}

	m_children.flush();
}

ParticleSystem::~ParticleSystem()
{
}

void ParticleSystem::saveToConfig(Config& c)
{
	Streamable::write(c, name());

	uint idx = 0;
	for (ObjectList::iterator i = m_children.begin(); i != m_children.end(); ++i, ++idx)
	{
		assert((*i)->rtti().isA(ParticleEmitter::static_rtti()));
		c.set(name(), std::string("Emitter") + idx, (*i)->toString());
	}
}

ParticleSystem* ParticleSystem::clone(const PtrGC<Object>& newParent)
{
	ParticleSystem* p = new ParticleSystem(*this);
	p->m_children.clear();
	p->setParent(newParent);

	for (ObjectList::iterator i = m_children.begin(); i != m_children.end(); ++i)
	{
		assert((*i)->rtti().isA(ParticleEmitter::static_rtti()));
		Cast<ParticleEmitter>(*i)->clone(p);
	}

	p->m_updated = false; // might not need this

	return p;
}

void ParticleSystem::finish()
{
	m_bFinishing = true;

	for (ObjectList::iterator i = m_children.begin(); i != m_children.end(); ++i)
	{
		PtrGC<ParticleEmitter> e = Cast<ParticleEmitter>(*i);
		if (e)
		{
			e->finish();
		}
	}
}

// this call means that a system has been added to the scene root rather than the particles root.
// these systems do not undergo the single-pass update, draw/vertex buffer compilation, and do not execute inline.
void ParticleSystem::update(float delta)
{
	Profile("Suboptimal Particles");
	Object::update(delta);
	inlineUpdate();
}
