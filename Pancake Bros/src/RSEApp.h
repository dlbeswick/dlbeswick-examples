// ------------------------------------------------------------------------------------------------
//
// RSEApp
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/AppBase.h"

class TextureAnimationManager;
class ParticleFactory;


class RSEApp : public SAppBase
{
public:
	RSEApp(const std::string& cmdLine);
	~RSEApp();

	Config& configGame() { return *m_configGame; }
	Config& configLevel() { return *m_configLevel; };
	Config& configAI() { return *m_configAI; };

	class Game& game() const { return *m_game; }

	virtual void update(float delta);

	virtual Path pathLocalSettings() const;

protected:
	virtual void init();
	virtual FilesystemCompound* constructFilesystem() const;

	Config* m_configGame;
	Config* m_configLevel;
	Config* m_configAI;
	class Game* m_game;
};

inline RSEApp& App() { return (RSEApp&)RSEApp::current(); }