// ------------------------------------------------------------------------------------------------
//
// IMusic
//
// ------------------------------------------------------------------------------------------------
#pragma once

#include "Standard/PtrGCHost.h"

class RSE_API IMusic : public PtrGCHost
{
public:
	virtual void play() = 0;
	virtual void stop() = 0;

	virtual float volume() const = 0;
	virtual bool playing() const = 0;

	virtual void setVolume(float f) = 0;

	virtual bool ready() const = 0;

protected:
	virtual float globalMusicVolume() const = 0;
};