#include "dsound/pch.h"
#include "CoreAudioDevice.h"
#include "AudioToolbox/AudioToolbox.h"

CoreAudioDevice::CoreAudioDevice(AudioDeviceID id, int stream, const std::string& description) :
	_id(id),
	m_description(description),
	m_open(false),
	m_playing(false),
	_audioConverter(0),
	_convertBuffer(0),
	_ioProc(0),
	_stream(stream),
	_consumeBufferSizeSamples(0),
	_convertBufferAvailableSamples(0),
	_consumeBuffer(0)
{
}

CoreAudioDevice::~CoreAudioDevice()
{
	close();
}

OSStatus CoreAudioDevice::converterInput(
	AudioConverterRef inAudioConverter,
	UInt32* ioNumberDataPackets,
	AudioBufferList* ioData,
	AudioStreamPacketDescription** outDataPacketDescription,
	void* inUserData)
{
	CoreAudioDevice* self = (CoreAudioDevice*)inUserData;

	assert(self);
    assert(*ioNumberDataPackets > 0);

    int requestedOutputSamples = *ioNumberDataPackets;
    while (requestedOutputSamples > 0)
    {
        int workSamples;

        if (self->_convertBufferAvailableSamples == 0)
        {
            workSamples = std::min(requestedOutputSamples, self->_consumeBufferSizeSamples);

            if (!self->consumeTo(self->_consumeBuffer, workSamples))
            {
                memset(self->_consumeBuffer, 0, sizeof(float) * workSamples * 2);
            }

            self->_convertBuffer = self->_consumeBuffer;
            self->_convertBufferAvailableSamples = workSamples;
        }
        else
        {
            workSamples = std::min(requestedOutputSamples, self->_convertBufferAvailableSamples);
        }

        requestedOutputSamples -= workSamples;
    }

	ioData->mBuffers[0].mData = self->_convertBuffer;
	ioData->mBuffers[0].mDataByteSize = self->_inputDesc.mBytesPerPacket * *ioNumberDataPackets;
	ioData->mNumberBuffers = 1;

	self->_convertBuffer += *ioNumberDataPackets * 2;
	self->_convertBufferAvailableSamples -= *ioNumberDataPackets;

    assert(self->_convertBufferAvailableSamples >= 0);

	return noErr;
}

OSStatus CoreAudioDevice::renderer(AudioDeviceID inDevice,
								   const AudioTimeStamp* inNow,
								   const AudioBufferList* inInputData,
								   const AudioTimeStamp* inInputTime,
								   AudioBufferList* outOutputData,
								   const AudioTimeStamp* inOutputTime,
								   void* inClientData)
{
	CoreAudioDevice* self = (CoreAudioDevice*)inClientData;
    Critical(self->_critical);

	AudioBuffer& buffer = outOutputData->mBuffers[self->_stream];

    if (!self->m_open)
    {
        memset(buffer.mData, 0, buffer.mDataByteSize);
        osVerify(AudioDeviceStop(self->_id, self->_ioProc));
        self->_closeFinished.signal();
        return noErr;
    }

	if (!self->m_playing)
	{
		memset(buffer.mData, 0, buffer.mDataByteSize);
	}
	else
	{
		int requestedOutputSamples = buffer.mDataByteSize / self->_audioDesc.mBytesPerPacket;

        UInt32 packets = requestedOutputSamples;
        osVerify(AudioConverterFillComplexBuffer(self->_audioConverter, &CoreAudioDevice::converterInput, (void*)self, &packets, outOutputData, 0));

        for (int i = 0; i < outOutputData->mNumberBuffers; ++i)
        {
            char *buf = (char*)outOutputData->mBuffers[i].mData;
            outOutputData->mBuffers[i].mData = (void*)(buf + packets * self->_audioDesc.mBytesPerPacket);
        }
	}

	return noErr;
}

bool CoreAudioDevice::isDefault() const
{
	return true;
}

bool CoreAudioDevice::opened() const
{
	return m_open;
}

void CoreAudioDevice::open()
{
	if (opened())
		throwf("Device %s is already open.", description().c_str());

	m_open = true;

	UInt32 size = 0;
	osVerify(AudioDeviceGetPropertyInfo(_id, 0, false, kAudioDevicePropertyStreams, &size, 0));
	int numStreams = size / sizeof(AudioStreamID);
	AudioStreamID* streams = new AudioStreamID[numStreams];
	osVerify(AudioDeviceGetProperty(_id, 0, false, kAudioDevicePropertyStreams, &size, (void*)streams));

	if (_stream >= numStreams)
		throwf("Device %s: stream number %d requested, but only %d streams are available.", description().c_str(), _stream, numStreams);

	size = sizeof(_audioDesc);
	osVerify(AudioStreamGetProperty(streams[_stream], 0, kAudioStreamPropertyVirtualFormat, &size, &_audioDesc));

	delete streams;

	_inputDesc.mSampleRate = 44100;
	_inputDesc.mBitsPerChannel = 32;
	_inputDesc.mReserved = 0;
	_inputDesc.mFormatID = kAudioFormatLinearPCM;
	_inputDesc.mFormatFlags = kLinearPCMFormatFlagIsFloat | kLinearPCMFormatFlagIsPacked;
	_inputDesc.mChannelsPerFrame = 2;
	_inputDesc.mFramesPerPacket = 1;
	_inputDesc.mBytesPerFrame = ((_inputDesc.mBitsPerChannel * _inputDesc.mChannelsPerFrame) / 8);
	_inputDesc.mBytesPerPacket = _inputDesc.mFramesPerPacket * _inputDesc.mBytesPerFrame;

	osVerify(AudioConverterNew(&_inputDesc, &_audioDesc, &_audioConverter));

	_consumeBufferSizeSamples = _inputDesc.mSampleRate;
    _convertBufferAvailableSamples = 0;

	_consumeBuffer = new float[_inputDesc.mChannelsPerFrame * _consumeBufferSizeSamples];

	osVerify(AudioDeviceCreateIOProcID(_id, &CoreAudioDevice::renderer, (void*)this, &_ioProc));
	osVerify(AudioDeviceStart(_id, _ioProc));
}

void CoreAudioDevice::close()
{
	{
		Critical(_critical);
		if (!m_open)
			return;

        _closeFinished.reset();
		m_open = false;
	}

    _closeFinished.wait();

	osVerify(AudioDeviceDestroyIOProcID(_id, _ioProc));

	_ioProc = 0;

	AudioConverterDispose(_audioConverter);

	delete[] _consumeBuffer;
}

void CoreAudioDevice::stop()
{
	m_playing = false;
}

void CoreAudioDevice::play()
{
	m_playing = true;
}
