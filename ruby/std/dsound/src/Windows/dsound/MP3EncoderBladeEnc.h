#pragma once

#ifndef NOMP3

#include "dsound/BufferFormat.h"
#include "dsound/MP3Encoder.h"
#include "Standard/CriticalSection.h"
#include <ostream>
#include <string>

class MP3EncoderBladeEnc : public Transcoder
{
public:
	MP3EncoderBladeEnc(class SoundLibrary& library, const char* fName);
	~MP3EncoderBladeEnc();

	void consume(const void* data, int bytes);

	void flush();

protected:
	virtual void initEncoder();
	void close();

	static std::string error(int error);

	DWORD m_writeSamples;
	int m_writeBytes;
	DWORD m_minOutputSize;
	unsigned long m_handle;

	int m_writtenBytes;
	char* m_bufferLeft;
	char* m_bufferRight;
	char* m_bufferOutput;

	BufferFormat::LocalFormat::SampleElement* m_writeLeft;
	BufferFormat::LocalFormat::SampleElement* m_writeRight;
};

#endif
