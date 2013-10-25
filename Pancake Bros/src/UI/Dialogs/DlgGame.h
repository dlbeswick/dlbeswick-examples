// ---------------------------------------------------------------------------------------------------------
// 
// DlgGame
// 
// ---------------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/UI/Dialogs/DlgStandardLevel.h"


class DlgGame : public DlgStandardLevel
{
	USE_RTTI(DlgGame, DlgStandardLevel);
public:
	DlgGame(const std::string& levelName="Level 1");
	~DlgGame();

	class DlgGameHUD& hud() const { return *m_hud; }

	virtual void update(float delta);

	class PancakeLevel& level() { return (PancakeLevel&)Super::level(); }

	virtual bool mouseDown(const Point2& pos, int button);

protected:
	virtual void construct();
	virtual void onActivate();
	virtual void onDraw();
	virtual void navigation() {}

	virtual void setupMenu(UIMenu& menu);

	virtual Level* createLevel();
	virtual DlgDebug* createDlgDebug() const;

	void onAIDebug(const MenuItemText&);
	void onWave(const MenuItemText& m);

	PtrGC<class Man> manSelected;

	class DlgGameHUD* m_hud;

private:
	virtual void onDestroy();
};
