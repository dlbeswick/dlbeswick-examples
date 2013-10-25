// ------------------------------------------------------------------------------------------------
//
// ParticleEmitterInline
//
// ------------------------------------------------------------------------------------------------
#ifndef STANDARD_PARTICLEEMITTERINLINE_H
#define STANDARD_PARTICLEEMITTERINLINE_H

#include "pch.h"

#include <Standard/Profiler.h>
#include <Standard/Rand.h>
#include <Render/VertexBuffer.h>
#include <Render/TextureAnimator.h>
#include <Render/TextureAnimationTypes.h>
#include <Render/ParticleRenderer.h>

__forceinline void Particle::update(float delta, ParticleEmitter& e)
{
	activeTime += delta;
	pos += vel * delta;
	vel += acceleration * delta;
	if (e.drag)
		vel *= pow((1.0f - e.drag), delta);

	if (e.bGravity)
		vel += e.level().gravity() * delta;

	if (!e.impactSpawnSystemName.empty())
		e.level().processParticle(e, *this);
}

__forceinline void Particle::draw(char*& buf, ParticleEmitter& e)
{
	Point3 newSize = (e.worldScale() + size + e.sizeGrow * (activeTime / lifeTime)) * 0.5f;
	float time = activeTime / lifeTime;

	RGBA colour;
	colour.r = Interpolate(time, e.startColour.r, e.endColour.r);
	colour.g = Interpolate(time, e.startColour.g, e.endColour.g);
	colour.b = Interpolate(time, e.startColour.b, e.endColour.b);
	colour.a = Interpolate(time, e.startColour.a, e.endColour.a);
	D3DCOLOR d3dc = colour;

	const TextureAnimationFrame* frame = e.animFrames[(uint)(((time / (e.animLength * animScale))) * e.animFrames.size()) & (e.animFrames.size() - 1)];

	Point2 uvMin = frame->uvMin;
	Point2 uvMax = frame->uvMax;

	newSize.x += (uvMax.x - uvMin.x) * 320.0f;
	newSize.y += (uvMax.y - uvMin.y) * 320.0f;

	Quad q(
		Point3(-newSize.x, 0, newSize.y),
		Point3(newSize.x, 0, newSize.y),
		Point3(newSize.x, 0, -newSize.y),
		Point3(-newSize.x, 0, -newSize.y)
	);

	q[0] += pos;
	q[1] += pos;
	q[2] += pos;
	q[3] += pos;

	ParticleEmitter::FVF* fvf = (ParticleEmitter::FVF*)buf;

	fvf->pos[0] = q[0].x;
	fvf->pos[1] = q[0].y;
	fvf->pos[2] = q[0].z;
	fvf->c = d3dc;
	fvf->uv[0] = uvMin.x;
	fvf->uv[1] = uvMin.y;
	++fvf;
	fvf->pos[0] = q[1].x;
	fvf->pos[1] = q[1].y;
	fvf->pos[2] = q[1].z;
	fvf->c = d3dc;
	fvf->uv[0] = uvMax.x;
	fvf->uv[1] = uvMin.y;
	++fvf;
	fvf->pos[0] = q[2].x;
	fvf->pos[1] = q[2].y;
	fvf->pos[2] = q[2].z;
	fvf->c = d3dc;
	fvf->uv[0] = uvMax.x;
	fvf->uv[1] = uvMax.y;
	++fvf;
	fvf->pos[0] = q[0].x;
	fvf->pos[1] = q[0].y;
	fvf->pos[2] = q[0].z;
	fvf->c = d3dc;
	fvf->uv[0] = uvMin.x;
	fvf->uv[1] = uvMin.y;
	++fvf;
	fvf->pos[0] = q[2].x;
	fvf->pos[1] = q[2].y;
	fvf->pos[2] = q[2].z;
	fvf->c = d3dc;
	fvf->uv[0] = uvMax.x;
	fvf->uv[1] = uvMax.y;
	++fvf;
	fvf->pos[0] = q[3].x;
	fvf->pos[1] = q[3].y;
	fvf->pos[2] = q[3].z;
	fvf->c = d3dc;
	fvf->uv[0] = uvMin.x;
	fvf->uv[1] = uvMax.y;
	++fvf;

	buf = (char*)fvf;
}

__forceinline void ParticleEmitter::inlineUpdate(float delta)
{
	//uint particleBytes = 6 * sizeof(FVF);

	ProfileValue("Particles", particles().size() - m_free.size());

	// maxParticles can be zero when 'finish' is called on system with no particles
	if (!m_animator || maxParticles == 0)
		return;

	if (m_lastWorldPos == Point3::MAX)
	{
		m_lastWorldPos = worldPos();
		m_lastWorldRot = worldRot();
	}

	if ((int)m_particles.size() > maxParticles && maxParticles > 0)
		reallocParticles();

	// spawn new particles
	if (m_bActive)
	{
		float spawnDeltaLeft = delta;
		if (spawnDeltaLeft < m_nextSpawn)
		{
			m_nextSpawn -= spawnDeltaLeft;
		}
		else
		{
			while (m_nextSpawn == 0 || (spawnDeltaLeft > 0 && spawnDeltaLeft - m_nextSpawn >= 0))
			{
				Particle& newParticle = spawnParticle(delta - spawnDeltaLeft, delta);

				spawnDeltaLeft -= m_nextSpawn;

				if (spawnDeltaLeft > delta)
					newParticle.update(spawnDeltaLeft - delta, *this);

				m_nextSpawn = 1.0f / Rand(spawnFrequencyMin, spawnFrequencyMax);

				if (!bContinuous && m_totalSpawned >= maxParticles)
				{
					m_nextSpawn = FLT_MAX;
				}
			}
		}
	}

	// update particles
/*	if (bSort)
	{
		m_sortedZ.resize(0);
		m_sortedTime.resize(0);
		m_sortedZ.reserve(m_particles.size());
		m_sortedTime.reserve(m_particles.size());
	}*/

	int numActive = 0;

	for (uint i = 0; i < m_particles.size(); ++i)
	{
		Particle& p = m_particles[i];

		if (p.bActive)
		{
			++numActive;

			p.update(delta, *this);

			if (p.activeTime >= p.lifeTime)
			{
				deactivateParticle(p);
			}
			else
			{
				// update sorted lists
/*				if (bSort)
				{
					m_sortedZ.push_back(&p);
					m_sortedTime.push_back(&p);
				}*/
				
				if (animFrames[0])
				{
					VertexBufferStream& v = level().particleRoot()->streamFor(animFrames[0]);

					if (v.ptrAccess() + sizeof(ParticleEmitter::FVF) * 6 > v.ptrEnd())
					{
						v.loadChunk();
					}

					p.draw(v.ptrAccess(), *this);
				}
			}
		}
	}

/*	if (bSort)
	{
		std::sort(m_sortedZ.begin(), m_sortedZ.end(), ParticleSort::Z());
		std::sort(m_sortedTime.begin(), m_sortedTime.end(), ParticleSort::Life());
	}*/

	if (!bContinuous && m_totalSpawned >= maxParticles && numActive == 0)
	{
		if (parent().downcast<ParticleSystem>()->bNeverDestroy)
		{
			m_nextSpawn = 1;
			m_totalSpawned = 0;
		}
		else
		{
			PtrGC<Object>(this).destroy();
		}
	}

	m_lastWorldPos = worldPos();
	m_lastWorldRot = worldRot();
}

#endif
