// ------------------------------------------------------------------------------------------------
//
// DlgMessageBox
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/UI/Dialog.h"
#include "RSE/UI/Controls/UIButton.h"
#include "RSE/UI/Controls/UITextBox.h"


class RSE_API DlgMessageBox : public Dialog
{
	USE_RTTI(DlgMessageBox, Dialog);
public:
	DlgMessageBox(const std::string& title, const std::string& message, const Delegate& onOk);
	DlgMessageBox(const std::string& title, const std::string& message = "Alert");

protected:
	virtual void construct();

	virtual void onClose();

	virtual void init(const std::string& title, const std::string& message, const Delegate& onOk);

	virtual bool textBoxEnabled() const;

	Delegate m_msgBoxMethod;
	void onOkMethod()
	{
		m_msgBoxMethod();
		close();
	}

	class UIButton* m_button;
	class UITextBox* m_text;
};

template <class T>
class DlgInputBox : public DlgMessageBox
{
public:
	DlgInputBox(const std::string& title, const std::string& message, const TPDelegate<T>& onOk) :
		DlgMessageBox(title, message)
	{
		m_method = onOk;

		m_button->onClick = delegate(&DlgInputBox::onOkMethod);
		m_text->setInputEnabled(true);
		m_text->setFocus();
	}

protected:
	virtual void construct()
	{
		DlgMessageBox::construct();

		m_text->setAlign(UITextBox::LEFT);
	}

	virtual bool textBoxEnabled() const { return true; }

private:
	TPDelegate<T> m_method;

	void onOkMethod()
	{
		T var;
		itextstream s(m_text->text());
		s << itextstream::str_delimiter("");
		s >> var;
		m_method(var);

		methodCloseDialog();
	}
};

class DlgInputListBox : public DlgMessageBox
{
	USE_RTTI(DlgInputListBox, DlgMessageBox);
public:
	DlgInputListBox(const std::string& title, const std::vector<std::string>& items, const TPDelegate<std::string>& onOk);

protected:
	virtual void construct();

private:
	TPDelegate<std::string> m_method;
	class UIListView* m_list;

	void onOkMethod();
	void onDblMethod(const PtrGC<UIElement>& item) { onOkMethod(); }
};
