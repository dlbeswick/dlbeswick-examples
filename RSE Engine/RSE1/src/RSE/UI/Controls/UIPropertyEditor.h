// ---------------------------------------------------------------------------------------------------------
// 
// Classes and interfaces for exposing objects to the "object property editor"
// 
// ---------------------------------------------------------------------------------------------------------
#ifndef RSE_UIPROPERTYEDITOR_H
#define RSE_UIPROPERTYEDITOR_H

#include "RSE/UI/UILayout.h"
#include "RSE/UI/UIElement.h"
#include "RSE/UI/Controls/UIButton.h"
#include "RSE/UI/Controls/UITextBox.h"
#include "RSE/UI/Controls/UICombo.h"
#include "RSE/UI/Controls/UIValueSlider.h"

typedef std::vector<PtrGC<class UIPropertyEditorBase> > PropertyEditorList;

// special case for direct editing of config variables
class RSE_API ConfigPropertyEditorBase : public UIElement
{
public:
	USE_RTTI(ConfigPropertyEditorBase, UIElement);

	ConfigPropertyEditorBase(Config& c, const std::string& category, const std::string& varName);

	void setEditor(UIPropertyEditorBase& editor);

	UIPropertyEditorBase& editor() const;

	virtual void commit() = 0;
	virtual void revert() = 0;

protected:
	UIPropertyEditorBase* m_editor;
	Config& m_config;
	std::string m_category;
	std::string m_varName;
};

// UIPropertyEditorBase
class RSE_API UIPropertyEditorBase : public UIElement
{
public:
	UIPropertyEditorBase(const std::string& title, const std::string& help) :
		m_title(title)
	{
		setHelp(help);
	}
	
	virtual std::string validate() { return ""; }
	
	void refresh()
	{
		m_changed = false;
		doRefresh();
	}
	
	void commit()
	{
		m_changed = true;
		doCommit();
	}
	
	const std::string& title() const { return m_title; }

	bool changed() const { return m_changed; }

	void setOnModify(const TPDelegate<UIPropertyEditorBase&>& d)
	{
		m_onModify = d;
		setOnModify(delegate(&UIPropertyEditorBase::onModifyCalled));
	}

	const PtrGC<UIElement>& actuator();

	// not for public use. sorry.
	virtual void setOnModify(const Delegate& d) = 0;

protected:
	UIPropertyEditorBase() {};

	virtual void doRefresh() = 0;
	virtual void doCommit() = 0;

	template <class Q>
	std::string stringVar(const Q& var)
	{
		std::ostringstream* outStr = new std::ostringstream;
		otextstream os(outStr);
		os << var;

		return outStr->str();
	}

	template <class Q>
	std::string setVar(Q& var, const std::string& strRep)
	{
		itextstream s(strRep);
		s << itextstream::str_delimiter("");
		s << itextstream::whitespace("");
		
		try
		{
			s >> var;
		}
		catch(...)
		{
		}

		return stringVar(var);
	}

	template <class Q>
	void setVarExact(Q& var, const Q& varIn)
	{
		var = varIn;
	}

	float					m_elementX;
	std::string				m_title;
	bool					m_changed;

private:
	void onModifyCalled()
	{
		m_onModify(*this);
	}

	TPDelegate<UIPropertyEditorBase&> m_onModify;
};

template <class T>
class RSE_API ConfigPropertyEditor : public ConfigPropertyEditorBase
{
public:
	ConfigPropertyEditor(Config& c, const std::string& category, const std::string& varName, const std::string& help, const std::string& displayName = "") :
		ConfigPropertyEditorBase(c, category, varName)
	{
		init(category, varName);

		const std::string* name;
		if (displayName != "")
			name = &displayName;
		else
			name = &varName;

		setEditor(*property(*name, m_var, help));
	}

	void setEditor(UIPropertyEditorBase& editor)
	{
		ConfigPropertyEditorBase::setEditor(editor);
		editor.setOnModify(delegate(&ConfigPropertyEditor::onModifyCalled));
	}

	ConfigPropertyEditor(Config& c, const std::string& category, const std::string& varName, const T& defaultVar = T()) :
		ConfigPropertyEditorBase(c, category, varName),
		m_var(defaultVar)
	{
		init(category, varName);
	}

	T& var() { return m_var; }

	virtual void commit()
	{
		m_config.set<T>(m_category, m_varName, m_var);
	}

	virtual void revert()
	{
		m_config.get<T>(m_category, m_varName, m_var);
		onChanged(m_var);
	}

	TPDelegate<const T&> onChanged;

protected:
	void onModifyCalled()
	{
		m_editor->commit();
		onChanged(m_var);
	}

	void init(const std::string& category, const std::string& varName)
	{
		m_config.get<T>(category, varName, m_var);
	}

	T m_var;
};

