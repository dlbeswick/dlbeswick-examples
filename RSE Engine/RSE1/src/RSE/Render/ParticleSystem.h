// ------------------------------------------------------------------------------------------------
//
// ParticleSystem
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/Game/Object.h"

class ParticleEmitter;


class RSE_API ParticleSystem : public Object
{
	USE_RTTI(ParticleSystem, Object);
	USE_EDITABLE;
	USE_STREAMING;

public:
	ParticleSystem();
	ParticleSystem(class Config& config, class TextureAnimationSet& animationSet, const std::string& name, bool bStart = true);
	virtual ~ParticleSystem();

	virtual void saveToConfig(class Config& c);
	virtual ParticleSystem* clone(const PtrGC<Object>& newParent);

	virtual void update(float delta);

	__forceinline void inlineUpdate();

	bool finishing() const { return m_bFinishing; }

	void finish();

	bool bNeverDestroy;

protected:
	bool m_bFinishing;
	bool m_updated;
	std::string m_spawnSound;
};