// ------------------------------------------------------------------------------------------------
//
// DlgGameHUD
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/UI/Dialog.h"
#include "Standard/Range.h"

class PlayerMan;
class Head;

class DlgGameHUD : public Dialog
{
	USE_RTTI(DlgGameHUD, Dialog);

public:
	DlgGameHUD(const std::string& styleName);

	void setTarget(const PtrGC<PlayerMan>& m);
	virtual void update(float delta);

	virtual bool usesFocus() const { return false; }

	virtual void addPancake(const PtrGC<Head>& head);

	UISTYLE
		Point2 pancakesStart;
		Range<float> pancakeScaleOnStack;
		Range<Point2> pancakeOffset;
		Range<int> pancakesPerStack;
		Range<Point2> pancakeStackOffset;
		float pancakeGiveFreq;
		Point2 barSize;
		Point2 barPos;
	UISTYLEEND

	virtual void onLevelOver();

	class UIStateInfo& info() const { return *m_info; }

protected:
	virtual void construct();

	void givePancake();
	void onPancakesDone();

	class UIStatusBar* m_bar;
	PtrGC<PlayerMan> m_target;
	Point2 m_currentStackPos;

	class UIStateInfo* m_info;

	typedef std::vector<class UIPancake*> Pancakes;
	typedef std::vector<Pancakes> PancakeStacks;
	PancakeStacks m_stacks;
};