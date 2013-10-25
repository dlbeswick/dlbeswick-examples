// ------------------------------------------------------------------------------------------------
//
// FMODProvider
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "FMODProvider.h"
#include "AppBase.h"
#include "FMODMusic.h"
#include "FMODSample.h"
#include "PathResource.h"
#include "Sound/SoundListener.h"
#include <Standard/Config.h>
#include <Standard/FileMgr.h>
#include <Standard/Log.h>
#include <Standard/Exception/Filesystem.h>

REGISTER_RTTI(FMODProvider);

FMODProvider::FMODProvider()
{
	setConfig(AppBase().options(), "Sound");
    
	if (!FSOUND_Init(get("SampleRate", 44100), get("Channels", 32), 0))
	{
		throwf("FMODProvider: init error: " + error());
	}

	// set up FMOD reference coordinate frame (same as identity)
	m_referenceFrame.identity();
	m_listenerToReference = m_referenceFrame;
}

void FMODProvider::construct()
{
	Super::construct();

	// sound volume
	setSoundVolume(get("SoundVolume", 1.0f));
	setMusicVolume(get("MusicVolume", 1.0f));
	setGlobalVolume(get("GlobalVolume", 1.0f));

	dlog << "FMODProvider: initialised, sound system ready." << dlog.endl;
}

FMODProvider::~FMODProvider()
{
	// free samples
	int count;
	count = 0;
	for (Samples::iterator i = m_samples.begin(); i != m_samples.end(); ++i, ++count)
	{
		delete i->second;
	}
	dlog << "FMODProvider: freed " << count << " samples." << dlog.endl;

	count = 0;
	for (Music::iterator i = m_music.begin(); i != m_music.end(); ++i, ++count)
	{
		i->destroy();
	}
	dlog << "FMODProvider: freed " << count << " music items." << dlog.endl;

	FSOUND_Close();
	dlog << "FMODProvider: closed." << dlog.endl;
}

SmartPtr<ibinstream> FMODProvider::soundStream(const PathResource& path)
{
	try
	{
		return new ibinstream(PathResource(*path + ".wav").open(std::ios::in | std::ios::binary));
	}
	// .ogg
	// .mp3, etc
	catch (ExceptionFilesystem&)
	{
		EXCEPTIONSTREAM(ExceptionFilesystem(path), "Couldn't open sound as .wav");
	}

	// this will never execute
	return 0;
}

SmartPtr<ibinstream> FMODProvider::musicStream(const PathResource& path)
{
	static const char* musicExtensions[] = {
		".it",
		".xm",
		".mod",
		0
	};

	int i = 0;

	while (musicExtensions[i])
	{
		try
		{
			return new ibinstream(PathResource(*path + musicExtensions[i]).open(std::ios::in | std::ios::binary));
		}
		catch (Exception& e)
		{
			++i;
			if (!musicExtensions[i])
				throwf("Couldn't open music: " + std::string(e.what()));
		}
	}

	// this will never execute
	return 0;
}

ISample& FMODProvider::sample(const PathResource& path)
{
	if (path.empty())
		throwf("FMODProvider::sample: empty path given");

	// path is used as an identifier to find the sound once it has been cached
	// make it lower so the case differences committed by callers will not matter
	// fix: change map to use lower-case hash function
	Samples::iterator i = m_samples.find(*path);
	if (i != m_samples.end())
		return *i->second;

	ISample* sample = new FMODSample(*soundStream(*path));
	m_samples.insert(std::pair<std::string, ISample*>(*path, sample));
	return *sample;
}

PtrGC<IMusic> FMODProvider::music(const PathResource& path)
{
	if (path.empty())
		throwf("FMODProvider::music: empty path given");

	PtrGC<IMusic> music = new FMODMusic(*musicStream(*path), *this);
	m_music.add(music);
	
	return music;
}

