// ------------------------------------------------------------------------------------------------
//
// DlgDefaultOptions
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "AppBase.h"
#include "DlgDefaultOptions.h"
#include "Sound/ISoundProvider.h"
#include "UI/DialogMgr.h"
#include "UI/UILayout.h"
#include "UI/Controls/UIButton.h"
#include "UI/Controls/UIKeyboardMenu.h"
#include "UI/Controls/UIPic.h"
#include "UI/Controls/UIPropertyEditor.h"
#include "UI/Controls/UITextBox.h"
#include <UI/Transitions/TransitionPieces.h>
#include <Standard/Config.h>

IMPLEMENT_RTTI(DlgDefaultOptions)
IMPLEMENT_RTTI(DlgDefaultOptionsKeyboard)

void DlgDefaultOptions::construct()
{
	Super::construct();

	addOptions();
}

void DlgDefaultOptions::addOptions()
{
	addOptions(m_options);
}

void DlgDefaultOptions::onOk()
{
	for (Elements::iterator i = m_options.begin(); i != m_options.end(); ++i)
		Cast<ConfigPropertyEditorBase>(*i)->commit();

	close();
}

void DlgDefaultOptions::onCancel()
{
	for (Elements::iterator i = m_options.begin(); i != m_options.end(); ++i)
		Cast<ConfigPropertyEditorBase>(*i)->revert();

	close();
}

void DlgDefaultOptions::close()
{
	self().destroy();
}

void DlgDefaultOptions::addOptions(Elements& list)
{
	ConfigPropertyEditor<float>* c;

	// example of custom property editor
	c = new ConfigPropertyEditor<float>(AppBase().options(), "Sound", "GlobalVolume");
	c->setEditor(*property("Global Volume", c->var(), "Adjusts the volume of all sound.", 0.0f, 1.0f, true));
	c->onChanged = delegate(&DlgDefaultOptions::onGlobalVolume);
	list.push_back(c);

	c = new ConfigPropertyEditor<float>(AppBase().options(), "Sound", "SoundVolume");
	c->setEditor(*property("Sound Volume", c->var(), "Adjusts the volume of sound effects.", 0.0f, 1.0f, true));
	c->onChanged = delegate(&DlgDefaultOptions::onSoundVolume);
	list.push_back(c);

	c = new ConfigPropertyEditor<float>(AppBase().options(), "Sound", "MusicVolume");
	c->setEditor(*property("Music Volume", c->var(), "Adjusts the volume of music.", 0.0f, 1.0f, true));
	c->onChanged = delegate(&DlgDefaultOptions::onMusicVolume);
	list.push_back(c);

	// standard usage
}

void DlgDefaultOptions::onMusicVolume(const float& v)
{
	AppBase().sound().setMusicVolume(v);
}

void DlgDefaultOptions::onSoundVolume(const float& v)
{
	AppBase().sound().setSoundVolume(v);
}

void DlgDefaultOptions::onGlobalVolume(const float& v)
{
	AppBase().sound().setGlobalVolume(v);
}

////

DlgDefaultOptionsKeyboard::DlgDefaultOptionsKeyboard()
{
}

void DlgDefaultOptionsKeyboard::construct()
{
	Super::construct();

	m_menu = addChild(new UIKeyboardMenu);
	m_layout = addChild(new UILayoutTable);
	m_layout->setSize(Point2(0.7f, 0.55f));
	m_layout->align(ALIGN_CENTER, VALIGN_CENTER);
	m_layout->setCols(1);

	UILayoutTable* table = addChild(new UILayoutTable);
	table->setCols(2);
	table->setSize(Point2(0.75f, 0.1f));
	table->align(ALIGN_CENTER, VALIGN_BOTTOM);
	
	m_ok = table->addChild(new UIButton("OK"));
	m_ok->onClick = delegate(&DlgDefaultOptionsKeyboard::onOk);

	m_cancel = table->addChild(new UIButton("Cancel"));
	m_cancel->onClick = delegate(&DlgDefaultOptionsKeyboard::onCancel);

	setFrame(registrant("DialogFrameNaked")->rtti);
	setPointer(0);

	layout(m_options);
}

void DlgDefaultOptionsKeyboard::layout(Elements& list)
{
	for (Elements::iterator i = m_options.begin(); i != m_options.end(); ++i)
	{
		ConfigPropertyEditorBase& e = *Cast<ConfigPropertyEditorBase>(*i);

		e.editor().refresh();
		e.editor().fitToChildren();
		e.fitToChildren();
		UILayoutTable* table = m_layout->addChild(new UILayoutTable);
		table->setSize(Point2(0.7f, 0.1f));
		table->setCols(2);
		table->addChild(new UILabel(e.editor().title()));
		table->addChild(*i);
		m_menu->add(table);
	}

	m_menu->add(m_ok);
	m_menu->add(m_cancel);
}

void DlgDefaultOptionsKeyboard::close()
{
	new UITransition(new TransitionPieces(this, 1), 0);
}
