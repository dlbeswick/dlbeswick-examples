// ------------------------------------------------------------------------------------------------
//
// Wave
//
// ------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "Wave.h"
#include "Man.h"
#include "PancakeLevel.h"
#include "RSEApp.h"
#include "RSE/Physics/PhysSphere.h"
#include "Standard/Registrar.h"
#include "Standard/Config.h"
#include "Standard/Rand.h"

IMPLEMENT_RTTI(WaveOp);

////

IMPLEMENT_RTTI(WaveOpSpawn);

WaveOpSpawn::WaveOpSpawn(const std::string& characterName) :
	type(characterName)
{
}

void WaveOpSpawn::init(const std::string& data)
{
	type = data;
}

bool WaveOpSpawn::execute(Wave& w, PancakeLevel& l)
{
	l.spawnCharacter(type, w.idx());

	return true;
}

////

REGISTER_RTTI_NAME(WaveOpFreq, "freq");

WaveOpFreq::WaveOpFreq(float _freq) :
	freq(_freq)
{
}

void WaveOpFreq::init(const std::string& data)
{
	freq = (float)atof(data.c_str());
}

bool WaveOpFreq::execute(Wave& w, PancakeLevel& l)
{
	w.setFreq(freq);
	return false;
}

////

void Wave::initFromData(const WaveOps& waveOps)
{
	setFreq(1);

	m_pauseTime = 0;
	m_nextSpawnTimeRemaining = 0;
	m_opsIdx = 0;

	m_ops = waveOps;
}

Wave::Wave(const std::string& data)
{
	WaveOps waveOps;
	
	std::vector<std::string> r;
	
	split(data, r, ",");

	for (uint i = 0; i < r.size(); i++)
	{
		std::vector<std::string> r2;

		split(r[i], r2, ":");
		if (r2.size() > 1)
		{
			// command
			std::string cmd = r2[0];
			std::string data = r2[1];

			WaveOp* w = WaveOp::newObjectByDesc(cmd.c_str());
			if (w)
			{
				w->init(data);
				waveOps.push_back(w);
			}
			else
			{
				derr << "Wave::Wave - command ignored (" + cmd + ")" << dlog.endl;
			}
		}
		else if (r2.size() == 1)
		{
			WaveOp* w = WaveOp::newObjectByDesc(r2[0].c_str());
			if (!w)
			{
				w = new WaveOpSpawn;
				w->init(r2[0]);
				waveOps.push_back(w);
			}
		}
	}

	initFromData(waveOps);
}

Wave::Wave(const WaveOps& waveOps)
{
	initFromData(waveOps);
}

Wave::~Wave()
{
	freeSTL(m_ops);
}

void Wave::update(float delta, PancakeLevel& l)
{
	if (finished())
		return;

	if (m_pauseTime > 0)
	{
		m_pauseTime -= delta;
		return;
	}

	if (m_nextSpawnTimeRemaining <= 0)
	{
		bool advance = false;

		do
		{
			advance = !m_ops[m_opsIdx]->execute(*this, l);
			++m_opsIdx;
		}
		while (advance && !finished());

		m_nextSpawnTimeRemaining = 1.0f / m_freq;
	}
	else
	{
		m_nextSpawnTimeRemaining -= delta;
	}
}

bool Wave::finished() const
{ 
	return m_opsIdx >= m_ops.size() || m_opsIdx > (uint)App().options().get("Pancake Bros", "WaveIdxMax", INT_MAX); 
}

void Wave::setFreq(float f)
{ 
	if (f == 0)
	{
		m_nextSpawnTimeRemaining = FLT_MAX;
	}
	else
	{
		m_freq = f; 
		m_nextSpawnTimeRemaining = 1.0f / m_freq; 
	}
}

void Wave::pause(float time) 
{ 
	m_pauseTime = time; 
}

uint Wave::idx() const 
{ 
	return m_opsIdx; 
}
