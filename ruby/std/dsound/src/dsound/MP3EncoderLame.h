#ifndef DSOUND_MP3ENCODERLAME_H
#define DSOUND_MP3ENCODERLAME_H

#include "MP3Encoder.h"

class MP3EncoderLame : public Transcoder
{
public:
	MP3EncoderLame(class SoundLibrary& library, float samples_per_sec, float channels, const char* fName);
	~MP3EncoderLame();

	virtual void flush();

protected:

	virtual void initEncoder();
	virtual void close();
	virtual void consume(const void* data, int bytes);

	CriticalSection _stream_access;
	std::ofstream m_stream;

	dword m_writeSamples;
	int m_samples;
	int m_writeBytes;
	unsigned long m_handle;
	CriticalSection m_writing;

	int m_writtenBytes;
	char* m_bufferLeft;
	char* m_bufferRight;
	char* m_bufferOutput;
	int m_outputSize;

	void* m_globalFlags;

	BufferFormat::LocalFormat::SampleElement* m_writeLeft;
	BufferFormat::LocalFormat::SampleElement* m_writeRight;
};

#endif
