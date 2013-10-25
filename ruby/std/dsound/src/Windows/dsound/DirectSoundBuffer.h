// TBD: incorporate this class fully into DirectSoundDevice. This became necessary after introducing the SoundDevice base class.
#ifndef DSOUND_DIRECTSOUNDBUFFER_H
#define DSOUND_DIRECTSOUNDBUFFER_H

#include "dsound/BufferFormat.h"
#include "dsound/BufferModifierHost.h"
#include "dsound/SoundConsumer.h"
#include <Standard/CircularBuffer.h>
#include <Standard/Timer.h>
#include <Standard/TimedLog.h>

class AsyncEvent;
class AsyncEvents;
class SoundBuffer;

class DirectSoundBuffer : public SoundConsumer
{
public:
	DirectSoundBuffer(class DirectSoundDevice& DS, float latency = 0.05f);

	virtual ~DirectSoundBuffer();

	virtual void play();
	
	virtual bool playing() const;

	virtual void stop();

	virtual int freq() const;

	virtual void setFreq(int freq);

	virtual float volume() const;

	virtual void setVolume(float v);

	virtual double progress() const;

	virtual void setProgress(double ms);

	virtual void setPlayCursor(int samples);

	virtual DWORD playCursor() const;
	virtual DWORD writeCursor() const;

	virtual void restart();

	virtual float samplesPerSec() const { return 44100.0f; }

protected:
	friend class DirectSoundDevice;

	virtual void fillSegment(int idx);
	virtual int segmentSamplePos(int idx);
	virtual DWORD status() const;
	void stopThread();
	virtual void updateDS();

	DWORD m_bufferWriteCursor;
	DWORD m_bufferFillTriggerPosition;
	DWORD m_bufferReadCursor;
	DWORD m_dsBufferBytes;
	int m_segmentSamples;
	int m_totalSamples;
	int m_localWritePosSamples;
	IDirectSoundBuffer8* m_dsBuffer;
	TThread<TDelegate<DirectSoundBuffer> >* m_bufferThread;
	TThread<TDelegate<DirectSoundBuffer> >* m_dsThread;
	AsyncEvents* m_dsWriteNeeded;
	CircularBuffer<BufferFormat::LocalFormat::SampleElement> m_work;
	DirectSoundDevice* _device;

	Timer m_frameTimer;
	TimedLogErr m_latencyLog;
	char* m_visSamples;
};

#endif
