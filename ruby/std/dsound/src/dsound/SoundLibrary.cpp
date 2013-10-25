#include "dsound/SoundLibrary.h"

SoundLibrary::SoundLibrary(const std::string& client_name)
#if WITH_SOX
	: _sox_inited(false)
#endif
{
}

SoundLibrary::~SoundLibrary()
{
	if (_sox_inited)
		sox_quit();
}

#if WITH_SOX
void SoundLibrary::ensureSox()
{
	if (!_sox_inited)
	{
		sox_init();
		_sox_inited = true;
	}
}
#endif
