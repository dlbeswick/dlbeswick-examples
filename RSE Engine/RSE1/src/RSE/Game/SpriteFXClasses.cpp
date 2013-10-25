// ------------------------------------------------------------------------------------------------
//
// SpriteFXClasses
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "SpriteFXClasses.h"
#include "Sprite.h"
#include "AppBase.h"
#include "PathResource.h"
#include "Game/Level.h"
#include "UI/Controls/UIPropertyEditor.h"
#include "Render/ParticleFactory.h"
#include "Render/ParticleRenderer.h"
#include "Render/ParticleSystem.h"
#include "Render/TextureAnimator.h"
#include "Render/TextureAnimationTypes.h"
#include "Standard/Exception/Filesystem.h"
#include "Sound/ISample.h"
#include "Sound/ISoundProvider.h"

REGISTER_RTTI_NAME(SpriteFXStop, "Stop Effect");
REGISTER_RTTI_NAME(SpriteFXMove, "Move Effect");
REGISTER_RTTI_NAME(SpriteFXParticles, "Start Particles");
REGISTER_RTTI_NAME(SpriteFXSound, "Play Sample");

// Stop FX
void SpriteFXStop::start(SpriteFXPlayer& mgr, SpriteFX* fx)
{
	if (fx != this)
		fx->stop(mgr);
}

// Move FX
void SpriteFXMove::start(SpriteFXPlayer& mgr, SpriteFX* fx)
{
	fx->setPos(m_pos);
	fx->setRot(m_rot);
};

void SpriteFXMove::editableProperties(PropertyEditorList& l)
{
	Super::editableProperties(l);

	// fix -- old style code
	l.push_back(property("Pos", m_pos, "Effect Position."));
	l.push_back(property("Rot", m_rot ,"Effect Rotation."));
}

// Particles
SpriteFXParticles::SpriteFXParticles()
{
}

void SpriteFXParticles::start(SpriteFXPlayer& mgr)
{
	assert(mgr.sprite());

	if (!m_system || m_system->finishing())
	{
		PtrGC<Object> parent;
		
		if (m_attachToParent)
			parent = mgr.sprite();
		else
			parent = mgr.sprite()->level().particleRoot();

		m_system = AppBase().particles()(parent, systemName);
		if (m_system)
		{
			Point3 pos = Point3(m_pos.x, 0, m_pos.y);
			
			if (!m_attachToParent)
				pos *= mgr.sprite()->worldXForm();

			m_system->setPos(pos);
			m_system->setRot(m_rot);
		}
	}
}

void SpriteFXParticles::stop(SpriteFXPlayer& mgr)
{
	if (m_system && !m_attachToParent)
		m_system->finish();
}

void SpriteFXParticles::editableProperties(PropertyEditorList& l)
{
	Super::editableProperties(l);

	std::vector<std::string> systems;
	AppBase().particles().systemNames(systems);

	// fix: old style code
	l.push_back(property("System", systemName, "Name of particle system to spawn.", systems));
	l.push_back(property("Pos", m_pos, "Initial position of particle system."));
	l.push_back(property("Rot", m_rot, "Initial rotation of particle system."));
}


// Sound
SpriteFXSound::SpriteFXSound() :
	m_sample(0),
	m_failed(false)
{
}

void SpriteFXSound::start(SpriteFXPlayer& mgr)
{
	if (m_failed || m_name.empty())
		return;

	if (!m_sample)
	{
		try
		{
			m_sample = &AppBase().sound().sample(PathResource(m_name));
		}
		catch (ExceptionFilesystem& e)
		{
			m_failed = true;

			try
			{
				derr << "SpriteFXSound: " << mgr.sprite()->animator().sequence().name << ": couldn't load sample '" << m_name << "': " << e.what() << derr.endl;
			}
			catch (...)
			{
				derr << "SpriteFXSound: <unknown sequence>: couldn't load sample '" << m_name << "'" << derr.endl;
			}

			return;
		}
	}

	m_instance = m_sample->play();
}

void SpriteFXSound::stop(SpriteFXPlayer& mgr)
{
	if (m_instance)
	{
		m_instance->stop();
		m_instance = 0;
	}
}

void SpriteFXSound::onEditableCommit()
{
	// ensure sound will play again
/*	m_sample = 0;
	if (m_instance)
	{
		m_instance->stop();
		m_instance = 0;
	}*/ // dbeswick: done with play button
	m_failed = false; // give us another try at loading
}

void SpriteFXSound::streamVars(StreamVars& v)
{
	Super::streamVars(v);
	STREAMVAR(m_name);
}

void SpriteFXSound::editableProperties(PropertyEditorList& l)
{
	Super::editableProperties(l);

	l.push_back(propertySound("Name", m_name, "Filename of the sound, relative to sounds dir."));
}
