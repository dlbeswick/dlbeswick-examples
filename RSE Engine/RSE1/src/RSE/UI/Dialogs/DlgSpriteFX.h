// ------------------------------------------------------------------------------------------------
//
// DlgSpriteFX
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "DlgStandard.h"
#include "Game/SpriteFX.h"


class DlgSpriteFX : public DlgStandard
{
	USE_RTTI(DlgSpriteFX, DlgStandard);
public:
	DlgSpriteFX();
	~DlgSpriteFX();

protected:
	virtual void construct();

	void onAnimSet(const PtrGC<UIElement>& u);
	void onAnimSequence(const PtrGC<UIElement>& u);
	void onFX(const PtrGC<UIElement>& u);
	void onAddFX();
	void onDelFX();
	void onConstructFX(std::string desc);
	void onSave();
	void onFrame();
	void onCommit();
	void onDisplayMarker();
	void fillSequence(const std::string& setName);

	class UICombo* m_animSets;
	class UICombo* m_animSequences;
	class UISpriteDisplay* m_display;
	class UIButton* m_addFX;
	class UIButton* m_delFX;
	class UIButton* m_save;
	class UIListView* m_fx;

	class DlgObjProperties* m_fxProperties;
	std::vector<class SpriteFX*>* m_currentFX;
};