#include "dsound/pch.h"
#include "dsound/BufferFormat.h"
#include "dsound/BufferModifier.h"
#include "dsound/SoundBuffer.h"
#include "DirectSoundBuffer.h"
#include "DirectSoundDevice.h"
#include "Standard/dxerr.h"

BufferFormat::Short BufferFormat::Library;


DirectSoundBuffer::DirectSoundBuffer(DirectSoundDevice& DS, float latency) :
	m_bufferWriteCursor(0),
	m_bufferReadCursor(0),
	m_bufferFillTriggerPosition(0),
	m_dsThread(0),
	m_frameTimer(FLT_MAX),
	m_latencyLog(1.0f),
	_device(&DS)
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

	IDirectSoundBuffer* buf;
	if (!DS.device())
		throwf("Directsound device not yet initialized");

	ensureDS(DS.device()->CreateSoundBuffer(&desc, &buf, 0));
	buf->QueryInterface(IID_IDirectSoundBuffer8, (LPVOID*)&m_dsBuffer);
	buf->Release();

	m_dsWriteNeeded = new AsyncEvents(fillSegments);

	DSBPOSITIONNOTIFY* notifies = new DSBPOSITIONNOTIFY[fillSegments];

	for (int i = 0; i < fillSegments; ++i)
	{
		notifies[i].dwOffset = m_segmentSamples * BufferFormat::Library.bytesPerSample() * i;
		notifies[i].hEventNotify = m_dsWriteNeeded->handle(i);
	}

	IDirectSoundNotify8* dsNotify;
	ensureDS(m_dsBuffer->QueryInterface(IID_IDirectSoundNotify8, (LPVOID*)&dsNotify));
	dsNotify->SetNotificationPositions(fillSegments, notifies);
	dsNotify->Release();

	delete[] notifies;

	// setup streaming data
	m_dsThread = new TThread<TDelegate<DirectSoundBuffer> >(TDelegate<DirectSoundBuffer>(this, &DirectSoundBuffer::updateDS), true, Thread::HIGHEST);
}

DirectSoundBuffer::~DirectSoundBuffer()
{
	stopThread();

	m_dsBuffer->Release();
}

void DirectSoundBuffer::play()
{
	m_dsThread->resume();
	ensureDS(m_dsBuffer->Play(0, 0, DSBPLAY_LOOPING));
}

bool DirectSoundBuffer::playing() const
{
	return status() & DSBSTATUS_PLAYING;
}

void DirectSoundBuffer::stop()
{
	m_dsBuffer->Stop();
}

int DirectSoundBuffer::freq() const
{
	DWORD freq;
	m_dsBuffer->GetFrequency(&freq);
	return freq;
}

void DirectSoundBuffer::setFreq(int  freq)
{
	m_dsBuffer->SetFrequency(freq);
}

float DirectSoundBuffer::volume() const
{
	LONG v;
	m_dsBuffer->GetVolume(&v);
	return Mapping::linear((float)v, (float)DSBVOLUME_MIN, (float)DSBVOLUME_MAX, 0.0f, 1.0f);
}

void DirectSoundBuffer::setVolume(float  v)
{
	m_dsBuffer->SetVolume((LONG)Mapping::linear(log10f(1.0f + v * 10.0f), 0.0f, 1.0f, (float)DSBVOLUME_MIN, (float)DSBVOLUME_MAX));
}

double DirectSoundBuffer::progress() const
{
	return 0;
}

void DirectSoundBuffer::setProgress(double ms)
{
}

void DirectSoundBuffer::setPlayCursor(int samples)
{
	m_dsBuffer->SetCurrentPosition(samples * BufferFormat::Library.bytesPerSample());
}

DWORD DirectSoundBuffer::playCursor() const
{
	DWORD play;

	m_dsBuffer->GetCurrentPosition(&play, 0);

	return play;
}

DWORD DirectSoundBuffer::writeCursor() const
{
	DWORD write;

	m_dsBuffer->GetCurrentPosition(0, &write);

	return write;
}

void DirectSoundBuffer::restart()
{
	bool needsPlay = playing();

	if (playing())
		stop();

	BufferFormat::LibraryFormat::SampleElement* buf0;
	BufferFormat::LibraryFormat::SampleElement* buf1;
	DWORD bytes0;
	DWORD bytes1;

	ensureDS(
		m_dsBuffer->Lock(
			0,
			m_dsBufferBytes,
			(void**)&buf0, &bytes0, (void**)&buf1, &bytes1, 0
		)
	);

	memset(buf0, m_dsBufferBytes, 0);

	ensureDS(m_dsBuffer->Unlock(buf0, bytes0, buf1, bytes1));

	setPlayCursor(0);
	fillSegment(0);
	
	if (needsPlay)
		play();
}

DWORD DirectSoundBuffer::status() const
{
	DWORD i;
	m_dsBuffer->GetStatus(&i);
	return i;
}

void DirectSoundBuffer::stopThread()
{
	if (m_dsThread)
		m_dsThread->stop();
	m_dsThread = 0;
}

void DirectSoundBuffer::fillSegment(int idx)
{
	BufferFormat::LibraryFormat::SampleElement* buf0;
	BufferFormat::LibraryFormat::SampleElement* buf1;
	DWORD bytes0;
	DWORD bytes1;

	int writePosSamples = segmentSamplePos(idx);

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

	BufferFormat::LibraryFormat::scaleCopy(buf0, workBuf, m_segmentSamples, 32767.0f);

	ensureDS(m_dsBuffer->Unlock(buf0, bytes0, buf1, bytes1));

	m_localWritePosSamples = writePosSamples;
}

int DirectSoundBuffer::segmentSamplePos(int idx)
{
	return (m_segmentSamples * idx) % m_totalSamples;
}

void DirectSoundBuffer::updateDS()
{
	float latency = m_work.size() / 2 / samplesPerSec();

	while (true)
	{
		AsyncWait(*m_dsWriteNeeded);

		m_frameTimer.startFrame();

		int segment = m_dsWriteNeeded->activatedIdx();

		_device->handleSyncRequests(
			CircularBufferBase::distance(
				playCursor() / BufferFormat::Library.bytesPerSample(), 
				segmentSamplePos(segment + 1),
				m_totalSamples
			) / samplesPerSec()
		);

		fillSegment(segment + 1);

		m_frameTimer.endFrame();
		float frameTime = m_frameTimer.frame();

		if (frameTime > latency)
			m_latencyLog << std::string("Directsound buffer frame time was ") + frameTime + ", latency is " + latency + ", reduce processing CPU requirements.";
	}
}

