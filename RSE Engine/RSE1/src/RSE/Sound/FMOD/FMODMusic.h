// ------------------------------------------------------------------------------------------------
//
// FMODMusic
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "Standard/binstream.h"
#include "Sound/IMusic.h"

class RSE_API FMODMusic : public IMusic
{
public:
	FMODMusic(ibinstream& s, class FMODProvider& provider);
	~FMODMusic();

	virtual void play();
	virtual void stop();

	virtual float volume() const;
	virtual bool playing() const;

	virtual void setVolume(float f);

	virtual bool ready() const { return m_loaded; }

protected:
	virtual float globalMusicVolume() const;

	FMUSIC_MODULE* m_handle;
	class FMODProvider& m_provider;
	float m_volume;
	bool m_loaded;
};