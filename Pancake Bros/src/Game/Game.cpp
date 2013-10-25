// ------------------------------------------------------------------------------------------------
//
// Game
//
// ------------------------------------------------------------------------------------------------
#include "stdafx.h"

#include "RSEApp.h"
#include "Game.h"
#include "Man.h"
#include "PancakeLevel.h"
#include "UI/Dialogs/DlgGame.h"
#include "UI/Dialogs/DlgGameHUD.h"
#include "UI/Dialogs/DlgTitle.h"
#include "RSE/Physics/PhysSphere.h"
#include "RSE/Sound/MusicManager.h"
#include "RSE/UI/Controls/UITextBox.h"
#include "RSE/UI/Controls/UIPic.h"
#include "RSE/UI/DialogMgr.h"
#include "RSE/UI/Transitions/TransitionFade.h"
#include "RSE/UI/Transitions/TransitionPieces.h"
#include <Standard/Config.h>
#include <Standard/CriticalSection.h>
#include <Standard/Thread.h>
#include <process.h>

class DlgPregame : public Dialog
{
	USE_RTTI(DlgPregame, Dialog);
public:
	DlgPregame(const std::string& levelText) :
	  _levelText(levelText)
	{
	}

	virtual void construct()
	{
		Super::construct();

		setFrame(registrant("DialogFrameNaked")->rtti);

		setColour(RGBA(0,0,0));

		UILabel* text = new UILabel;
		addChild(text);
		text->setText(_levelText);
		text->setPos(Point2::ZERO);
		text->setSize(Point2(1, 0.75f));
		text->setWrap(true);
		text->align(ALIGN_CENTER, VALIGN_CENTER);

		setPointer(0);
	}

	virtual void draw()
	{
		Super::draw();
	}

protected:
	std::string _levelText;
};

IMPLEMENT_RTTI(DlgPregame);
IMPLEMENT_RTTI(Game);

Game::Game() :
	m_currentLevel(0),
	m_nextLevel(1)
{
	BEGIN_STATEMACHINE;
	ADD_STATE(Ingame);
	ADD_STATE(Metagame);
	ADD_STATE(Pregame);

	Metagame->shouldStartGameForNotitle = 
		AppBase().getCmdLineOption("d").empty()	&& AppBase().hasCmdLineOption("notitle");

	states.go(Metagame);

	// tbd: move to rse
	if (AppBase().getCmdLineOption("d").empty())
	{
		if (!AppBase().hasCmdLineOption("notitle"))
			m_dlgTitle = Cast<DlgTitle>(DialogMgr().add("DlgTitle"));
	}
}

void Game::update(float delta)
{
	Super::update(delta);
	states.update(delta);
}

void Game::nextLevel(int num)
{
	if (num == -1)
		++m_nextLevel;
	else
	{
		m_nextLevel = num;
		m_currentLevel = -1;
	}

	if (!App().configLevel().exists(std::string("Level ") + m_nextLevel))
	{
		gameOver();
	}
}

void Game::startGame()
{
	dlog << "Start game" << dlog.endl;

	m_currentLevel = 0;

	if (AppBase().hasCmdLineOption("l"))
		m_nextLevel = atoi(AppBase().getCmdLineOption("l").c_str());
	else
		m_nextLevel = 1;

	states.go(Ingame);
}

bool Game::hasNextLevel() const
{
	return m_nextLevel != -1;
}

void Game::goIngame()
{
	states.go(Ingame);
}

void Game::onDlgGameDestroy()
{
	states.go(Metagame);
}

void Game::gameOver()
{
	dlog << "Game over" << dlog.endl;

	clearPlayers();

	DialogMgr().add("DlgTitle");
	new UITransition(new TransitionPieces(m_dlgGame, 1), 0);
	states.go(Metagame); // need this because dialog isn't destroyed until transition end. can it be done nicer?
}

