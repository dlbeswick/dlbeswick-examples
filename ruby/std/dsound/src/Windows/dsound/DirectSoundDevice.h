#ifndef DSOUND_DIRECTSOUNDDEVICE_H
#define DSOUND_DIRECTSOUNDDEVICE_H

#include "dsound/SoundDevice.h"

class DirectSoundDevice : public SoundDevice
{
public:
	DirectSoundDevice(class SoundLibrary& library, GUID guid, const std::string& description, float latency = 0.05f);
	~DirectSoundDevice();

	bool isDefault() const;
	std::string description() const { return _description.c_str(); }

	void setWindow(int window);

	IDirectSound* device() const { return _ds; }

	virtual void addInput(SoundGeneratorOutput& output);

// SoundDevice
	virtual bool opened() const;

	virtual bool playing() const;

	virtual void restart();

	virtual void setPlayCursor(int samples);

	virtual int playCursor() const;
	virtual int playCursorMaxSamples() const;
	virtual int writeCursor() const;

	virtual float samplesPerSec() const { return 44100.0f; }

protected:
	friend class DirectSoundBuffer;

	virtual void close();
	virtual void open();
	virtual void play();
	virtual void stop();

	class DirectSoundBuffer* _buffer;
	std::string _description;
	IDirectSound8* _ds;
	GUID _guid;
	bool _hasWindow;
	float _latency;
	bool _shouldBePlaying;
};

#endif
