// TBD: run periodic assertion to ensure that all output read cursors are within m_buffer.size()

#include "dsound/pch.h"
#include "BufferFormat.h"
#include "SoundBuffer.h"
#include "BufferModifier.h"

BufferFormat::LocalFormat BufferFormat::Local;

////

SoundBuffer::SoundBuffer(SoundLibrary& library, float bufferLengthSeconds, int buffers) :
	m_buffers(buffers),
	m_secondsPerBuffer(bufferLengthSeconds),
	m_bufferWriteCursor(0),
	m_bufferThread(0),
	m_updatingStream(new CriticalSection),
	m_updatingClient(new CriticalSection),
	_library(library)
{
	m_samplesPerBuffer = (int)(samplesPerSec() * m_secondsPerBuffer);

	m_buffer.resize(m_samplesPerBuffer * m_buffers);

	// setup streaming data
	m_nextBufferSegmentFillRequest = new AsyncEvent;
	m_nextBufferSegmentFillCompleted = new AsyncEvent;
	m_nextBufferSegmentFillCompleted->signal();
	m_bufferThread = new TThread<TDelegate<SoundBuffer> >(TDelegate<SoundBuffer>(this, &SoundBuffer::updateStream), false, Thread::TIME_CRITICAL);
}

SoundBuffer::~SoundBuffer()
{
	stopThread();

	delete m_nextBufferSegmentFillRequest;
	delete m_nextBufferSegmentFillCompleted;

	delete m_updatingStream;
}

SoundGeneratorOutput* SoundBuffer::createOutput()
{
	return new SoundGeneratorOutput(*this);
}

void SoundBuffer::restart()
{
	initBuffer();
	m_valid = false;
}

void SoundBuffer::stopThread()
{
	if (m_bufferThread)
	{
		m_bufferThread->stopRequest();
		m_nextBufferSegmentFillRequest->signal();
		m_bufferThread->waitForExit();
	}

	m_bufferThread = 0;
}

void SoundBuffer::initBuffer()
{
	// initalize stream buffer by filling first segment
	m_bufferWriteCursor = 0;
	m_bufferFillTriggerPosition = 0;

	// reset output's read cursors
	for (int i = 0; i < (int)m_outputs.size(); ++i)
	{
		SoundGeneratorOutput& o = *m_outputs[i];
		o.invalidate();
	}

	loadNextSegment();
	waitForSegmentFill();
}

void SoundBuffer::loadNextSegment()
{
	// tbd: need a better way to reset segment fill completed, probably hanging osx
	m_nextBufferSegmentFillRequest->signal();
}

void SoundBuffer::waitForSegmentFill()
{
	m_nextBufferSegmentFillCompleted->wait();
}

void SoundBuffer::updateStream()
{
	// fix: null check required because thread starts before variable assignment. Fix this
	// by passing in the thread object to the delegate, or by using functor threads.
	while (!m_bufferThread || !m_bufferThread->exiting())
	{
		AsyncWait(*m_nextBufferSegmentFillRequest);

		if (!m_bufferThread->exiting())
		{
			// fill next segment
			if (fillSelf(m_buffer.get(m_bufferWriteCursor), m_samplesPerBuffer))
			{
				m_isFilled = true;
				m_bufferFillTriggerPosition = m_bufferWriteCursor;
				m_buffer.advance(m_bufferWriteCursor, m_samplesPerBuffer);
			}
			else
			{
				m_isFilled = false;
			}
		}

		m_nextBufferSegmentFillCompleted->signal();
	}
}

bool SoundBuffer::handleGet(SoundGeneratorOutput& output, BufferFormat::LocalFormat::SampleElement* bufOut, int samples)
{
	// get requested samples from local buffer
	// clients shouldn't be reading more bytes than the buffer segment size -- the decoding thread won't be able to keep up
	if (samples > m_samplesPerBuffer)
	{
		assert(0);
		samples = m_samplesPerBuffer;
	}

	BufferFormat::LocalFormat::SampleElement* workBufOut = bufOut;

	m_valid = true;

	while (samples > 0 && m_valid)
	{
		if (!m_isFilled)
		{
			BufferFormat::LocalFormat::clear(workBufOut, samples);
			return false;
		}

		// we're out of samples, get samples from stream buffer and modify them
		assert(output.remainingSamples >= 0);
		if (output.remainingSamples == 0)
		{
	#if _DEBUG
			// check to see if we'll overwrite data any of our clients are still using; shouldn't happen in normal operation.
			//for (uint i = 0; i < m_outputs.size(); ++i)
			//{
			//	SoundGeneratorBufferedOutput* o = (SoundGeneratorBufferedOutput*)m_outputs[i];
			//	assert(!o->m_buffer || o == &output || (o->m_buffer - m_modified) > workSamples);
			//}
	#endif
			if (!output.valid())
			{
				if (m_outputs.size() == 1)
					output.readCursor = m_bufferFillTriggerPosition;
				else
				{
					for (uint i = 0; i < m_outputs.size(); ++i)
					{
						if (m_outputs[i]->valid())
							output.readCursor = std::max(output.readCursor, m_outputs[i]->readCursor);
					}
				}

				output.validate();
			}

			output.remainingSamples = m_buffer.distance(output.readCursor, m_bufferWriteCursor);
			if (output.remainingSamples == 0)
			{
#if !IS_LINUX // tbd
				m_zeroSamples << "Error: Output read cursor equals write cursor. Increase stream buffer length or reduce stream buffer processing time." << dlog.endl;
#endif
				loadNextSegment();
				return true;
			}
			
			// if less samples are available than what was requested, then the read cursor has likely passed or will pass the write cursor, 
			// the buffer fill has taken too long, and the request can be only partially fulfilled.
			if (output.remainingSamples < samples)
			{
#if !IS_LINUX // tbd
				m_underrun << "SoundBuffer: Buffer underrun, " << samples - output.remainingSamples << " samples. Increase stream buffer length or reduce stream buffer processing time." << dlog.endl;
#endif
			}
		}

		int startReadCursor = output.readCursor;
		int copySamples = std::min(output.remainingSamples, samples);

		// copy from source data buffer to out buffer
		m_buffer.copyTo(output.readCursor, workBufOut, copySamples);
		m_buffer.advance(output.readCursor, copySamples);
		workBufOut += copySamples * 2;
		output.remainingSamples -= copySamples;
		assert(output.remainingSamples >= 0);
		samples -= copySamples;

		// initiate fill request for next buffer segment, if necessary
		if (m_buffer.passed(startReadCursor, m_bufferFillTriggerPosition, output.readCursor))
			loadNextSegment();
	}

	return true;
}

bool SoundBuffer::get(SoundGeneratorOutput& output, short* buf, int samples)
{
	if (!enabled())
		return false;

	return handleGet(output, BufferFormat::Local.buf(buf), samples);
}

bool SoundBuffer::get(SoundGeneratorOutput& output, float* buf, int samples)
{
	if (!enabled())
		return false;

	return handleGet(output, BufferFormat::Local.buf(buf), samples);
}

const BufferFormat::LocalFormat::SampleElement* SoundBuffer::data() const
{
	return m_buffer.buf();
}

