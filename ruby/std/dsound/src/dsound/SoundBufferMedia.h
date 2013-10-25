#include "SoundBuffer.h"

class SoundBufferMedia : public SoundBuffer
{
public:
	SoundBufferMedia(class SoundLibrary& library, float seconds = 0.5f);
	~SoundBufferMedia();

	virtual void open(const char* filePath);

	virtual double progress() const;

	virtual void setProgress(double ms);

	virtual double lengthMS() const;
	virtual void waitForLengthCalculation() const;

	// Sample rate of loaded file.
	virtual int rate() const;

	virtual bool opening() const { return m_opening; }
	virtual bool opened() const { return !opening() && m_decoder; }

protected:
	void doOpen();

	virtual bool fillSelf(short* buf, int samples);
	virtual bool fillSelf(float* buf, int samples);

	CriticalSection m_decoderAccess;
	class DSoundDecoder* m_decoder;
	std::string m_openPath;
	bool m_opening;
	bool _is_mp3decoder;
};
