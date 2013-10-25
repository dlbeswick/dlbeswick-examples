#include "dsound/pch.h"
#include "MP3Decoder.h"

#define INPUT_BUFFER_SIZE	(8192)

MP3Decoder::MP3Decoder(class SoundLibrary& library, std::istream* stream, std::istream* frameMapStream, id3_file& tagFile) :
	DSoundDecoder(library),
	m_istream(stream),
	m_istreamFrameMap(frameMapStream),
	m_unprocessedSamples(0),
	m_currentSample(0),
	m_finished(false),
	m_frameMapProgress(0),
	m_frameMapAccess(new CriticalSection),
	m_decoding(new CriticalSection),
	m_streamLengthMS(1000),
	m_frameMapBuilt(false),
	m_skipFrameSamplesNormalized(0),
	_sample_rate(0)
{
	assert(m_istream);
	assert(!m_istream->fail());
	assert(m_istreamFrameMap);
	assert(!m_istreamFrameMap->fail());

	id3_tag tag(*id3_file_tag(&tagFile));

	mad_stream_init(&m_stream);
	mad_frame_init(&m_frame);
	mad_synth_init(&m_synth);
	mad_timer_reset(&m_timer);

	m_istream->seekg(0, std::ios::end);
	m_streamLength = m_istream->tellg();

	// find offset to first mp3 header
	// go to file start, see if there is a tag there. if not, and it's not eof, then, mp3 data starts there.
	m_istream->seekg(0, std::ios::beg);

	int offset = 0;
	id3_byte_t query[ID3_TAG_QUERYSIZE];
	while (true)
	{
		m_istream->read((char*)query, ID3_TAG_QUERYSIZE);
		if (m_istream->eof())
		{
			offset = 0;
			break;
		}

		int size = id3_tag_query(query, ID3_TAG_QUERYSIZE);
		if (!size)
			break;

		m_istream->seekg(size - ID3_TAG_QUERYSIZE, std::ios::cur);
		offset = (int)m_istream->tellg();
		break;
	}
	
	m_istream->seekg(offset);
	m_istreamFrameMap->seekg(offset);
	if (m_istream->fail())
		throwf("Frame map: could not seek past ID3 tag.");

	m_inputBuffer = new unsigned char[INPUT_BUFFER_SIZE+MAD_BUFFER_GUARD];

	// start building frame map
	m_frameMapBuilder = new TThread<TDelegate<MP3Decoder> >(TDelegate<MP3Decoder>(this, &MP3Decoder::buildFrameMap), false, Thread::IDLE);
}

MP3Decoder::~MP3Decoder()
{
	if (m_frameMapBuilder)
		m_frameMapBuilder->stop();

	delete m_istream;
	delete m_istreamFrameMap;
	delete[] m_inputBuffer;
	delete m_frameMapAccess;

	mad_synth_finish(&m_synth);
	mad_frame_finish(&m_frame);
	mad_stream_finish(&m_stream);
}

/* A fixed point number is formed of the following bit pattern:
*
* SWWWFFFFFFFFFFFFFFFFFFFFFFFFFFFF
* MSB                          LSB
* S ==> Sign (0 is positive, 1 is negative)
* W ==> Whole part bits
* F ==> Fractional part bits
*
* This pattern contains MAD_F_FRACBITS fractional bits, one
* should alway use this macro when working on the bits of a fixed
* point number. It is not guaranteed to be constant over the
* different platforms supported by libmad.
*/

int MP3Decoder::getPCM(float* out, int samples)
{
	if (finished())
		return 0;

	// 23 bits = 8,388,607
	// exp = -127 to 127

	int retrieved_samples = 0;

	while (samples > 0)
	{
		if (m_unprocessedSamples == 0)
		{
			nextFrame();
		}

		// if still out of decoded samples after 'nextFrame', then there's nothing else to retrieve.
		if (m_unprocessedSamples == 0)
		{
			return retrieved_samples;
		}

		int copy_samples = std::min(samples, m_unprocessedSamples);

		for (int i = 0; i < copy_samples; ++i)
		{
			*(out++) = (float)mad_f_todouble(m_synth.pcm.samples[0][m_currentSample + i]);
			*(out++) = (float)mad_f_todouble(m_synth.pcm.samples[1][m_currentSample + i]);
		}

		samples -= copy_samples;
		m_unprocessedSamples -= copy_samples;
		m_currentSample += copy_samples;
		retrieved_samples += copy_samples;
	}

	return retrieved_samples;
}

