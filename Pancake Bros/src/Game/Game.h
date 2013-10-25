// ------------------------------------------------------------------------------------------------
//
// Game
// TBD: Generalize
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include <Standard/StateMachine.h>
#include <Standard/Base.h>
#include <Standard/PendingList.h>

class PlayerMan;


class Game : public Base
{
	USE_RTTI(Game, Base);
public:
	Game();

	virtual void update(float delta);

	void startGame();
	void levelOver();
	void nextLevel(int num = -1);
	void gameOver();
	bool hasNextLevel() const;

	void onDlgGameDestroy();
	void onGotPancake(const PtrGC<class Man>& who, const PtrGC<class Head>& what);

	int currentLevel() const { return m_currentLevel; }
	static std::string levelName(int num);

	PtrGC<class PlayerMan> player(int idx);

protected:
	// states
	DECLARE_STATECLASS(Game);
	DECLARE_STATEMACHINE;

	void loadNextLevel();
	void clearPlayers();
	void goIngame();

	STATE(Metagame)
	{
		void start();

		bool shouldStartGameForNotitle;
	};

	STATE(Ingame)
	{
		void start();
		void end();
		void update(float delta);
		void checkNextLevel();
	};

	STATE(Pregame)
	{
		void start();
		void loadLevel();
		void go();
		PtrGC<class DlgGame> nextDlg;
		PtrGC<class DlgPregame> pregame;
	};

	PtrGC<class DlgGame> m_dlgGame;
	PtrGC<class DlgTitle> m_dlgTitle;
	std::vector<PtrGC<PlayerMan> > m_players;

	int m_currentLevel;
	int m_nextLevel;
};