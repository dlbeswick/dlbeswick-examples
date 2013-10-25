// ------------------------------------------------------------------------------------------------
//
// FMODSampleInstance
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "FMODProvider.h"
#include "FMODSampleInstance.h"

FMODSampleInstance::FMODSampleInstance(int channel) :
	m_channel(channel)
{
}

FMODSampleInstance::~FMODSampleInstance()
{
}

void FMODSampleInstance::play()
{
	FSOUND_SetPaused(m_channel, false);
}

bool FMODSampleInstance::playing()
{
	return FSOUND_IsPlaying(m_channel) != 0;
}

void FMODSampleInstance::pause()
{
	FSOUND_SetPaused(m_channel, true);
}

void FMODSampleInstance::stop()
{
	FSOUND_StopSound(m_channel);
}

void FMODSampleInstance::set3D(const Point3& pos, const Point3& vel)
{
	FSOUND_3D_SetAttributes(m_channel, &pos.x, &vel.x);
}
