// ------------------------------------------------------------------------------------------------
//
// ParticleSystemInline
//
// ------------------------------------------------------------------------------------------------
#pragma once

#include "ParticleSystem.h"
#include "ParticleEmitter.h"
#include "ParticleEmitterInline.h"
#include "PathResource.h"
#include "AppBase.h"
#include "Sound/ISample.h"
#include "Sound/ISampleInstance.h"
#include "Sound/ISoundProvider.h"

__forceinline void ParticleSystem::inlineUpdate()
{
	m_children.flush();

	if (!m_updated && !m_spawnSound.empty())
		AppBase().sound().sample(PathResource(m_spawnSound)).play();

	if (children().size() == 0 && !bNeverDestroy)
	{
		PtrGC<Object>(this).destroy();
	}

	m_updated = true;
}