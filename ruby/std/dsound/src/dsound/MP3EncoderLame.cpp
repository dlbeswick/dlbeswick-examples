#include "pch.h"
#include "MP3EncoderLame.h"

#if IS_MSVC
	#include "lame.h"
#else
	#include "lame/lame.h"
#endif

#include "BufferFormat.h"

MP3EncoderLame::MP3EncoderLame(class SoundLibrary& library, float samples_per_sec, float channels, const char* fName) :
	Transcoder(library, samples_per_sec, channels, fName),
	m_bufferLeft(0),
	m_bufferRight(0),
	m_bufferOutput(0)
{
}

void MP3EncoderLame::initEncoder()
{
	std::string file_path = m_path + ".mp3";

	m_stream.open(file_path.c_str(), std::ios::trunc | std::ios::binary);
	if (!m_stream.is_open())
		throwf("MP3Encoder: Couldn't open %s for writing.", file_path.c_str());

	lame_global_flags* global_flags = lame_init();
	m_globalFlags = (void*)global_flags;

    // bitrate
	//lame_set_brate((lame_global_flags*)m_globalFlags, 192);

	// equivalent to "medium" preset, from lame frontend source
	lame_set_VBR_q(global_flags, 4);
	lame_set_VBR(global_flags, vbr_default);
	
	// "standard" preset, from lame frontend source
	//lame_set_VBR_q(gfp, 2);
	//lame_set_VBR(gfp, vbr_default);

	int ret_code = lame_init_params((lame_global_flags*)m_globalFlags);
	if (ret_code < 0)
	{
		throwf("Couldn't initialize LAME encoder.");
	}
	
    // output from dsound is -1.0->1.0, lame expects -32768.0->32768.0, so set scale.
    float scale = 32768.0f; 
    
    // default LAME flags often scale down by a small amount, maybe to prevent encoding artifacts at max range?
    if (lame_get_scale((lame_global_flags*)m_globalFlags) != 0)
        scale *= lame_get_scale((lame_global_flags*)m_globalFlags);
    
    lame_set_scale((lame_global_flags*)m_globalFlags, scale);
    
	m_writeSamples = 44100 / 10;
	m_outputSize = (int)(1.25 * m_writeSamples + 7200);
	
	m_writeBytes = (BufferFormat::Local.bytesPerSample() * m_writeSamples);
	m_writtenBytes = 0;
	
	m_bufferLeft = new char[m_writeBytes / 2];
	m_bufferRight = new char[m_writeBytes / 2];
	m_bufferOutput = new char[m_outputSize];
	
	m_writeLeft = BufferFormat::Local.buf(m_bufferLeft);
	m_writeRight = BufferFormat::Local.buf(m_bufferRight);
}

MP3EncoderLame::~MP3EncoderLame()
{
	delete[] m_bufferLeft;
	delete[] m_bufferRight;
	delete[] m_bufferOutput;
}

void MP3EncoderLame::consume(const void* data, int bytes)
{
	Critical(_stream_access);

	if (!m_stream.is_open())
		throwf("MP3Encoder: consume called, but stream is not open.");

	const BufferFormat::LocalFormat::SampleElement* eData = BufferFormat::Local.buf((void*)data);

	int bytesPerSample = BufferFormat::Local.bytesPerSample();

	while (bytes > 0)
	{
		assert(m_writeBytes != 0);

		while (bytes > 0 && m_writtenBytes < m_writeBytes)
		{
			*m_writeLeft++ = *eData++;
			*m_writeRight++ = *eData++;
			
			m_writtenBytes += bytesPerSample;
			bytes -= bytesPerSample;
		}

		assert(bytes >= 0);

		if (m_writeBytes == 0 || m_writtenBytes == m_writeBytes)
		{
			int result = lame_encode_buffer_float((lame_global_flags*)m_globalFlags, (float*)m_bufferLeft, (float*)m_bufferRight, m_writtenBytes / bytesPerSample, (uchar*)m_bufferOutput, m_outputSize);
			if (result < 0)
				throwf("MP3EncoderLame: encode error (%d)", result);

			m_stream.write(m_bufferOutput, result);
			if (!m_stream.good())
				throwf("MP3EncoderLame: error writing to file (%s)", m_path.c_str());

			m_writeLeft = BufferFormat::Local.buf(m_bufferLeft);
			m_writeRight = BufferFormat::Local.buf(m_bufferRight);
			m_writtenBytes = 0;
		}
	}
}

void MP3EncoderLame::flush()
{
	Critical(_stream_access);
	m_stream.flush();
}

void MP3EncoderLame::close()
{
	Critical(_stream_access);
	m_stream.close();
}
