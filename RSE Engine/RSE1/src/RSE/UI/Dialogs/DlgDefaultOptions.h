// ------------------------------------------------------------------------------------------------
//
// DlgDefaultOptions
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/UI/Dialogs/DlgStandard.h"

class RSE_API DlgDefaultOptions : public DlgStandard
{
	USE_RTTI(DlgDefaultOptions, DlgStandard);
public:
	virtual void addOptions();
	virtual void close();

protected:
	virtual void construct();

	void onOk();
	void onCancel();

	virtual void addOptions(Elements& list);

	Elements m_options;

	void onMusicVolume(const float& v);
	void onSoundVolume(const float& v);
	void onGlobalVolume(const float& v);
};

class RSE_API DlgDefaultOptionsKeyboard : public DlgDefaultOptions
{
	USE_RTTI(DlgDefaultOptionsKeyboard, DlgDefaultOptions);
public:
	DlgDefaultOptionsKeyboard();

	virtual void close();

protected:
	virtual void construct();
	virtual void layout(Elements& list);

	class UILayoutTable* m_layout;
	class UIKeyboardMenu* m_menu;
	class UIButton* m_ok;
	class UIButton* m_cancel;
};