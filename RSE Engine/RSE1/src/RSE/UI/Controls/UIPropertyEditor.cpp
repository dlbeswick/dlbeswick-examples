// ---------------------------------------------------------------------------------------------------------
// 
// UIPropertyEditor
// 
// ---------------------------------------------------------------------------------------------------------
#include "pch.h"

#include "UIPropertyEditor.h"
#include "AppBase.h"
#include "PathResource.h"
#include "Sound/ISample.h"
#include "Sound/ISampleInstance.h"
#include "Sound/ISoundProvider.h"
#include "UI/Controls/UIButton.h"

// ConfigPropertyEditorBase
IMPLEMENT_RTTI(ConfigPropertyEditorBase);

ConfigPropertyEditorBase::ConfigPropertyEditorBase(Config& c, const std::string& category, const std::string& varName) :
	m_config(c),
	m_category(category),
	m_varName(varName)
{
}

void ConfigPropertyEditorBase::setEditor(UIPropertyEditorBase& editor)
{
	m_editor = &editor;
	addChild(m_editor);
	fitToChildren();
}

UIPropertyEditorBase& ConfigPropertyEditorBase::editor() const
{
	return *m_editor;
}

// UIPropertyEditorBase
const PtrGC<UIElement>& UIPropertyEditorBase::actuator()
{
	assert(children()[0]); 
	return children()[0];
}

// UIPropertyEditorSpinners
UIPropertyEditorSpinners::UIPropertyEditorSpinners(const std::string& title, uint num, const std::string& help, bool fractional) :
	  UIPropertyEditorBase(title, help)
{
	for (uint i = 0; i < num; ++i)
	{
		m_sliders.push_back(addChild(new UIValueSlider));
		m_sliders[i]->setExtent(-100, 100);
		m_sliders[i]->setValue(0);
		m_sliders[i]->setFractional(fractional);

		if (i > 0)
			m_sliders[i]->setPos(Point2(m_sliders[i-1]->pos().x + m_sliders[i-1]->size().x, 0));
	}
}

std::string UIPropertyEditorSpinners::validate()
{
	return "";
}

void UIPropertyEditorSpinners::setOnModify(const Delegate& d)
{
	for (uint i = 0; i < m_sliders.size(); ++i)
	{
		m_sliders[i]->onChange = d;
	}
}

// UIPropertyEditorSoundTag
UIPropertyEditorSoundTag::UIPropertyEditorSoundTag(const std::string& title, std::string& var,const std::string& help) :
	UIPropertyEditorTextBoxWithButton<std::string>(title, var, help, 25, new UIButton("Play"))
{
	m_button->onClick = delegate(&UIPropertyEditorSoundTag::play);
}

void UIPropertyEditorSoundTag::play()
{
	try
	{
		AppBase().sound().sample(PathResource(m_var)).play();
	}
	catch (Exception&)
	{
	}
}
