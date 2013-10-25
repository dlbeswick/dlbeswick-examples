#include "dsound/SoundDevice.h"

class CoreAudioDevice : public SoundDevice
{
public:
	CoreAudioDevice(AudioDeviceID id, int stream, const std::string& description);
	~CoreAudioDevice();

	bool isDefault() const;
	bool opened() const;
	void open();
	void close();
	const std::string& description() const { return m_description; }

	void play();
	void stop();

	virtual bool playing() const { return m_playing; }
	virtual void setPlayCursor(int bytes) {}
	virtual int playCursor() const { return 0; }
	virtual int writeCursor() const { return 0; }
	virtual float samplesPerSec() const { return 44100.0; }

protected:
	static OSStatus	renderer(AudioDeviceID inDevice,
							 const AudioTimeStamp* inNow,
							 const AudioBufferList* inInputData,
							 const AudioTimeStamp* inInputTime,
							 AudioBufferList* outOutputData,
							 const AudioTimeStamp* inOutputTime,
							 void* inClientData);

	static OSStatus converterInput(
		AudioConverterRef inAudioConverter,
		UInt32* ioNumberDataPackets,
		AudioBufferList* ioData,
		AudioStreamPacketDescription** outDataPacketDescription,
		void* inUserData
	);

	bool m_playing;
	bool m_open;
	AudioConverterRef _audioConverter;
	AudioStreamBasicDescription _audioDesc;
	AudioStreamBasicDescription _inputDesc;
	float* _consumeBuffer;
	float* _convertBuffer;
	int _consumeBufferSizeSamples;
	int _convertBufferAvailableSamples;
	std::string m_description;
	AudioDeviceID _id;
	AudioDeviceIOProcID _ioProc;
	int _stream;
	CriticalSection _critical;
    AsyncEvent _closeFinished;
};
