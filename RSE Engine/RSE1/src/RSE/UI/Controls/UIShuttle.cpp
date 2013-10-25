// ------------------------------------------------------------------------------------------------
//
// UIShuttle
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "UIShuttle.h"
#include "UIButton.h"

IMPLEMENT_RTTI(UIShuttle);

UIShuttle::UIShuttle(bool defaultPlaying, const PtrGC<IShuttleNotify>& notify)
{
	m_notify = notify;

	m_first = addChild(new UIButton);
	m_first->setText("|<");
	m_first->onClick = delegate(&UIShuttle::internalOnFirst);
	m_back = addChild(new UIButton);
	m_back->setText("<");
	m_back->onClick = delegate(&UIShuttle::internalOnBack);
	m_play = addChild(new UIButton);
	m_play->setText(">>");
	m_play->onClick = delegate(&UIShuttle::internalOnPlay);
	m_pause = addChild(new UIButton);
	m_pause->setText("||");
	m_pause->onClick = delegate(&UIShuttle::internalOnPause);
	m_forward = addChild(new UIButton);
	m_forward->setText(">");
	m_forward->onClick = delegate(&UIShuttle::internalOnForward);
	m_last = addChild(new UIButton);
	m_last->setText(">|");
	m_last->onClick = delegate(&UIShuttle::internalOnLast);

	m_pause->setVisible(false);
	updateButtons(defaultPlaying);
}

void UIShuttle::setSize(const Point2& reqSize)
{
	Point2 buttonSize(reqSize.x / 5.0f, reqSize.y);
	Point2 p(0, 0);

	m_first->setPos(p);
	m_first->setSize(buttonSize);
	p.x += m_first->size().x;

	m_back->setPos(p);
	m_back->setSize(buttonSize);
	p.x += m_back->size().x;

	m_play->setPos(p);
	m_play->setSize(buttonSize);
	m_pause->setPos(p);
	m_pause->setSize(buttonSize);
	p.x += m_play->size().x;

	m_forward->setPos(p);
	m_forward->setSize(buttonSize);
	p.x += m_forward->size().x;

	m_last->setPos(p);
	m_last->setSize(buttonSize);

	Super::setSize(reqSize);
}

void UIShuttle::onButton()
{
	if (m_pause->visible())
		internalOnPause();
}

void UIShuttle::internalOnFirst()
{
	onButton();
	onFirst();
}

void UIShuttle::internalOnBack()
{
	onButton();
	onBack();
}

void UIShuttle::internalOnPlay()
{
	updateButtons(true);

	onPlay();
}

void UIShuttle::internalOnPause()
{
	updateButtons(false);

	onPause();
}

void UIShuttle::internalOnForward()
{
	onButton();
	onForward();
}

void UIShuttle::internalOnLast()
{
	onButton();
	onLast();
}

void UIShuttle::update(float delta)
{
	Super::update(delta);
	if (m_notify)
	{
		if (m_notify->shuttleIsPlaying() && !m_play->visible())
		{
			updateButtons(m_notify->shuttleIsPlaying());
		}
		else if (!m_notify->shuttleIsPlaying() && !m_pause->visible())
		{
			updateButtons(m_notify->shuttleIsPlaying());
		}
	}
}

void UIShuttle::updateButtons(bool playing)
{
	m_pause->setVisible(playing);
	m_play->setVisible(!playing);
}
