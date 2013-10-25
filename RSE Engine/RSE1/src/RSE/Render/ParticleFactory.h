// ------------------------------------------------------------------------------------------------
//
// ParticleFactory
//
// ------------------------------------------------------------------------------------------------
#ifndef RSE_PARTICLEFACTOR_H
#define RSE_PARTICLEFACTOR_H

#include "RSE/RSE.h"
#include "Standard/Config.h"
#include "Standard/Math.h"
#include "Standard/PendingList.h"
#include "Standard/PtrGC.h"

class Object;
class Level;
class ParticleSystem;
class TextureAnimationSet;

class RSE_API ParticleFactory
{
public:
	ParticleFactory(const std::string& configName, TextureAnimationSet& textures);
	~ParticleFactory();

	void update();
	void reload();

	class ParticleSystem* operator() (const PtrGC<Object>& parent, const std::string& name);
	class ParticleSystem* operator() (Level& parent, const std::string& name);
	class ParticleSystem* operator() (Level& parent, const std::string& name, const Point3& pos);

	void systemNames(std::vector<std::string>& v);

private:
	void load();

	Config							m_config;
	TextureAnimationSet&			m_textures;
	
	typedef stdext::hash_map<std::string, ParticleSystem*> SystemsMap;
	SystemsMap						m_systems;
};

#endif