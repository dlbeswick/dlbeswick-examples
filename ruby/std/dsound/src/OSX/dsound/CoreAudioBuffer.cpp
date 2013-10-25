#if 0
#include "../pch.h"
#include "BufferFormat.h"
#include "CoreAudioBuffer.h"
#include "CoreAudioDevice.h"
#include "BufferModifier.h"
#include "SoundBuffer.h"

BufferFormat::Short BufferFormat::Library;


CoreAudioBuffer::DirectSoundBuffer(DirectSoundDevice& DS, float latency) :
	m_bufferWriteCursor(0),
	m_bufferReadCursor(0),
	m_bufferFillTriggerPosition(0),
	m_dsThread(0),
	m_frameTimer(FLT_MAX),
	m_latencyLog(1.0f)
{
	// create directsound objects
	DSBUFFERDESC desc;
	memset(&desc, 0, sizeof(desc));

	WAVEFORMATEX format;
	memset(&format, 0, sizeof(format));

	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = 2;
	format.nSamplesPerSec = 44100;
	format.wBitsPerSample = (BufferFormat::Library.bytesPerSample() * 8) / 2;
	format.nBlockAlign = (format.nChannels * format.wBitsPerSample) / 8;
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;

	const int fillSegments = 4;

	m_segmentSamples = (int)(format.nSamplesPerSec * latency);
	m_totalSamples = m_segmentSamples * fillSegments;
	m_dsBufferBytes = m_totalSamples * format.nBlockAlign;

	desc.dwSize = sizeof(desc);
	desc.dwFlags = DSBCAPS_LOCSOFTWARE | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLVOLUME | DSBCAPS_GETCURRENTPOSITION2;
	desc.dwBufferBytes = m_dsBufferBytes;
	desc.lpwfxFormat = &format;

	m_work.resize(m_segmentSamples * 2);
	m_localWritePosSamples = 0;

	ICoreAudioBuffer* buf;
	if (!DS.device())
		throwf("Directsound device not yet initialized");

	ensureDS(DS.device()->CreateSoundBuffer(&desc, &buf, 0));
	buf->QueryInterface(IID_ICoreAudioBuffer8, (LPVOID*)&m_dsBuffer);
	buf->Release();

	m_dsWriteNeeded = new AsyncEvents(fillSegments);

	DSBPOSITIONNOTIFY* notifies = new DSBPOSITIONNOTIFY[fillSegments];

	for (int i = 0; i < fillSegments; ++i)
	{
		notifies[i].dwOffset = m_segmentSamples * BufferFormat::Library.bytesPerSample() * i;
		notifies[i].hEventNotify = m_dsWriteNeeded->handle(i);
	}

	ICoreAudioNotify8* dsNotify;
	ensureDS(m_dsBuffer->QueryInterface(IID_ICoreAudioNotify8, (LPVOID*)&dsNotify));
	dsNotify->SetNotificationPositions(fillSegments, notifies);
	dsNotify->Release();

	delete[] notifies;

	// setup streaming data
	m_dsThread = new TThread<TDelegate<CoreAudioBuffer> >(TDelegate<DirectSoundBuffer>(this, updateDS), true, Thread::HIGHEST);
}

CoreAudioBuffer::~DirectSoundBuffer()
{
	stopThread();

	m_dsBuffer->Release();
}

void CoreAudioBuffer::play()
{
	m_dsThread->resume();
	ensureDS(m_dsBuffer->Play(0, 0, DSBPLAY_LOOPING));
}

bool CoreAudioBuffer::playing() const
{
	return status() & DSBSTATUS_PLAYING;
}

void CoreAudioBuffer::stop()
{
	m_dsBuffer->Stop();
}

int CoreAudioBuffer::freq() const
{
	DWORD freq;
	m_dsBuffer->GetFrequency(&freq);
	return freq;
}

void CoreAudioBuffer::setFreq(int  freq)
{
	m_dsBuffer->SetFrequency(freq);
}

float CoreAudioBuffer::volume() const
{
	LONG v;
	m_dsBuffer->GetVolume(&v);
	return Mapping::linear((float)v, (float)DSBVOLUME_MIN, (float)DSBVOLUME_MAX, 0.0f, 1.0f);
}

void CoreAudioBuffer::setVolume(float  v)
{
	m_dsBuffer->SetVolume((LONG)Mapping::linear(log10f(1.0f + v * 10.0f), 0.0f, 1.0f, (float)DSBVOLUME_MIN, (float)DSBVOLUME_MAX));
}

double CoreAudioBuffer::progress() const
{
	return 0;
}

void CoreAudioBuffer::setProgress(double  ms)
{
}

void CoreAudioBuffer::setPlayCursor(DWORD bytes)
{
	m_dsBuffer->SetCurrentPosition(bytes);
}

DWORD CoreAudioBuffer::playCursor() const
{
	DWORD play;

	m_dsBuffer->GetCurrentPosition(&play, 0);

	return play;
}

DWORD CoreAudioBuffer::writeCursor() const
{
	DWORD write;

	m_dsBuffer->GetCurrentPosition(0, &write);

	return write;
}

void CoreAudioBuffer::restart()
{
}

DWORD CoreAudioBuffer::status() const
{
	DWORD i;
	m_dsBuffer->GetStatus(&i);
	return i;
}

void CoreAudioBuffer::stopThread()
{
	if (m_dsThread)
		m_dsThread->stop();
	m_dsThread = 0;
}

void CoreAudioBuffer::updateDS()
{
	BufferFormat::LibraryFormat::SampleElement* buf0;
	BufferFormat::LibraryFormat::SampleElement* buf1;
	DWORD bytes0;
	DWORD bytes1;

	float latency = m_work.size() / 2 / samplesPerSec();

	while (true)
	{
		AsyncWait(*m_dsWriteNeeded);

		m_frameTimer.startFrame();

		int segment = m_dsWriteNeeded->activatedIdx();
		int writeSegment = segment + 1;
		int writePosSamples = (m_segmentSamples * writeSegment) % m_totalSamples;

		BufferFormat::LocalFormat::SampleElement* workBuf = m_work.buf();

		consumeTo(workBuf, m_segmentSamples);

		int lockStart = writePosSamples * BufferFormat::Library.bytesPerSample();
		int lockEnd = lockStart + m_segmentSamples * BufferFormat::Library.bytesPerSample();

		ensureDS(
			m_dsBuffer->Lock(
				lockStart,
				lockEnd - lockStart,
				(void**)&buf0, &bytes0, (void**)&buf1, &bytes1, 0
				)
			);

		BufferFormat::LibraryFormat::copy(buf0, workBuf, m_segmentSamples);

		ensureDS(m_dsBuffer->Unlock(buf0, bytes0, buf1, bytes1));

		m_frameTimer.endFrame();
		float frameTime = m_frameTimer.frame();

		if (frameTime > latency)
			m_latencyLog << std::string("Directsound buffer frame time was ") + frameTime + ", latency is " + latency + ", reduce processing CPU requirements.";

		m_localWritePosSamples = writePosSamples;
	}
}

#endif
