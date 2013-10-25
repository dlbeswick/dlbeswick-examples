#include "dsound/pch.h"
#include "SoundBufferMedia.h"
#include "BufferFormat.h"
#include "MP3Decoder.h"
#include "DecoderSox.h"

SoundBufferMedia::SoundBufferMedia(SoundLibrary& library, float seconds) :
	SoundBuffer(library, seconds),
	m_decoder(0),
	m_opening(false)
{
}

SoundBufferMedia::~SoundBufferMedia()
{
	stopThread();
	delete m_decoder;
}

void SoundBufferMedia::open(const char* filePath)
{
	m_openPath = filePath;
	new TThread<TDelegate<SoundBufferMedia> >(TDelegate<SoundBufferMedia>(this, &SoundBufferMedia::doOpen));
}

void SoundBufferMedia::doOpen()
{
	m_opening = true;
	setEnabled(false);
	
	{
		Critical(m_decoderAccess);

		if (m_decoder)
		{
			delete m_decoder;
			m_decoder = 0;
		}

		dlog << "SoundBufferMedia opening " << m_openPath << "...\n";

		_is_mp3decoder = m_openPath.substr(m_openPath.size() - 4) == ".mp3";

		if (_is_mp3decoder)
		{
			dlog << "SoundBufferMedia using MP3Decoder." << dlog.endl;

			std::ifstream* stream = new std::ifstream(m_openPath.c_str(), std::ios::binary);
			if (stream->fail())
			{
				delete stream;
				throw(ExceptionString("Couldn't open file."));
			}

			std::ifstream* streamFrameMap = new std::ifstream(m_openPath.c_str(), std::ios::binary);
			if (streamFrameMap->fail())
			{
				delete stream;
				delete streamFrameMap;
				throw(ExceptionString("Couldn't open frame map stream."));
			}

			id3_file* file = id3_file_open(m_openPath.c_str(), ID3_FILE_MODE_READONLY);
			if (!file)
			{
				throw(ExceptionString("Couldn't read id3 tag."));
			}

			m_decoder = new MP3Decoder(_library, stream, streamFrameMap, *file);

			id3_file_close(file);
		}
		else
		{
			dlog << "SoundBufferMedia using DecoderSox." << dlog.endl;
			m_decoder = new DecoderSox(_library, m_openPath);
		}
	}

	initBuffer();
	m_opening = false;
}

double SoundBufferMedia::progress() const
{
	Critical(m_decoderAccess);

	if (!m_decoder)
		return 0;

	return m_decoder->progress();

	// this extra code was maybe for organising the buffer sync. it shouldn't be needed anymore.
	/*
		- ((std::max(0.0f, (float)m_bufferWriteCursor) / 44100.0f) * 1000.0f)
			- ((m_decoder->unprocessedSamples() / 44100.0f) * 1000.0f);*/
}

void SoundBufferMedia::setProgress(double ms)
{
	{
		Critical(m_decoderAccess);
		{
			Critical(*m_updatingStream);
			if (m_decoder)
			{
				assert(ms >= 0);

				// ensure we are not seeking past the beginning of the stream
				ms = std::max(0.0, ms);

				m_decoder->seek(ms);
			}
		}
	}
	restart();
}

double SoundBufferMedia::lengthMS() const
{
	Critical(m_decoderAccess);

	if (!m_decoder)
		return 0;

	return m_decoder->lengthMS();
}

bool SoundBufferMedia::fillSelf(short* buf, int samples)
{
	Critical(m_decoderAccess);

	if (!m_decoder || m_decoder->finished())
		return false;

	int retrieved_samples = m_decoder->getPCM(buf, samples);

	// clear remainder of buffer if fewer samples were retrieved than were required.
	if (retrieved_samples < samples)
		memset(buf, 0, (samples - retrieved_samples) * sizeof(short) * 2);

	return true;
}

bool SoundBufferMedia::fillSelf(float* buf, int samples)
{
	Critical(m_decoderAccess);

	if (!m_decoder || m_decoder->finished())
		return false;

	int retrieved_samples = m_decoder->getPCM(buf, samples);

	// clear remainder of buffer if fewer samples were retrieved than were required.
	if (retrieved_samples < samples)
		memset(buf, 0, (samples - retrieved_samples) * sizeof(float) * 2);

	return true;
}

void SoundBufferMedia::waitForLengthCalculation() const
{ 
	Critical(m_decoderAccess);

	// note: standardise interface, or remove this method
	if (m_decoder && _is_mp3decoder)
		((MP3Decoder*)m_decoder)->waitForFrameMap();
}

int SoundBufferMedia::rate() const
{
  Critical(m_decoderAccess);

  if (!m_decoder)
	return 0;
  else
	return m_decoder->rate();
}