inline signed short MadFixedToSshort(mad_fixed_t Fixed)
{
	if(Fixed>=MAD_F_ONE)
		return(SHRT_MAX);
	if(Fixed<=-MAD_F_ONE)
		return(-SHRT_MAX);

	Fixed=Fixed>>(MAD_F_FRACBITS-15);
	return((signed short)Fixed);
}

int MP3Decoder::getPCM(short* out, int samples)
{
	if (finished())
		return 0;

	int retrieved_samples;

	for (retrieved_samples = 0; samples; --samples)
	{
		if (m_unprocessedSamples == 0)
		{
			nextFrame();
		}

		// if still out of decoded samples after 'nextFrame', then there's nothing else to retrieve.
		if (m_unprocessedSamples == 0)
		{
			return retrieved_samples;
		}

		*(out++) = MadFixedToSshort(m_synth.pcm.samples[0][m_currentSample]);
		*(out++) = MadFixedToSshort(m_synth.pcm.samples[1][m_currentSample]);

		--m_unprocessedSamples;
		++m_currentSample;
	}

	return retrieved_samples;
}

MP3Decoder::NEXTFRAME_RESULT MP3Decoder::nextFrame()
{
	Critical(*m_decoding);

	// taken from madlld example
	unsigned char *GuardPtr = 0;

	/* Decode the next MPEG frame. The streams is read from the
		* buffer, its constituents are break down and stored the the
		* Frame structure, ready for examination/alteration or PCM
		* synthesis. Decoding options are carried in the Frame
		* structure from the Stream structure.
		*
		* Error handling: mad_frame_decode() returns a non zero value
		* when an error occurs. The error condition can be checked in
		* the error member of the Stream structure. A mad error is
		* recoverable or fatal, the error status is checked with the
		* MAD_RECOVERABLE macro.
		*
		* {4} When a fatal error is encountered all decoding
		* activities shall be stopped, except when a MAD_ERROR_BUFLEN
		* is signaled. This condition means that the
		* mad_frame_decode() function needs more input to complete
		* its work. One shall refill the buffer and repeat the
		* mad_frame_decode() call. Some bytes may be left unused at
		* the end of the buffer if those bytes forms an incomplete
		* frame. Before refilling, the remaining bytes must be moved
		* to the beginning of the buffer and used for input for the
		* next mad_frame_decode() invocation. (See the comments
		* marked {2} earlier for more details.)
		*
		* Recoverable errors are caused by malformed bit-streams, in
		* this case one can call again mad_frame_decode() in order to
		* skip the faulty part and re-sync to the next frame.
		*/
	do
	{
		/* The input bucket must be filled if it becomes empty or if
		* it's the first execution of the loop.
		*/
		if (!m_stream.buffer || m_stream.error==MAD_ERROR_BUFLEN)
		{
			size_t			ReadSize,
							Remaining;
			unsigned char	*ReadStart;

			/* {2} libmad may not consume all bytes of the input
				* buffer. If the last frame in the buffer is not wholly
				* contained by it, then that frame's start is pointed by
				* the next_frame member of the m_stream structure. This
				* common situation occurs when mad_frame_decode() fails,
				* sets the stream error code to MAD_ERROR_BUFLEN, and
				* sets the next_frame pointer to a non NULL value. (See
				* also the comment marked {4} bellow.)
				*
				* When this occurs, the remaining unused bytes must be
				* put back at the beginning of the buffer and taken in
				* account before refilling the buffer. This means that
				* the input buffer must be large enough to hold a whole
				* frame at the highest observable bit-rate (currently 448
				* kb/s). XXX=XXX Is 2016 bytes the size of the largest
				* frame? (448000*(1152/32000))/8
				*/
			if(m_stream.next_frame!=NULL)
			{
				Remaining=m_stream.bufend-m_stream.next_frame;
				memmove(m_inputBuffer,m_stream.next_frame,Remaining);
				ReadStart=m_inputBuffer+Remaining;
				ReadSize=INPUT_BUFFER_SIZE-Remaining;
			}
			else
				ReadSize=INPUT_BUFFER_SIZE,
					ReadStart=m_inputBuffer,
					Remaining=0;

			/* Fill-in the buffer. If an error occurs print a message
				* and leave the decoding loop. If the end of stream is
				* reached we also leave the loop but the return status is
				* left untouched.
				*/
			bool isEOF = false;
			bool isBad;
			std::istream::pos_type lastPos = m_istream->tellg();
			
			m_istream->read((char*)ReadStart,ReadSize);
			
			isBad = m_istream->fail();
			
			if (isBad)
			{
				isEOF = m_istream->eof();
				m_istream->clear();
			}

			ReadSize = m_istream->tellg() - lastPos;
			if(ReadSize<=0)
			{
				if(isEOF)
				{
					dlog << "MP3Decoder: end of stream reached." << dlog.endl;
					m_finished = true;
					return FINISHED;
				}

				if(isBad)
				{
					throwf("read error on bit-stream (%s)\n", strerror(errno));
				}
			}

			/* {3} When decoding the last frame of a file, it must be
				* followed by MAD_BUFFER_GUARD zero bytes if one wants to
				* decode that last frame. When the end of file is
				* detected we append that quantity of bytes at the end of
				* the available data. Note that the buffer can't overflow
				* as the guard size was allocated but not used the the
				* buffer management code. (See also the comment marked
				* {1}.)
				*
				* In a message to the mad-dev mailing list on May 29th,
				* 2001, Rob Leslie explains the guard zone as follows:
				*
				*    "The reason for MAD_BUFFER_GUARD has to do with the
				*    way decoding is performed. In Layer III, Huffman
				*    decoding may inadvertently read a few bytes beyond
				*    the end of the buffer in the case of certain invalid
				*    input. This is not detected until after the fact. To
				*    prevent this from causing problems, and also to
				*    ensure the next frame's main_data_begin pointer is
				*    always accessible, MAD requires MAD_BUFFER_GUARD
				*    (currently 8) bytes to be present in the buffer past
				*    the end of the current frame in order to decode the
				*    frame."
				*/
			if(isEOF)
			{
				GuardPtr=ReadStart+ReadSize;
				memset(GuardPtr,0,MAD_BUFFER_GUARD);
				ReadSize+=MAD_BUFFER_GUARD;
			}

			/* Pipe the new buffer content to libmad's stream decoder
				* facility.
				*/
			mad_stream_buffer(&m_stream,m_inputBuffer,ReadSize+Remaining);
		}

		m_stream.error=MAD_ERROR_NONE;

		if (mad_frame_decode(&m_frame,&m_stream))
		{
			if(m_stream.error==MAD_ERROR_BUFLEN)
				continue;

			if (m_stream.error == MAD_ERROR_BADDATAPTR && m_seeking)
			{
				// madlib requires one frame to recover from seek operation. we should have sought the frame before our target frame.
				m_seeking = false;
				continue;
			}

			if (MAD_RECOVERABLE(m_stream.error))
			{
				/* Do not print a message if the error is a loss of
					* synchronization and this loss is due to the end of
					* stream guard bytes. (See the comments marked {3}
					* supra for more informations about guard bytes.)
					*/
				if(m_stream.error!=MAD_ERROR_LOSTSYNC ||
					m_stream.this_frame!=GuardPtr)
				{
					dlog << "MP3Decoder: recoverable frame level error (" << mad_stream_errorstr(&m_stream) << ")" << dlog.endl;

					// return silence
					if (m_stream.error!=MAD_ERROR_LOSTSYNC)
					{
						mad_frame_mute(&m_frame);
						m_stream.error = MAD_ERROR_NONE;
					}
				}
			}
			else
				throwf("unrecoverable frame level error (%s).\n", mad_stream_errorstr(&m_stream));
		}
	}
	while (m_stream.error!=MAD_ERROR_NONE);

	/* Accounting. The computed frame duration is in the frame
		* header structure. It is expressed as a fixed point number
		* whole data type is mad_timer_t. It is different from the
		* samples fixed point format and unlike it, it can't directly
		* be added or subtracted. The timer module provides several
		* functions to operate on such numbers. Be careful there, as
		* some functions of libmad's timer module receive some of
		* their mad_timer_t arguments by value!
		*/
	mad_timer_add(&m_timer,m_frame.header.duration);

	/* Once decoded the frame is synthesized to PCM samples. No errors
		* are reported by mad_synth_frame();
		*/
	mad_synth_frame(&m_synth,&m_frame);

	_sample_rate = m_synth.pcm.samplerate;
			  
	m_unprocessedSamples = m_synth.pcm.length;
	m_currentSample = 0;

	if (m_skipFrameSamplesNormalized != 0.0)
	{
		int skipSamples = (int)((double)m_unprocessedSamples * m_skipFrameSamplesNormalized);
		m_unprocessedSamples -= skipSamples;
		m_currentSample += skipSamples;
		m_skipFrameSamplesNormalized = 0.0;
	}
	return OK;
}

