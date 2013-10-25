#ifndef RSE1_SCROLLINGCONTAINER_H
#define RSE1_SCROLLINGCONTAINER_H

#include "RSE/UI/UIElement.h"

class ScrollingContainer : public UIContainer
{
	USE_RTTI(ScrollingContainer, UIContainer);
public:
	virtual UIElement* _addChild(const PtrGC<UIElement>& pElement);
	virtual void setSize(const Point2 &s);

	virtual UIContainer& clientArea();

protected:
	virtual void construct();

	virtual void onClientSizeChanged();
	virtual void onScroll();

	class UIContainer* _clientArea;
	class UIScrollBar* _scrollHorz;
	class UIScrollBar* _scrollVert;
};

#endif