std::string FMODProvider::error(int e)
{
	if (e == INT_MAX)
		e = FSOUND_GetError();

	switch (e)
	{
	case FMOD_ERR_NONE: return "No errors";
	case FMOD_ERR_BUSY: return "Cannot call this command after FSOUND_Init.  Call FSOUND_Close first.";
	case FMOD_ERR_UNINITIALIZED: return "This command failed because FSOUND_Init or FSOUND_SetOutput was not called";
	case FMOD_ERR_INIT: return "Error initializing output device.";
	case FMOD_ERR_ALLOCATED: return "Error initializing output device, but more specifically, the output device is already in use and cannot be reused.";
	case FMOD_ERR_PLAY: return "Playing the sound failed.";
	case FMOD_ERR_OUTPUT_FORMAT: return "Soundcard does not support the features needed for this soundsystem (16bit stereo output)";
	case FMOD_ERR_COOPERATIVELEVEL: return "Error setting cooperative level for hardware.";
	case FMOD_ERR_CREATEBUFFER: return "Error creating hardware sound buffer.";
	case FMOD_ERR_FILE_NOTFOUND: return "File not found";
	case FMOD_ERR_FILE_FORMAT: return "Unknown file format";
	case FMOD_ERR_FILE_BAD: return "Error loading file";
	case FMOD_ERR_MEMORY: return "Not enough memory or resources";
	case FMOD_ERR_VERSION: return "The version number of this file format is not supported";
	case FMOD_ERR_INVALID_PARAM: return "An invalid parameter was passed to this function";
	case FMOD_ERR_NO_EAX: return "Tried to use an EAX command on a non EAX enabled channel or output.";
	case FMOD_ERR_CHANNEL_ALLOC: return "Failed to allocate a new channel";
	case FMOD_ERR_RECORD: return "Recording is not supported on this machine";
	case FMOD_ERR_MEDIAPLAYER: return "Windows Media Player not installed so cannot play wma or use internet streaming.";
	case FMOD_ERR_CDDEVICE: return "error occured trying to open the specified CD device";
	default: return "unknown error " + e;
	}
}

void FMODProvider::update(float delta)
{
	Super::update(delta);

	if (m_listener)
	{
		Quat rot = m_listener->listenerRotation();
		Point3 forward = Point3(0, 0, 1) * rot;
		Point3 top = Point3(0, 1, 0) * rot;
		Point3 origin = m_listener->listenerOrigin() * m_listenerToReference;
		Point3 velocity = m_listener->listenerVelocity() * m_listenerToReference;

		FSOUND_3D_Listener_SetAttributes(
			&origin.x,
			&velocity.x,
			forward.x,
			forward.y,
			forward.z,
			top.x,
			top.y,
			top.z
		);
	}
	else
	{
		FSOUND_3D_Listener_SetAttributes(
			&Point3::ZERO.x,
			&Point3::ZERO.x,
			1,
			0,
			0,
			0,
			1,
			0
		);
	}
}

void FMODProvider::setListener(const SoundListener* listener)
{
	m_listener = listener;
	if (m_listener)
		m_listenerToReference = m_referenceFrame * m_listener->listenerReferenceFrame();
}

void FMODProvider::onListenerDestroyed(const SoundListener& listener)
{
	if (&listener == m_listener)
		setListener(0);
}

void FMODProvider::setGlobalVolume(float v)
{
	m_globalVolume = v;
	setSoundVolume(soundVolume());
	setMusicVolume(musicVolume());
}

void FMODProvider::setSoundVolume(float v)
{
	m_soundVolume = v;
	FSOUND_SetSFXMasterVolume((int)(255.0f * m_soundVolume * m_globalVolume));
}

void FMODProvider::setMusicVolume(float v)
{
	m_musicVolume = v;

	for (Music::iterator i = m_music.begin(); i != m_music.end(); ++i)
		(*i)->setVolume((*i)->volume());
}

float FMODProvider::globalVolume() const
{
	return m_globalVolume;
}

float FMODProvider::soundVolume() const
{
	return m_soundVolume;
}

float FMODProvider::musicVolume() const
{
	return m_musicVolume;
}
