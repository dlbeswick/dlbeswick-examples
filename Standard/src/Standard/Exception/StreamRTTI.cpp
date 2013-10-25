#include "Standard/pch.h"
#include "StreamRTTI.h"
#include "Standard/Base.h"

ExceptionStreamRTTI::ExceptionStreamRTTI(const Base& subject)
{
	addContext(subject.rtti().className());
}
