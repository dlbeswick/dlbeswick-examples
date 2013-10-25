#pragma once
#if 0
#include "BufferFormat.h"
#include "BufferModifierHost.h"
#include "SoundConsumer.h"
#include "Standard/CircularBuffer.h"
#include "Standard/Timer.h"
#include "Standard/TimedLog.h"

class AsyncEvent;
class AsyncEvents;
class SoundBuffer;

class CoreAudioBuffer : public SoundConsumer
{
public:
	CoreAudioBuffer(class DirectSoundDevice& DS, float latency = 0.05f);

	virtual ~CoreAudioBuffer();

	virtual void play();

	virtual bool playing() const;

	virtual void stop();

	virtual int freq() const;

	virtual void setFreq(int freq);

	virtual float volume() const;

	virtual void setVolume(float v);

	virtual double progress() const;

	virtual void setProgress(double ms);

	virtual void setPlayCursor(DWORD bytes);

	virtual DWORD playCursor() const;
	virtual DWORD writeCursor() const;

	virtual void restart();

	virtual float samplesPerSec() const { return 44100.0f; }

protected:
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
	ICoreAudioBuffer8* m_dsBuffer;
	TThread<TDelegate<CoreAudioBuffer> >* m_bufferThread;
	TThread<TDelegate<CoreAudioBuffer> >* m_dsThread;
	AsyncEvents* m_dsWriteNeeded;
	CircularBuffer<BufferFormat::LocalFormat::SampleElement> m_work;

	Timer m_frameTimer;
	TimedLogErr m_latencyLog;
	char* m_visSamples;
};
#endif