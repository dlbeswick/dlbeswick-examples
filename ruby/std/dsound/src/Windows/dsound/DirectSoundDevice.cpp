#include "dsound/pch.h"
#include "DirectSoundDevice.h"
#include "DirectSoundBuffer.h"

DirectSoundDevice::DirectSoundDevice(SoundLibrary& library, GUID guid, const std::string& description, float latency) :
	SoundDevice(library),
	_ds(0),
	_guid(guid),
	_description(description),
	_latency(latency),
	_buffer(0),
	_hasWindow(false),
	_shouldBePlaying(false)
{
}

DirectSoundDevice::~DirectSoundDevice()
{
	close();
}

void DirectSoundDevice::close()
{
	delete _buffer;
	_buffer = 0;

	if (_ds)
		_ds->Release();

	_ds = 0;
}

bool DirectSoundDevice::isDefault() const
{
	GUID guid;

	GetDeviceID(&DSDEVID_DefaultPlayback, &guid);

	return (_guid == guid) != 0;
}

void DirectSoundDevice::open()
{
	if (opened())
		return;

	if (DirectSoundCreate8(&_guid, &_ds, 0) != DS_OK)
		throwf("Couldn't create device for '%s'", description().c_str());

	_buffer = new DirectSoundBuffer(*this, _latency);
}

void DirectSoundDevice::setWindow(int window)
{
	if (_ds->SetCooperativeLevel((HWND)window, DSSCL_PRIORITY) != DS_OK)
		throwf("Couldn't set priority level for device '%s'", description().c_str());

	_hasWindow = true;

	if (_shouldBePlaying && !playing())
		play();
}

void DirectSoundDevice::play()
{
	_shouldBePlaying = true;

	if (_hasWindow)
	{
		if (!_buffer)
			throwf("Device is not open.");

		_buffer->play();
	}
}

bool DirectSoundDevice::playing() const
{
	if (!_buffer)
		throwf("Device is not open.");

	return _buffer->playing();
}

void DirectSoundDevice::stop()
{
	_shouldBePlaying = false;

	if (!_buffer)
		throwf("Device is not open.");

	_buffer->stop();
}

void DirectSoundDevice::setPlayCursor(int samples)
{
	if (!_buffer)
		throwf("Device is not open.");

	_buffer->setPlayCursor(samples);
}

int DirectSoundDevice::playCursor() const
{
	if (!_buffer)
		throwf("Device is not open.");

	return _buffer->playCursor();
}

int DirectSoundDevice::writeCursor() const
{
	if (!_buffer)
		throwf("Device is not open.");

	return _buffer->writeCursor();
}

void DirectSoundDevice::addInput(SoundGeneratorOutput& output)
{
	SoundDevice::addInput(output);

	if (!_buffer)
		throwf("Device is not open.");

	_buffer->addInput(output);
}

bool DirectSoundDevice::opened() const
{
	return _ds != 0;
}

void DirectSoundDevice::restart()
{
	if (_buffer)
		_buffer->restart();
}

int DirectSoundDevice::playCursorMaxSamples() const
{
	return _buffer->m_totalSamples;
}
