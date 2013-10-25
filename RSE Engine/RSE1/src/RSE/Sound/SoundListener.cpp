// ------------------------------------------------------------------------------------------------
//
// SoundListener
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "AppBase.h"
#include "ISoundProvider.h"
#include "SoundListener.h"

SoundListener::~SoundListener()
{
	if (AppBase().isRunning())
		AppBase().sound().onListenerDestroyed(*this);
}