template <class Q>
class RSE_API UIPropertyEditor : public UIPropertyEditorBase
{
public:
	UIPropertyEditor(const std::string& title, Q& var, const std::string& help) :
		UIPropertyEditorBase(title, help), m_var(var)
	{
		setHelp(help);
	}

	Q& var() { return m_var; }

protected:
	Q& m_var;
};

// UIPropertyEditorTextBox
template <class Q>
class RSE_API UIPropertyEditorTextBox : public UIPropertyEditor<Q>
{
public:
	UIPropertyEditorTextBox(const std::string& title, Q& var, const std::string& help, int chars) :
	  UIPropertyEditor<Q>(title, var, help)
	{
		init(chars);
	}

	void init(int chars)
	{
		m_pTextBox = this->addChild(new UITextBox);
		m_pTextBox->setSize(chars);
	}

	virtual std::string validate() { return ""; }

protected:
	virtual void setOnModify(const Delegate& d) { m_pTextBox->onFocusLost = d; }

	virtual void doRefresh()
	{
		m_pTextBox->setText(this->stringVar(this->m_var));
	}

	virtual void doCommit()
	{
		this->setVar(this->m_var, m_pTextBox->text());
	}

	UITextBox*						m_pTextBox;
};

template <class Q>
inline UIPropertyEditor<Q>* propertyTextBox(const std::string& title, Q& var, const std::string& help, int chars = 25)
{
	return new UIPropertyEditorTextBox<Q>(title, var, help, chars);
}


template<> inline std::string UIPropertyEditorTextBox<float>::validate()
{
	const std::string& s = m_pTextBox->text();
	
	for (uint i = 0; i < s.size(); i++)
	{
		if (isalpha(s[i]))
			return "No alphabetic characters are allowed";
	}
	
	return "";
}

template<> inline std::string UIPropertyEditorTextBox<int>::validate()
{
	const std::string& s = m_pTextBox->text();
	
	for (uint i = 0; i < s.size(); i++)
	{
		if (isalpha(s[i]))
			return "No alphabetic characters are allowed";

		if (s[i] == '.')
			return "No fractional portion is allowed";
	}
	
	return "";
}


// UIPropertyEditorTextBoxWithButton
template <class Q>
class RSE_API UIPropertyEditorTextBoxWithButton : public UIPropertyEditorTextBox<Q>
{
public:
	UIPropertyEditorTextBoxWithButton(const std::string& title, Q& var, const std::string& help, int chars, UIButton* button) :
		UIPropertyEditorTextBox<Q>(title, var, help, chars), m_button(button)
	{
		// make button
		this->addChild(m_button);
		m_button->fitToData();
		/*m_button->setPos(Point2(m_container->size().x - m_button->size().x, 0));

		// shorten textbox to fit button if necessary
		if (m_pTextBox->size().x > m_button->pos().x)
			m_pTextBox->setSize(Point2(m_button->pos().x, m_pTextBox->size().y));*/
	}

protected:
	virtual void _onFontChange()
	{
		this->m_button->setPos(Point2(this->m_pTextBox->size().x, 0));
	}

	UIButton*						m_button;
};

// UIPropertyEditorSoundTag
class RSE_API UIPropertyEditorSoundTag : public UIPropertyEditorTextBoxWithButton<std::string>
{
public:
	UIPropertyEditorSoundTag(const std::string& title, std::string& var, const std::string& help);

	void play();
};

// UIPropertyEditorCombo
template <class Q>
class RSE_API UIPropertyEditorCombo : public UIPropertyEditor<Q>
{
public:
	UIPropertyEditorCombo(const std::string& title, Q& var, const std::string& help, const std::vector<Q>& items) :
	  UIPropertyEditor<Q>(title, var, help)
	{
		m_pCombo = new TUICombo<Q>;
        for (uint i = 0; i < items.size(); ++i)
			m_pCombo->add(new ListText(this->stringVar(items[i])));
		//m_pCombo->setSize(Point2(0.23f, 0.5f));

		m_pCombo->onSelect = delegate(&UIPropertyEditorCombo::onSelect);
		this->addChild(m_pCombo);
	}

	virtual std::string validate() { return ""; }

protected:
	virtual void setOnModify(const Delegate& d) { this->m_onModify = d; }

	virtual void doCommit()
	{
		this->setVarExact(this->m_var, *this->m_pCombo->selected());
	}

	virtual void doRefresh()
	{
		assert(0);
		// tbd: add combo select-by-value method
		//this->m_pCombo->select(this->m_var);
	}

	void onSelect(const PtrGC<UIElement>& d) { this->m_onModify(); }

	TUICombo<Q>*					m_pCombo;
	Delegate						m_onModify;
};

template <class Q>
UIPropertyEditor<Q>* propertyCombo(const std::string& title, Q& var, const std::string& help, const std::vector<Q>& items)
{
	return new UIPropertyEditorCombo<Q>(title, var, help, items);
}

