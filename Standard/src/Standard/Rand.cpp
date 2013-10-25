// ------------------------------------------------------------------------------------------------
//
// Rand
//
// ------------------------------------------------------------------------------------------------
#include "Standard/pch.h"
#include "Rand.h"
#include <time.h>

STANDARD_API SRand Rand;


SRand::SRand()
{
	m_seed = (int)clock();
}

float SRand::operator() (float min, float max)
{
	m_seed = (m_seed * 1103515245 + 12345) & 0x7fffffff;
	return min + (float)m_seed * ((max - min) / 0x7fffffff);
}
