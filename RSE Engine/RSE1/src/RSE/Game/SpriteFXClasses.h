// ------------------------------------------------------------------------------------------------
//
// SpriteFXClasses
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "SpriteFX.h"
#include "Sound/ISampleInstance.h"

// Stop FX
class RSE_API SpriteFXStop : public SpriteFXIterating
{
	USE_RTTI(SpriteFXStop, SpriteFXIterating);
public:
	virtual const char* name() { return "Stop Effect"; }
	virtual const char* help() {
		return "Stop effects of the given id";
	};
	
	virtual void start(SpriteFXPlayer& mgr, SpriteFX* fx);
};

// Move FX
class RSE_API SpriteFXMove : public SpriteFXIterating
{
	USE_RTTI(SpriteFXMove, SpriteFXIterating);
public:
	SpriteFXMove() :
	  m_pos(0,0),
	  m_rot(1,0,0,0)
	{}

	virtual void editableProperties(PropertyEditorList& l);

	virtual const char* name() { return "Move Effect"; }
	virtual const char* help() {
		return "Affect position and rotation of effects of the given id";
	};

	virtual void start(SpriteFXPlayer& mgr, SpriteFX* fx);

	virtual void setPos(const Point2& pos) { m_pos = pos; }
	virtual const Point2& pos() const { return m_pos; }
	virtual void setRot(const Quat& rot) { m_rot = rot; }
	virtual const Quat& rot() const { return m_rot; }

protected:
	Point2 m_pos;
	Quat m_rot;

	virtual void streamVars(StreamVars& v)
	{
		Super::streamVars(v);
		STREAMVAR(m_pos);
		STREAMVAR(m_rot);
	}
};

// Particles
class RSE_API SpriteFXParticles : public SpriteFXPos
{
	USE_RTTI(SpriteFXParticles, SpriteFXPos);
public:
	SpriteFXParticles();

	virtual void editableProperties(PropertyEditorList& l);

	std::string systemName;

	virtual const char* name() { return "Start Particles"; }
	virtual const char* help() {
		return "Spawn a new particle system";
	};

	virtual void start(SpriteFXPlayer& mgr);
	virtual void stop(SpriteFXPlayer& mgr);

protected:
	class PtrGC<class ParticleSystem> m_system;

	virtual void streamVars(StreamVars& v)
	{
		Super::streamVars(v);
		STREAMVAR(systemName);
	}
};

// Sound
class RSE_API SpriteFXSound : public SpriteFX
{
	USE_RTTI(SpriteFXSound, SpriteFX);
public:
	SpriteFXSound();

	std::string systemName;

	virtual void editableProperties(PropertyEditorList& l);

	virtual const char* name() { return "Play Sound"; }
	virtual const char* help() {
		return "Play a sound sample.";
	};

	virtual void start(SpriteFXPlayer& mgr);
	virtual void stop(SpriteFXPlayer& mgr);

	virtual void onEditableCommit();

protected:
	std::string m_name;
	class ISample* m_sample;
	SmartPtr<ISampleInstance> m_instance;
	bool m_failed;

	virtual void streamVars(StreamVars& v);
};