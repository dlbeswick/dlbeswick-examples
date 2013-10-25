// ------------------------------------------------------------------------------------------------
//
// RSEApp
//
// ------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "RSEApp.h"
#include "RSE/Render/TextureAnimationManager.h"
#include "RSE/Render/ParticleFactory.h"
#include "RSE/Render/ParticleSystem.h"
#include "RSE/UI/DialogMgr.h"
#include "Standard/Config.h"
#include "Standard/FileMgr.h"
#include "Game/Game.h"

void RSEApp::init()
{
	SAppBase::init();

	pathLocalSettings().create(); 

	loadConfig("game.ini");
	loadConfig("levels.ini");
	loadConfig("ai.ini");
	loadConfig("parallax.ini");

	m_configGame = &config("game.ini");
	m_configLevel = &config("levels.ini");
	m_configAI = &config("ai.ini");

	m_game = new Game;
}

RSEApp::RSEApp(const std::string& cmdLine) : 
	SAppBase("Pancake Bros", "", cmdLine)
{
	//m_updateLoopFreq = 0;
}

RSEApp::~RSEApp()
{
}

void RSEApp::update(float delta)
{
	SAppBase::update(delta);

	m_game->update(delta);
}

Path RSEApp::pathLocalSettings() const
{
	return pathResources() + Path("/local");
}

FilesystemCompound* RSEApp::constructFilesystem() const
{
	FilesystemCompound* filesystem = SAppBase::constructFilesystem();

	filesystem->addFileMethod(new FileMethodDiskRelative(pathLocalSettings()));
	filesystem->addFileMethod(new FileMethodDiskRelative(pathResources() + Path("cfg")));
	filesystem->addFileMethod(new FileMethodDiskRelative(pathResources()));
	filesystem->addFileMethod(new FileMethodDiskRelative(pathResources() + Path("media")));

	return filesystem;
}
