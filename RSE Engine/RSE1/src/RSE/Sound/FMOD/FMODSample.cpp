// ------------------------------------------------------------------------------------------------
//
// FMODSample
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "FMODSample.h"
#include "FMODProvider.h"
#include "FMODSampleInstance.h"

FMODSample::FMODSample(ibinstream& s)
{
	char* buf;
	size_t length;
	s.toBuf(buf, length);

	m_sample = FSOUND_Sample_Load(FSOUND_UNMANAGED | FSOUND_HW3D, buf, FSOUND_LOADMEMORY, 0, length);
	if (!m_sample)
		throwf("FMODSample: FSOUND_Sample_Load failed: " + FMODProvider::error());
}

FMODSample::~FMODSample()
{
	FSOUND_Sample_Free(m_sample);
}

SmartPtr<ISampleInstance> FMODSample::play()
{
	return new FMODSampleInstance(FSOUND_PlaySound(FSOUND_FREE, m_sample));
}
