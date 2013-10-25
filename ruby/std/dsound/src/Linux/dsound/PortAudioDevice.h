#ifndef DSOUND_PORTAUDIODEVICE_H
#define DSOUND_PORTAUDIODEVICE_H

#include "dsound/common.h"
#include "dsound/SoundDevice.h"

class PortAudioDevice : public SoundDevice
{
public:
	PortAudioDevice(class SoundLibrary& library, const PaDeviceInfo& deviceInfo, PaDeviceIndex deviceIndex);

	virtual std::string description() const;
    virtual bool opened() const;
	virtual bool playing() const;
	virtual void restart();
	virtual float samplesPerSec() const;
	virtual void setPlayCursor(int samples);
	virtual int playCursor() const;
	virtual int playCursorMaxSamples() const;
	virtual int writeCursor() const;

protected:
	static int callback(
		const void *input,
		void *output,
		unsigned long frameCount,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags,
		void *userData);

	virtual void close();
	virtual void open();
	virtual void play();
	virtual void stop();

	virtual uint bufferSize() const;
	virtual double latency() const;

	const PaDeviceInfo& _deviceInfo;
	PaDeviceIndex _deviceIndex;
	bool _playing;
	PaStream* _stream;
};

#endif
