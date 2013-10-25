// ------------------------------------------------------------------------------------------------
//
// FMODSample
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "Sound/ISample.h"

class RSE_API FMODSample : public ISample
{
public:
	FMODSample(ibinstream& s);
	~FMODSample();

	virtual SmartPtr<ISampleInstance> play();

protected:
	FSOUND_SAMPLE* m_sample;
};