double MP3Decoder::progress() const
{
	return mad_timer_count(m_timer, MAD_UNITS_MILLISECONDS)/* - m_unprocessedSamples / 44100000.0*/;
}

void MP3Decoder::buildFrameMap()
{
	mad_stream stream;
	mad_frame frame;
	mad_timer_t timer;
	unsigned char* inputBuffer = new unsigned char[INPUT_BUFFER_SIZE];
	unsigned char *GuardPtr = 0;
	std::istream::pos_type lastProgressMessage;
	std::istream::pos_type filePosAtInputBuffer;

	mad_stream_init(&stream);
	mad_frame_init(&frame);
	mad_timer_reset(&timer);

	try
	{
		do
		{
			bool recoverableError = false;

			if (!stream.buffer || stream.error==MAD_ERROR_BUFLEN)
			{
				size_t			ReadSize,
								Remaining;
				unsigned char	*ReadStart;

				if(stream.next_frame!=NULL)
				{
					Remaining=stream.bufend-stream.next_frame;
					memmove(inputBuffer,stream.next_frame,Remaining);
					ReadStart=inputBuffer+Remaining;
					ReadSize=INPUT_BUFFER_SIZE-Remaining;
				}
				else
					ReadSize=INPUT_BUFFER_SIZE,
						ReadStart=inputBuffer,
						Remaining=0;

				bool isEOF = false;
				bool isBad;
				std::istream::pos_type lastPos = m_istreamFrameMap->tellg();
				
				m_istreamFrameMap->read((char*)ReadStart,ReadSize);
				
				filePosAtInputBuffer = std::streamoff(lastPos) - (ReadStart - inputBuffer);

				isBad = m_istreamFrameMap->fail();
				
				if (isBad)
				{
					isEOF = m_istreamFrameMap->eof();
					m_istreamFrameMap->clear();
				}

				ReadSize = m_istreamFrameMap->tellg() - lastPos;
				if(ReadSize<=0)
				{
					if(isEOF)
					{
						dlog << "MP3Decoder frame map: end of stream reached." << dlog.endl;
						break;
					}

					if(isBad)
					{
						throwf("read error on bit-stream (%s)\n", strerror(errno));
					}
				}

				if(isEOF)
				{
					GuardPtr=ReadStart+ReadSize;
					memset(GuardPtr,0,MAD_BUFFER_GUARD);
					ReadSize+=MAD_BUFFER_GUARD;
				}

				/* Pipe the new buffer content to libmad's stream decoder
					* facility.
					*/
				mad_stream_buffer(&stream,inputBuffer,ReadSize+Remaining);
			}

			stream.error=MAD_ERROR_NONE;

			if (mad_header_decode(&frame.header,&stream))
			{
				if(stream.error==MAD_ERROR_BUFLEN)
					continue;

				if (MAD_RECOVERABLE(stream.error))
				{
					if(stream.error!=MAD_ERROR_LOSTSYNC ||
						stream.this_frame!=GuardPtr)
					{
						recoverableError = true;
						dlog << "MP3Decoder frame map: recoverable frame level error (" << mad_stream_errorstr(&stream) << ")" << dlog.endl;
					}
				}
				else
					throwf("MP3Decoder frame map: unrecoverable frame level error (%s).\n", mad_stream_errorstr(&stream));
			}

			// if we had an error on this frame, then there will be no valid mapping between the file position and a time
			if (!recoverableError)
			{
				Critical(*m_frameMapAccess);
				m_frameMapProgress = mad_timer_count(timer, MAD_UNITS_MILLISECONDS);

				unsigned int filePos = std::streamoff(filePosAtInputBuffer) + std::streamoff(stream.this_frame - stream.buffer);

				assert(m_framePositions.find((unsigned int)m_frameMapProgress) == m_framePositions.end());
				m_framePositions[(unsigned int)m_frameMapProgress] = filePos;
				if (!m_frameMapProgress || std::streamoff(filePosAtInputBuffer - lastProgressMessage) > (m_streamLength / 3.0f))
				{
					lastProgressMessage = filePosAtInputBuffer;
					dlog << "MP3Decoder frame map: " << (int)(((float)filePos / m_streamLength) * 100.0f) << "% frame at " << (float)m_frameMapProgress << "ms, offset " << std::streamoff(m_framePositions[(int)m_frameMapProgress]) << dlog.endl;
				}
			}

			mad_timer_add(&timer,frame.header.duration);
		}
		while (true);
	}
	catch (...)
	{
		delete[] inputBuffer;

		mad_synth_finish(&synth);
		mad_frame_finish(&frame);
		mad_stream_finish(&stream);
		throw;
	}

	delete[] inputBuffer;

	mad_synth_finish(&synth);
	mad_frame_finish(&frame);
	mad_stream_finish(&stream);

	m_streamLengthMS = m_frameMapProgress;
	dlog << "MP3Decoder frame map: stream duration is " << durationtostr((int)m_streamLengthMS) << dlog.endl;

	m_frameMapBuilt = true;
	m_frameMapBuilder = 0;
}

