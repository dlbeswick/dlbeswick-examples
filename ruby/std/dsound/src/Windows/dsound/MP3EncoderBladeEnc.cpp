#include "dsound/pch.h"
#include "MP3EncoderBladeEnc.h"

#ifndef NOMP3

#define _BLADEDLL
#include "BladeMP3EncDll.h"

#include "dsound/BufferFormat.h"

MP3EncoderBladeEnc::MP3EncoderBladeEnc(class SoundLibrary& library, const char* fName) :
	m_bufferLeft(0),
	m_bufferRight(0),
	m_bufferOutput(0),
	Transcoder(library, fName)
{
}

void MP3EncoderBladeEnc::initEncoder()
{
	BE_CONFIG config;
	memset(&config, 0, sizeof(config));
	config.dwConfig = BE_CONFIG_LAME;
	config.format.LHV1.dwStructVersion = 1;
	config.format.LHV1.dwStructSize = sizeof(config);
	config.format.LHV1.dwSampleRate = 44100;
	config.format.LHV1.nMode = BE_MP3_MODE_JSTEREO;
	config.format.LHV1.nPreset = LQP_STANDARD;
	config.format.LHV1.nVbrMethod = VBR_METHOD_DEFAULT;
	config.format.LHV1.bWriteVBRHeader = true;
	config.format.LHV1.bEnableVBR = true;

	BE_ERR err = beInitStream(&config, &m_writeSamples, &m_minOutputSize, &m_handle);
	if (err != BE_ERR_SUCCESSFUL)
		throwf("MP3EncoderBladeEnc: couldn't open stream (%s)", error(err).c_str());

	m_writeBytes = (BufferFormat::Local.bytesPerSample() * m_writeSamples);
	m_writtenBytes = 0;

	m_bufferLeft = new char[m_writeBytes / 2];
	m_bufferRight = new char[m_writeBytes / 2];
	m_bufferOutput = new char[m_minOutputSize];

	m_writeLeft = BufferFormat::Local.buf(m_bufferLeft);
	m_writeRight = BufferFormat::Local.buf(m_bufferRight);
}

MP3EncoderBladeEnc::~MP3EncoderBladeEnc()
{
	Critical(m_writing);
	close();
	delete[] m_bufferLeft;
	delete[] m_bufferRight;
	delete[] m_bufferOutput;
}

std::string MP3EncoderBladeEnc::error(int error)
{
	switch (error)
	{
		case BE_ERR_INVALID_FORMAT:	return "BE_ERR_INVALID_FORMAT";
		case BE_ERR_INVALID_FORMAT_PARAMETERS:	return "BE_ERR_INVALID_FORMAT_PARAMER";
		case BE_ERR_NO_MORE_HANDLES:	return "BE_ERR_NO_MORE_HANDLE";
		case BE_ERR_INVALID_HANDLE:	return "BE_ERR_INVALID_HANDLE";
		case BE_ERR_BUFFER_TOO_SMALL:	return "BE_ERR_BUFFER_TOO_SMALL";
		default: return "BE_ERR_SUCCESSFUL";
	};
}

void MP3EncoderBladeEnc::consume(const void* data, int bytes)
{
	if (!m_stream.is_open())
		throwf("MP3EncoderBladeEnc: Write called, but stream is not open.");

	if (!m_stream.good())
		return;

	Critical(m_writing);

	const BufferFormat::LocalFormat::SampleElement* eData = BufferFormat::Local.buf((void*)data);

	int bytesPerSample = BufferFormat::Local.bytesPerSample();

	while (bytes > 0)
	{
		while (bytes > 0 && m_writtenBytes < m_writeBytes)
		{
			*m_writeLeft++ = *eData++ * 32767.0f;
			*m_writeRight++ = *eData++ * 32767.0f;

			m_writtenBytes += bytesPerSample;
			bytes -= bytesPerSample;
		}

		assert(bytes >= 0);

		if (m_writtenBytes == m_writeBytes)
		{
			DWORD outputSamples = m_minOutputSize;

			BE_ERR err = beEncodeChunkFloatS16NI(m_handle, m_writeSamples, (float*)m_bufferLeft, (float*)m_bufferRight, (PBYTE)m_bufferOutput, &outputSamples);
			if (err != BE_ERR_SUCCESSFUL)
				throwf("MP3EncoderBladeEnc: encode error (%s)", error(err).c_str());

			m_stream.write(m_bufferOutput, outputSamples);
			if (!m_stream.good())
				derr << "MP3EncoderBladeEnc: error writing to file " << m_path.c_str() << derr.endl;

			m_writeLeft = BufferFormat::Local.buf(m_bufferLeft);
			m_writeRight = BufferFormat::Local.buf(m_bufferRight);
			m_writtenBytes = 0;
		}
	}
}

void MP3EncoderBladeEnc::flush()
{
	m_stream.flush();
}

void MP3EncoderBladeEnc::close()
{
	m_stream.close();
	beWriteVBRHeader(m_path.c_str());
}

#endif
