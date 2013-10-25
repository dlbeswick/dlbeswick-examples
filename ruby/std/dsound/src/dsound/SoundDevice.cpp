#include "dsound/pch.h"
#include "SoundDevice.h"
#include "SoundGeneratorOutput.h"

void SoundDevice::addInput(SoundGeneratorOutput& output)
{
	bool needsOpen = inputs().empty();

	SoundConsumer::addInput(output);

	if (needsOpen)
	{
		open();

		if (!playing())
			play();
	}
}

void SoundDevice::removeInput(SoundGeneratorOutput& output)
{
	SoundConsumer::removeInput(output);

	if (inputs().empty())
	{
		if (playing())
			stop();

		close();
	}
}

void SoundDevice::clearInputs()
{
	SoundConsumer::clearInputs();

	if (playing())
		stop();
}

void SoundDevice::syncRequestAdd(
		SoundGeneratorOutput& source, 
		SoundGeneratorOutput& dest, 
		SoundDevice& syncee)
{
#if 0
	SyncRequest request;
	request.source = &source;
	request.dest = &dest;
	request.device = &syncee;

	Critical(_syncRequestsLock);
	_syncRequests.push_back(request);
#endif
}

void SoundDevice::handleSyncRequests(float syncAfterSeconds)
{
#if 0
	std::set<SoundDevice*> devices;

	{
		Critical(_syncRequestsLock);

		assert(syncAfterSeconds >= 0);

		if (_syncRequests.empty())
			return;

		for (uint i = 0; i < _syncRequests.size(); ++i)
		{
			if (_syncRequests[i].device->playing())
				_syncRequests[i].device->stop();

			devices.insert(_syncRequests[i].device);

			if (_syncRequests[i].source->valid())
				_syncRequests[i].dest->syncTo(*_syncRequests[i].source);
		}

		_syncRequests.clear();
	}
	
	for (std::set<SoundDevice*>::iterator i = devices.begin(); i != devices.end(); ++i)
	{
		SoundDevice& device = **i;

		device.restart();

		int syncAfterSamples = syncAfterSeconds * device.samplesPerSec();

		assert(syncAfterSamples < device.playCursorMaxSamples());

		device.setPlayCursor(device.playCursorMaxSamples() - syncAfterSamples);
		device.play();
	}
#endif
}
