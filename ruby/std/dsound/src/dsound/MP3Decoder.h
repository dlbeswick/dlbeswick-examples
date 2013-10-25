#ifndef DSOUND_MP3DECODER_H
#define DSOUND_MP3DECODER_H

#include <istream>
#include "dsound/Decoder.h"

class ExceptionMP3Decoder : public ExceptionDecoder
{
public:
	ExceptionMP3Decoder(const std::string& s) : ExceptionDecoder(s) {}
};

class MP3Decoder : public DSoundDecoder
{
public:
	MP3Decoder(class SoundLibrary& library, std::istream* decodeStream, std::istream* frameMapStream, id3_file& tagFile);
	~MP3Decoder();

	int getPCM(float* out, int samples);
	int getPCM(short* out, int samples);

	bool finished() const { return m_finished; }

	double progress() const;
	double lengthMS() const { return m_streamLengthMS; }
	int unprocessedSamples() { return m_unprocessedSamples; }

	void seek(double ms);

	void waitForFrameMap() const;

	virtual int rate() const;
	  
protected:
	enum NEXTFRAME_RESULT
	{
		OK = 0,
		FINISHED
	};

	NEXTFRAME_RESULT nextFrame();
	void buildFrameMap();

	std::istream* m_istream;
	std::istream* m_istreamFrameMap;
	int m_streamLength;
	double m_streamLengthMS;
	double m_skipFrameSamplesNormalized;
	int m_unprocessedSamples;
	int m_currentSample;
	bool m_finished;
	bool m_frameMapBuilt;
	bool m_seeking;
	unsigned char* m_inputBuffer;
	int _sample_rate;

	typedef std::map<unsigned int, std::istream::pos_type> FramePositions;
	FramePositions m_framePositions;
	double m_frameMapProgress;
	class CriticalSection* m_frameMapAccess;
	class CriticalSection* m_decoding;
	PtrGC<TThread<TDelegate<class MP3Decoder> > > m_frameMapBuilder;

	struct mad_stream m_stream;
	struct mad_frame m_frame;
	struct mad_synth m_synth;
	mad_timer_t m_timer;
};

#endif
