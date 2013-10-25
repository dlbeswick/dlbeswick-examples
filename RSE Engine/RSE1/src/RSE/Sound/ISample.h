// ------------------------------------------------------------------------------------------------
//
// ISample
// Interface to a sample loaded in memory.
// Samples return ISampleInstance objects when they are played.
//
// ------------------------------------------------------------------------------------------------
#ifndef STANDARD_ISAMPLE_H
#define STANDARD_ISAMPLE_H

#include "RSE/RSE.h"
#include "Standard/SmartPtr.h"
class ISampleInstance;
class ibinstream;

class RSE_API ISample
{
public:
	virtual ~ISample() {};

	virtual SmartPtr<ISampleInstance> play() = 0;
};

#endif