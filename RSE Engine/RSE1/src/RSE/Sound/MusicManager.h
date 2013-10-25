// ------------------------------------------------------------------------------------------------
//
// MusicManager
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/RSE.h"
#include "RSE/PathResource.h"
#include "Standard/Base.h"
#include "Standard/PtrGC.h"
#include "Standard/StateMachine.h"

class RSE_API Music : public Base
{
	USE_RTTI(Music, Base);
public:
	float volume;
protected:
	virtual void streamVars(StreamVars& v);
};

class RSE_API MusicManager : public Base
{
	USE_RTTI(MusicManager, Base);
public:
	MusicManager();

	void update(float delta);

	void loadPlaylist(const std::string& playlist, bool shuffle = false);
	void clearPlaylist();
	void startPlaylist();
	void setShuffle(bool b);
	void play(const PathResource& fileName);
	void fade(float time = 2.0f);
	void stop();

protected:
	DECLARE_STATECLASS(MusicManager);

	STATE(Default)
	{
		void update(float delta);
	};

	STATE(Transition)
	{
		StateTransition()
		{
			time = 2;
		}

		void start();
		void loadSong();
		
		float time;
		PathResource nextSongPath;

	protected:
		PtrGC<class Thread> curThread;
		bool loading;
		bool songSuccess;
		float startVolume;
		double startTime;
		PtrGC<class IMusic> nextSong;
	};

	EXTEND_STATE(FadeOut, StateTransition)
	{
		void update(float delta);
	};

	DECLARE_STATEMACHINE;

	void doPlay(const PathResource& fileName);

	typedef std::vector<std::string> Playlist;
	typedef stdext::hash_map<std::string, Playlist> Playlists;
	Playlists m_playlists;
	Playlist m_currentPlaylist;
	std::string m_currentPlaylistName;
	int m_playlistProgress;
	bool m_playlistPlaying;
	bool m_forceNextPlaylist;

	typedef stdext::hash_map<std::string, Music> MusicMap;
	MusicMap m_music;

	PtrGC<class IMusic> m_currentSong;

	bool m_shuffle;
};