void MP3Decoder::seek(double ms)
{
	bool warned = false;

	// wait for frame map to build to our desired seek point, if necessary
	do
	{
		bool ready = false;

		{
			Critical(*m_frameMapAccess);
			ready = m_frameMapBuilt || (!m_framePositions.empty() && m_frameMapProgress >= ms);
		}

		if (ready)
		{
			if (warned)
			{
				dlog << "Frame map ready with entry for " << (float)ms << " ms." << dlog.endl;
			}

			break;
		}
		else
		{
			// tbd: use timed log?
			if (!warned)
			{
				warned = true;
				dlog << "Waiting for frame map to build past " << (float)ms << " ms." << dlog.endl;
			}

			const int sleepMs = 10;
#if IS_WINDOWS // fix
			Sleep(sleepMs);
#else
			usleep(sleepMs * 1000);
#endif
		}
	}
	while (true);

	assert(!m_framePositions.empty());

	Critical(*m_decoding);

	m_unprocessedSamples = 0;
	m_currentSample = 0;

	FramePositions::iterator i = m_framePositions.upper_bound((unsigned int)ms);

	double max = i->first;

	if (i != m_framePositions.begin())
		--i;

	double min = i->first;

	assert(ms >= min && ms <= max);

	// we can only seek on frame boundaries -- calculate the normalized distance of our target time between the 
	// time of the minimum and maximum frame boundaries
	if (ms == min)
		m_skipFrameSamplesNormalized = 0;
	else
		m_skipFrameSamplesNormalized = Mapping::linear(ms, min, max, 0.0, 1.0);

	mad_frame_finish(&m_frame);
	mad_stream_finish(&m_stream);
	mad_frame_init(&m_frame);
	mad_stream_init(&m_stream);
	mad_timer_set(&m_timer, (unsigned long)(ms / 1000.0), (unsigned long)fmod(ms, 1000.0), (unsigned long)1000.0);

	// seek to frame before desired frame, if possible -- madlib requires one frame to recover from seek operation
	if (i != m_framePositions.begin())
		--i;

	m_istream->seekg(i->second);

	m_seeking = true;
	m_finished = false;
}

void MP3Decoder::waitForFrameMap() const 
{ 
	if (m_frameMapBuilder)
	{
		m_frameMapBuilder->setPriority(ThreadBase::ABOVE_NORMAL);
		
		while (m_frameMapBuilder)
		{
			const int sleepMs = 100;
#if IS_WINDOWS // fix
			Sleep(sleepMs);
#else
			usleep(sleepMs * 1000);
#endif
		}
	}
}

int MP3Decoder::rate() const
{
  return _sample_rate;
}
