// ------------------------------------------------------------------------------------------------
//
// Rand
//
// ------------------------------------------------------------------------------------------------
#pragma once

#include "Standard/api.h"
#include "Singleton.h"


class STANDARD_API SRand : public Singleton<SRand>
{
public:
	SRand();

	int seed() { return m_seed; }
	void setSeed(int seed) { m_seed = seed; }

	float operator() (float min = 0.0f, float max = 1.0f);

private:
	int m_seed;
};

extern STANDARD_API SRand Rand;
