#ifndef _DSOUND_PORTAUDIOLIBRARY_H
#define _DSOUND_PORTAUDIOLIBRARY_H

#include "dsound/common.h"
#include "dsound/SoundLibrary.h"

class PortAudioDevice;

class PortAudioLibrary : public SoundLibrary
{
public:
	PortAudioLibrary(const std::string& client_name);

	virtual void close();
	PortAudioDevice& defaultDevice();
	std::vector<PortAudioDevice*> devices() const;
	virtual void open();
	virtual void refreshDevices();

	virtual void setDesiredLatency(float latency);
	virtual float desiredLatency() const;

private:
	std::string _client_name;
	std::vector<PortAudioDevice*> _devices;
	bool _open;
	float _desiredLatency;
};

#endif
