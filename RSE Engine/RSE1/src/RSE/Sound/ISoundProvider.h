// ------------------------------------------------------------------------------------------------
//
// ISoundProvider
// Base interface for providers of sound services.
//
// ------------------------------------------------------------------------------------------------
#pragma once

#include "Standard/Base.h"
#include "Standard/PtrGC.h"

class ISample;
class IMusic;
class SoundListener;
class PathResource;

class RSE_API ISoundProvider : public Base
{
public:
	virtual ~ISoundProvider() {}

	virtual ISample& sample(const PathResource& path) = 0;
	virtual PtrGC<IMusic> music(const PathResource& path) = 0;

	// setListener
	// sets the object used for listener position info
	virtual void setListener(const SoundListener* listener) = 0;

	virtual void setGlobalVolume(float v) = 0;
	virtual void setSoundVolume(float v) = 0;
	virtual void setMusicVolume(float v) = 0;

	virtual float globalVolume() const = 0;
	virtual float soundVolume() const = 0;
	virtual float musicVolume() const = 0;

	// callbacks
	// call when SoundListener is destroyed
	virtual void onListenerDestroyed(const SoundListener& listener) = 0;
};