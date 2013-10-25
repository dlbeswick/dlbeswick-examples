#pragma once
#include "BufferFormat.h"
#include "SoundGeneratorOutput.h"
#include "Standard/CircularBuffer.h"
#if !IS_LINUX // tbd
#include "Standard/TimedLog.h"
#endif

class AsyncEvent;
class AsyncEvents;

class SoundBuffer : public SoundGenerator
{
public:
	SoundBuffer(class SoundLibrary& library, float bufferLengthSeconds = 0.5f, int buffers = 3);

	virtual ~SoundBuffer();

	virtual void restart();
	
	virtual void initBuffer();

	virtual double progress() const = 0;

	virtual void setProgress(double ms) = 0;

	virtual void stopThread();

	virtual bool get(SoundGeneratorOutput& output, short* buf, int samples);
	virtual bool get(SoundGeneratorOutput& output, float* buf, int samples);

	virtual const BufferFormat::LocalFormat::SampleElement* data() const;

protected:
	virtual void waitForSegmentFill();

	virtual SoundGeneratorOutput* createOutput();

	virtual bool fillSelf(short* buf, int samples) = 0;
	virtual bool fillSelf(float* buf, int samples) = 0;

	bool handleGet(SoundGeneratorOutput& output, BufferFormat::LocalFormat::SampleElement* bufOut, int samples);
	void writeToBuffer(BufferFormat::LocalFormat::SampleElement* buf, BufferFormat::LocalFormat::SampleElement* buf0, int samples0, BufferFormat::LocalFormat::SampleElement* buf1, int samples1);

	void loadNextSegment();

	virtual void updateStream();

	int m_buffers;
	int m_samplesPerBuffer;
	bool m_isFilled;
	bool m_valid;
	float m_secondsPerBuffer;
	int m_bufferWriteCursor;
	int m_bufferFillTriggerPosition;
	CircularBuffer<BufferFormat::LocalFormat::SampleElement, 2> m_buffer;
	TThread<TDelegate<SoundBuffer> >* m_bufferThread;
	AsyncEvent* m_nextBufferSegmentFillRequest;
	AsyncEvent* m_nextBufferSegmentFillCompleted;

	CriticalSection* m_updatingStream;
	CriticalSection* m_updatingClient;

	class SoundLibrary& _library;

#if !IS_LINUX // tbd
	TimedLogErr m_zeroSamples;
	TimedLogErr m_underrun;
#endif
};