void Game::clearPlayers()
{
	for (std::vector<PtrGC<PlayerMan> >::iterator i = m_players.begin(); i != m_players.end(); ++i)
	{
		i->destroy();
	}
	m_players.clear();
}

void Game::loadNextLevel()
{
	/*Critical(App().criticalMainLoop());

	int loadingLevel = m_currentLevel;
	DlgGame* oldDlg = m_dlgGame.ptr();

	if (loadingLevel == m_currentLevel)
	{
		m_dlgGame = game;

		// unpossess player of old level
		if (oldDlg)
			oldDlg->level().player()->controller()->possess(0);

		if (m_dlgTitle)
			DialogMgr(->addChild(new.mainBranch()) UITransition(new TransitionFade(m_dlgTitle, 0.25), new TransitionFade(m_dlgGame, 1)));
		else
			DialogMgr(->addChild(new.mainBranch()) UITransition(new TransitionFade(oldDlg, 1), new TransitionFade(m_dlgGame, 1)));
	}
	else // aborted
		delete game;*/
}

std::string Game::levelName(int num)
{
	return std::string("Level ") + num;
}

PtrGC<PlayerMan> Game::player(int idx)
{
	if (idx >= (int)m_players.size())
		return 0;

	return m_players[idx];
}

void Game::onGotPancake(const PtrGC<class Man>& who, const PtrGC<class Head>& what)
{
	if (m_dlgGame)
		m_dlgGame->hud().addPancake(what);
}

void Game::levelOver()
{
	for (std::vector<PtrGC<PlayerMan> >::iterator i = m_players.begin(); i != m_players.end(); ++i)
	{
		(*i)->controller().destroy();
	}

	if (m_dlgGame)
		m_dlgGame->hud().onLevelOver();
}

////

void Game::StateMetagame::start()
{
	o->clearPlayers();

	if (shouldStartGameForNotitle)
	{
		shouldStartGameForNotitle = false;
		o->startGame();
	}
}

////

void Game::StateIngame::update(float delta)
{
	checkNextLevel();
}

void Game::StateIngame::start()
{
	checkNextLevel();
}

void Game::StateIngame::end()
{
	for (std::vector<PtrGC<PlayerMan> >::iterator i = o->m_players.begin(); i != o->m_players.end(); ++i)
	{
		if ((*i)->controller())
			(*i)->controller()->possess(0);
		(*i)->setParent(0);
		(*i)->physics()->setParent(0);
		(*i)->physics()->removeFromGroups();
	}
}

void Game::StateIngame::checkNextLevel()
{
	if (o->m_currentLevel != o->m_nextLevel)
	{
		o->m_currentLevel = o->m_nextLevel;

		o->m_dlgGame.destroy();
		o->states.go(o->Pregame);
	}
}

////

void Game::StatePregame::start()
{
	std::string curLevel = o->levelName(o->m_currentLevel);
	nextDlg.destroy();

	std::string title;
	App().configLevel().get(curLevel, "Title", title);

	pregame = UIElement::dlgRoot()->addChild(new DlgPregame(title));
	counters.set(0.05f, delegate(&StatePregame::loadLevel));
}

void Game::StatePregame::loadLevel()
{
	std::string curLevel = o->levelName(o->m_currentLevel);

	if (o->m_players.empty())
	{
		PlayerMan* m = new PlayerMan("hammerman");
		o->m_players.push_back(m);
	}

	nextDlg = new DlgGame(curLevel);
	counters.set(std::max(0.0f, nextDlg->level().get<float>("TitleTime")), delegate(&StatePregame::go));

	if (nextDlg->level().hasMusic())
	{
		App().music().fade(1.0f);
		App().music().clearPlaylist();
	}
}

void Game::StatePregame::go()
{
	pregame.destroy();
	UIElement::dlgRoot()->addChild(nextDlg);
	nextDlg->level().begin();
	o->m_dlgGame = nextDlg;
	o->states.go(o->Ingame);
}
