// ------------------------------------------------------------------------------------------------
//
// DlgMessageBox
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "DlgMessageBox.h"
#include "UI/Controls/UIListView.h"

IMPLEMENT_RTTI(DlgMessageBox);


DlgMessageBox::DlgMessageBox(const std::string& title, const std::string& message, const Delegate& onOk)
{
	m_msgBoxMethod = onOk;
    init(title, message, delegate(&DlgMessageBox::onOkMethod));
}

DlgMessageBox::DlgMessageBox(const std::string& title, const std::string& message)
{
    init(title, message, delegate(&DlgMessageBox::onOkMethod));
}

void DlgMessageBox::construct()
{
	Super::construct();
	
	addChild(m_button);
	addChild(m_text);

	setSizeForClient(Point2(0.5f, 0.25f));
	align(ALIGN_CENTER, VALIGN_CENTER, parent());

	m_button->setSize(Point2(0.1f, 0.05f));
	m_button->setPos(Point2(0, clientSize().y - m_button->size().y));
	align(ALIGN_CENTER, VALIGN_NONE, parent());

	m_text->setWrap(true);
	m_text->setEnabled(textBoxEnabled());
	m_text->setAlign(UITextBox::CENTER);
	m_text->setVAlign(UITextBox::VCENTER);
	m_text->setColour(RGBA(0,0,0,0));
	m_text->setSize(Point2(clientSize().x, clientSize().y - m_button->size().y));
}

bool DlgMessageBox::textBoxEnabled() const
{
	return false;
}

void DlgMessageBox::onClose()
{
	m_msgBoxMethod();
}

void DlgMessageBox::init(const std::string& title, const std::string& message, const Delegate& onOk)
{
	setTitle(title);

	m_button = new UIButton;
	m_button->setText("Ok");
	m_button->onClick = onOk;

	m_text = new UITextBox;
	m_text->setText(message);
}

// DlgInputListBox
IMPLEMENT_RTTI(DlgInputListBox);

DlgInputListBox::DlgInputListBox(const std::string& title, const std::vector<std::string>& items, const TPDelegate<std::string>& onOk) :
	DlgMessageBox(title, "")
{
	m_method = onOk;

	m_button->onClick = delegate(&DlgInputListBox::onOkMethod);
	m_list = new UIListView;
	m_list->onDoubleClick = delegate(&DlgInputListBox::onDblMethod);
	m_list->setFont("smallarial");
	
	for (uint i = 0; i < items.size(); ++i)
	{
		m_list->add(new ListText(items[i]));
	}
}

void DlgInputListBox::construct()
{
	Super::construct();

	addChild(m_list);
	m_list->setSize(Point2(size().x, clientArea().size().y - m_button->size().y));
}

void DlgInputListBox::onOkMethod()
{
	m_method(m_list->selected()->text());
	self().destroy();
}

