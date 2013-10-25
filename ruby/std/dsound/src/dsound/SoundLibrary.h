#ifndef _DSOUND_SOUNDLIBRARY_H
#define _DSOUND_SOUNDLIBRARY_H

#include "dsound/common.h"

class SoundDevice;

class SoundLibrary
{
public:
	virtual ~SoundLibrary();

	virtual void close() = 0;
	SoundDevice& defaultDevice() const { throw("Unimplemented."); }
	std::vector<SoundDevice*> devices() const { throw("Unimplemented."); return std::vector<SoundDevice*>(); }

	virtual float desiredLatency() const = 0;
	virtual void setDesiredLatency(float latency) = 0;

	virtual void open() = 0;
	virtual void refreshDevices() = 0;

#if WITH_SOX
	virtual void ensureSox();
#endif

protected:
	// A human-readable string identifying the name or function of the client using the library.
	// The actualy function of this string is platform-specific.
	SoundLibrary(const std::string& client_name);

#if WITH_SOX
	bool _sox_inited;
#endif
};

#endif
