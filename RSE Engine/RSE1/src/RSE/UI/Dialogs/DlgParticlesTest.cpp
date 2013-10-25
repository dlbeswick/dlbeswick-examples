// ------------------------------------------------------------------------------------------------
//
// DlgParticlesTest
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "DlgParticlesTest.h"
#include "AppBase.h"
#include "Render/ParticleFactory.h"
#include "Render/ParticleSystem.h"
#include "Render/ParticleEmitter.h"

//REGISTER_RTTI(DlgParticlesTest);
IMPLEMENT_RTTI(DlgParticlesTest);

void DlgParticlesTest::onActivate()
{
	for (uint i = 0; i < 1500; ++i)
	{
		const std::string& p = "test_1_particles";
		ParticleSystem* s = AppBase().particles()(level(), p);
		s->setPos(Point3(0, 500, 0));
	}
}