// ------------------------------------------------------------------------------------------------
//
// Wave
//
// ------------------------------------------------------------------------------------------------
#pragma once

#include <Standard/Registrar.h>

class PancakeLevel;
class Wave;

////

class WaveOp : public Registrar<WaveOp>
{
	USE_RTTI(WaveOp, WaveOp);
public:
	virtual void init(const std::string& data) = 0;
	virtual bool execute(Wave& w, PancakeLevel& l) = 0; // return true to block
};

////

class WaveOpSpawn : public WaveOp
{
	USE_RTTI(WaveOpSpawn, WaveOp);
public:
	WaveOpSpawn(const std::string& characterName = "");

	virtual void init(const std::string& data);

	virtual bool execute(Wave& w, PancakeLevel& l);

	std::string type;
};

////

class WaveOpFreq : public WaveOp
{
	USE_RTTI(WaveOpFreq, WaveOp);
public:
	WaveOpFreq(float _freq = 1);

	virtual void init(const std::string& data);

	virtual bool execute(Wave& w, PancakeLevel& l);

	float freq;
};

////

class Wave
{
public:
	typedef std::vector<WaveOp*> WaveOps;

	// Construct a Wave from a list of WaveOp objects.
	Wave(const WaveOps& waveOps);

	// Construct a Wave from a config file definition.
	Wave(const std::string& data);

	~Wave();

	void update(float delta, class PancakeLevel& l);

	void setFreq(float f);
	void pause(float time);
	void play() { pause(0); }

	bool finished() const;
	uint idx() const;

protected:
	float m_freq;
	float m_pauseTime;
	float m_nextSpawnTimeRemaining;
	uint m_opsIdx;

	WaveOps m_ops;

private:
	void initFromData(const std::vector<WaveOp*>& waveOps);
};