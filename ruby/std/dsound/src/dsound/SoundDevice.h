#ifndef DSOUND_SOUNDDEVICE_H
#define DSOUND_SOUNDDEVICE_H

#include "SoundConsumer.h"

class SoundDevice : public SoundConsumer
{
public:
	SoundDevice(class SoundLibrary& library) :
		_library(library)
	{};

	virtual void addInput(SoundGeneratorOutput& output);
	virtual void clearInputs();
	virtual std::string description() const = 0;
	virtual void removeInput(SoundGeneratorOutput& output);

    virtual bool opened() const = 0;

	virtual bool playing() const = 0;

	// should do whatever's necessary to begin playing the sound buffer as if created from scratch, for example,
	// clearing and refilling buffers and setting read pointers to the start of the buffer.
	virtual void restart() = 0;

	virtual float samplesPerSec() const = 0;
	virtual void setPlayCursor(int samples) = 0;

	virtual void syncRequestAdd(
		SoundGeneratorOutput& source, 
		SoundGeneratorOutput& dest, 
		SoundDevice& syncee
	);

	virtual int playCursor() const = 0;
	virtual int playCursorMaxSamples() const = 0;
	virtual int writeCursor() const = 0;

protected:
	virtual void close() = 0;
	virtual void open() = 0;
	virtual void play() = 0;
	virtual void stop() = 0;

	struct SyncRequest
	{
		SoundGeneratorOutput* source;
		SoundGeneratorOutput* dest;
		SoundDevice* device;
	};

	SoundLibrary& _library;
	std::vector<SyncRequest> _syncRequests;
	CriticalSection _syncRequestsLock;

	virtual void handleSyncRequests(float syncAfterSeconds);
};

#endif
