// ------------------------------------------------------------------------------------------------
//
// SoundListener
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include <Standard/Base.h>

class RSE_API SoundListener
{
public:
	virtual ~SoundListener();

	virtual Point3 listenerOrigin() const = 0;
	virtual Point3 listenerVelocity() const = 0;
	virtual Quat listenerRotation() const = 0;
	virtual const Matrix4& listenerReferenceFrame() const = 0;
};