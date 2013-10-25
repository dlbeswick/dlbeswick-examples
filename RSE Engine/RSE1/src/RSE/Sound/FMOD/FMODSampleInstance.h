// ------------------------------------------------------------------------------------------------
//
// FMODSampleInstance
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "Sound/ISampleInstance.h"

class RSE_API FMODSampleInstance : public ISampleInstance
{
public:
	FMODSampleInstance(int channel);
	~FMODSampleInstance();

	virtual void pause();
	virtual void play();
	virtual bool playing();
	virtual void stop();

	virtual void set3D(const Point3& pos, const Point3& vel);

protected:
	int m_channel;
};