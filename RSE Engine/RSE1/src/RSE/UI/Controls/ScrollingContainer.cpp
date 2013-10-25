#include "pch.h"
#include "ScrollingContainer.h"
#include "UI/Controls/UIScrollBar.h"

IMPLEMENT_RTTI(ScrollingContainer);

void ScrollingContainer::construct()
{
	Super::construct();

	_clientArea = new UIContainer;
	Super::_addChild(_clientArea);
	_clientArea->onSizeChanged.add(*this, &ScrollingContainer::onClientSizeChanged);

	_scrollHorz = new UIScrollBar;
	Super::_addChild(_scrollHorz);
	_scrollHorz->setOnPosChange(delegate(&ScrollingContainer::onScroll));
	_scrollHorz->setVisible(false);

	_scrollVert = new UIScrollBar;
	Super::_addChild(_scrollVert);
	_scrollVert->setOnPosChange(delegate(&ScrollingContainer::onScroll));
	_scrollVert->setVisible(false);
}

UIElement* ScrollingContainer::_addChild(const PtrGC<UIElement>& pElement)
{
	return _clientArea->addChild(pElement);
}

void ScrollingContainer::onScroll()
{
	_clientArea->setPos(-Point2(_scrollHorz->scrollPos(), _scrollVert->scrollPos()));
}

void ScrollingContainer::setSize(const Point2 &s)
{
	Super::setSize(s);
}

UIContainer& ScrollingContainer::clientArea()
{
	return *_clientArea;
}

void ScrollingContainer::onClientSizeChanged()
{
	Point2 scrollExtent;
	scrollExtent.x = _clientArea->size().x - size().x;
	scrollExtent.y = _clientArea->size().y - size().y;

	_scrollHorz->setVisible(scrollExtent.x > size().x);
	_scrollVert->setVisible(scrollExtent.y > size().y);

	if (_scrollVert->visible())
		_scrollHorz->setExtent(0, scrollExtent.x + _scrollVert->size().x);
	else
		_scrollHorz->setExtent(0, scrollExtent.x);

	if (_scrollHorz->visible())
		_scrollVert->setExtent(0, scrollExtent.y + _scrollHorz->size().y);
	else
		_scrollVert->setExtent(0, scrollExtent.y);
}
