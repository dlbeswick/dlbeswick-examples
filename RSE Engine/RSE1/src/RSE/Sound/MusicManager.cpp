// ------------------------------------------------------------------------------------------------
//
// MusicManager
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "MusicManager.h"
#include "AppBase.h"
#include "Sound/IMusic.h"
#include "Sound/ISoundProvider.h"
#include <Standard/Config.h>
#include <Standard/CriticalSection.h>
#include <Standard/Interpolation.h>
#include <Standard/Rand.h>
#include <Standard/Thread.h>

IMPLEMENT_RTTI(Music);

void Music::streamVars(StreamVars& v)
{
	STREAMVAR(volume);
}

////

void MusicManager::StateDefault::update(float delta)
{
	if (o->m_playlistPlaying && o->m_playlistProgress < (int)o->m_currentPlaylist.size())
	{
		if ((!o->m_currentSong || !o->m_currentSong->playing()) || o->m_forceNextPlaylist)
		{
			o->m_forceNextPlaylist = false;
			o->doPlay(PathResource("media\\music\\" + o->m_currentPlaylist[o->m_playlistProgress]));
			++o->m_playlistProgress;
		}
	}
}

////

static CriticalSection SongLoad;

void MusicManager::StateTransition::start()
{
	if (curThread)
		curThread->stop();

	loading = !nextSongPath.empty();
	if (loading)
	{
		curThread = new Thread(delegate(&MusicManager::StateTransition::loadSong));
	}

	if (o->m_currentSong)
		startVolume = o->m_currentSong->volume();
	else
		startVolume = 0;
	
	startTime = FLT_MAX;
}

void MusicManager::StateTransition::loadSong()
{
	PtrGC<IMusic> song;

	try
	{
		song = AppBase().sound().music(nextSongPath);
	}
	catch (Exception& e)
	{
		derr << "Music manager had trouble loading a song: " << e << dlog.endl;
		songSuccess = false;
		return;
	}

	songSuccess = true;
	nextSong = song;
	loading = false;
}

////

void MusicManager::StateFadeOut::update(float delta)
{
	if (loading)
	{
		if (!songSuccess)
			o->states.go(o->Default);
		return;
	}

	if (startTime == FLT_MAX)
		startTime = AppBase().timer().time();

	float elapsed = (float)(AppBase().timer().time() - startTime);

	if (o->m_currentSong)
	{
		o->m_currentSong->setVolume(Interpolation::linear(startVolume, 0.0f, elapsed / time));
	}
	
	if (!o->m_currentSong || elapsed >= time)
	{
		o->m_currentSong.destroy();
		o->m_currentSong = nextSong;
		if (o->m_currentSong)
			o->m_currentSong->play();
		nextSong = 0;
		o->states.go(o->Default);
	}
}

////

IMPLEMENT_RTTI(MusicManager);

MusicManager::MusicManager() :
	m_currentPlaylist(0),
	m_shuffle(true),
	m_playlistProgress(0),
	m_forceNextPlaylist(false),
	m_playlistPlaying(false)
{
	// read music
	Config& cfgMusic = AppBase().config("music.ini");
	const Config::Categories& names = cfgMusic.categories();
	for (Config::Categories::const_iterator i = names.begin(); i != names.end(); ++i)
	{
		Music& music = m_music.insert(MusicMap::value_type(i->category->name, Music())).first->second;
		music.setConfig(cfgMusic, i->category->name);
		music.streamFromConfig();
	}

	// read playlists
	setConfig(AppBase().config("musicmanager.ini"));
	get("playlists", m_playlists);

	BEGIN_STATEMACHINE;
	ADD_STATE(Default);
	ADD_STATE(FadeOut);

	states.go(Default);
}

void MusicManager::clearPlaylist()
{
	m_currentPlaylist.clear();
}

void MusicManager::loadPlaylist(const std::string& playlist, bool shuffle)
{
	if (m_currentPlaylistName == playlist && m_currentSong)
		return;

	setShuffle(shuffle);

	Playlists::iterator i = m_playlists.find(playlist);
	if (i == m_playlists.end())
		throwf("No playlist called " + playlist);
	
	m_currentPlaylistName = playlist;
	m_currentPlaylist = i->second;
	srand(GetTickCount());
	if (m_shuffle)
		std::random_shuffle(m_currentPlaylist.begin(), m_currentPlaylist.end());
	m_playlistProgress = 0;
	m_playlistPlaying = false;
	m_forceNextPlaylist = true;
}

void MusicManager::startPlaylist()
{
	m_playlistPlaying = true;
}

void MusicManager::setShuffle(bool b)
{
	m_shuffle = b;
}

void MusicManager::update(float delta)
{
	Super::update(delta);

	states.update(delta);
}

void MusicManager::doPlay(const PathResource& fileName)
{
	FadeOut->nextSongPath = fileName;
	states.go(FadeOut, true);
}

void MusicManager::play(const PathResource& fileName)
{
	m_currentPlaylistName.clear();
	doPlay(fileName);
}

void MusicManager::fade(float time)
{
	FadeOut->nextSongPath = PathResource();
	FadeOut->time = time;
	states.go(FadeOut, true);
}

void MusicManager::stop()
{
	if (m_currentSong)
	{
		m_currentSong->stop();
	}
}