// UIPropertyEditorValueSlider
template <class Q>
class RSE_API UIPropertyEditorValueSlider : public UIPropertyEditor<Q>
{
public:
	UIPropertyEditorValueSlider(const std::string& title, Q& var, const std::string& help, bool bFractional, float min, float max, bool bHardLimit) :
	  UIPropertyEditor<Q>(title, var, help), m_var(var)
	{
		m_slider = this->addChild(new UIValueSlider);
		m_slider->setFractional(bFractional);
		m_slider->setHardLimit(bHardLimit);
		m_slider->setExtent(min, max);
		m_slider->setValue(0);
	}

	virtual std::string validate() { return ""; }

protected:
	virtual void setOnModify(const Delegate& d) { m_slider->onChange = d; }

	virtual void doRefresh()
	{
		m_slider->setValue((float)m_var);
	}

	virtual void doCommit()
	{
		m_var = (Q)m_slider->value();
	}

	UIValueSlider*					m_slider;
	Q&								m_var;
};

// UIPropertyEditorSpinners
class RSE_API UIPropertyEditorSpinners : public UIPropertyEditorBase
{
public:
	UIPropertyEditorSpinners(const std::string& title, uint num, const std::string& help, bool fractional);

	virtual std::string validate();

protected:
	virtual void setOnModify(const Delegate& d);

	std::vector<UIValueSlider*>	m_sliders;
};

// UIPropertyEditorPoint2
class RSE_API UIPropertyEditorPoint2 : public UIPropertyEditorSpinners
{
public:
	UIPropertyEditorPoint2(const std::string& title, Point2& var, const std::string& help) :
	  UIPropertyEditorSpinners(title, 2, help, true), m_var(var)
	{
	}

	Point2& var() { return m_var; }

protected:
	virtual void doRefresh()
	{
		m_sliders[0]->setValue(m_var.x);
		m_sliders[1]->setValue(m_var.y);
	}

	virtual void doCommit()
	{
		m_var.x = m_sliders[0]->value();
		m_var.y = m_sliders[1]->value();
	}

private:
	Point2& m_var;
};

// UIPropertyEditorPoint3
class RSE_API UIPropertyEditorPoint3 : public UIPropertyEditorSpinners
{
public:
	UIPropertyEditorPoint3(const std::string& title, Point3& var, const std::string& help) :
	  UIPropertyEditorSpinners(title, 3, help, true), m_var(var)
	{
	}

	Point3& var() { return m_var; }

protected:
	virtual void doRefresh()
	{
		m_sliders[0]->setValue(m_var.x);
		m_sliders[1]->setValue(m_var.y);
		m_sliders[2]->setValue(m_var.z);
	}

	virtual void doCommit()
	{
		m_var.x = m_sliders[0]->value();
		m_var.y = m_sliders[1]->value();
		m_var.z = m_sliders[2]->value();
	}

private:
	Point3& m_var;
};

// generic property adding functions
template <class T>
inline UIPropertyEditor<T>* property(const std::string& title, T& var, const std::string& help)
	{ return propertyTextBox<T>(title, var, help); }

// vector
template <class T>
inline UIPropertyEditor<T>* property(const std::string& title, T& var, const std::string& help, const std::vector<T>& items)
	{ return propertyCombo<T>(title, var, help, items); }

// float
inline UIPropertyEditor<float>* property(const std::string& title, float& var, const std::string& help, float min = -100, float max = 100, bool bHardLimit = false)
	{ return new UIPropertyEditorValueSlider<float>(title, var, help, true, min, max, bHardLimit); }

// int
inline UIPropertyEditor<int>* property(const std::string& title, int& var, const std::string& help, float min = -100, float max = 100, bool bHardLimit = false)
	{ return new UIPropertyEditorValueSlider<int>(title, var, help, false, min, max, bHardLimit); }

// Point2/3
inline UIPropertyEditorPoint2* property(const std::string& title, Point2& var, const std::string& help)
	{ return new UIPropertyEditorPoint2(title, var, help); }

inline UIPropertyEditorPoint3* property(const std::string& title, Point3& var, const std::string& help)
	{ return new UIPropertyEditorPoint3(title, var, help); }

// bool
template<> inline UIPropertyEditor<bool>* property(const std::string& title, bool& var, const std::string& help)
{
	std::vector<bool> items;
	items.push_back(true);
	items.push_back(false);
	return propertyCombo<bool>(title, var, help, items);
}

// sound tag
inline UIPropertyEditorSoundTag* propertySound(const std::string& title, std::string& var, const std::string& help)
{
	return new UIPropertyEditorSoundTag(title, var, help);
}

// Streamable (compound object)
template<class T> inline UIPropertyEditor<T>* property(const std::string& title, T*& var, const std::string& help)
{
	/*std::vector<std::string> rttiTypes;
	T::registrantsOfClass(T::static_rtti(), rttiTypes);
	propertyCombo<T*>(vars, title, var, rttiTypes);*/
	assert(0);
	return 0;
}

#endif
