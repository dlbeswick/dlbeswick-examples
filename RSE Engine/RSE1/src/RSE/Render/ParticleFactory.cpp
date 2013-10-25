// ------------------------------------------------------------------------------------------------
//
// ParticleFactory
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "ParticleFactory.h"
#include "AppBase.h"
#include "ParticleSystem.h"
#include "ParticleRenderer.h"
#include "PathResource.h"
#include "Game/Level.h"
#include <Standard/Config.h>

ParticleFactory::ParticleFactory(const std::string& configName, TextureAnimationSet& textures) : 
	m_textures(textures)
{
	m_config.add(*new ConfigService(PathResource(configName), false));
	m_config.load();
	load();
}

ParticleFactory::~ParticleFactory()
{
	freeHash(m_systems);
}

ParticleSystem* ParticleFactory::operator() (const PtrGC<Object>& parent, const std::string& name)
{
	SystemsMap::iterator i = m_systems.find(name);
	if (i == m_systems.end())
		return 0;

	ParticleSystem* newSystem = i->second->clone(parent);

	return newSystem;
}

ParticleSystem* ParticleFactory::operator() (Level& parent, const std::string& name)
{
	return operator() (parent.particleRoot(), name);
}

ParticleSystem* ParticleFactory::operator() (Level& parent, const std::string& name, const Point3& pos)
{
	ParticleSystem* s = operator() (parent.particleRoot(), name);
	if (s)
		s->setPos(pos);
	return s;
}

void ParticleFactory::reload()
{
	m_config.load();
	load();
}

void ParticleFactory::load()
{
	freeHash(m_systems);

	// load all particle systems from config file
	const Config::Categories& c = m_config.categories();
	for (Config::Categories::const_iterator i = c.begin(); i != c.end(); ++i)
	{
		m_systems[i->category->name] = new ParticleSystem(m_config, m_textures, i->category->name);
	}	
}

void ParticleFactory::update()
{
}

void ParticleFactory::systemNames(std::vector<std::string>& v)
{
	v.clear();
	for (SystemsMap::iterator i = m_systems.begin(); i != m_systems.end(); ++i)
	{
		v.push_back(i->first);
	}

	std::sort(v.begin(), v.end());
}
