// ------------------------------------------------------------------------------------------------
//
// FMODMusic
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "FMODMusic.h"
#include "FMODProvider.h"

FMODMusic::FMODMusic(ibinstream& s, FMODProvider& provider) :
	m_provider(provider),
	m_loaded(false)
{
	char* buf;
	size_t length;
	s.toBuf(buf, length);

	m_handle = FMUSIC_LoadSongEx(buf, 0, length, FSOUND_LOADMEMORY, 0, 0);
	if (!m_handle)
		throwf("FMODMusic: FMUSIC_LoadSongEx failed: " + FMODProvider::error());

	setVolume(FMUSIC_GetMasterVolume(m_handle) / 256.0f);

	m_loaded = true;
}

float FMODMusic::globalMusicVolume() const
{
	return m_provider.musicVolume() * m_provider.globalVolume();
}

FMODMusic::~FMODMusic()
{
	FMUSIC_FreeSong(m_handle);
}

void FMODMusic::play()
{
	FMUSIC_PlaySong(m_handle);
}

void FMODMusic::stop()
{
	FMUSIC_StopSong(m_handle);
}

float FMODMusic::volume() const
{
	return m_volume;
}

bool FMODMusic::playing() const
{
	return FMUSIC_IsPlaying(m_handle) != 0;
}

void FMODMusic::setVolume(float f)
{
	m_volume = f;
	FMUSIC_SetMasterVolume(m_handle, (int)(f * globalMusicVolume() * 256.0f));
}
