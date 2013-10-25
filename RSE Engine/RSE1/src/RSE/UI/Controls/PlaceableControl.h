// ---------------------------------------------------------------------------------------------------------
// 
// PlaceableControl
// 
// ---------------------------------------------------------------------------------------------------------
#ifndef STANDARD_PLACEABLECONTROL_H
#define STANDARD_PLACEABLECONTROL_H

#include "RSE/UI/UIElement.h"
#include "RSE/UI/Controls/EditableProperties.h"
#include "Standard/Exception/StreamRTTI.h"

class Dialog;


class RSE_API UIEditable : public UIElement
{
	USE_RTTI(UIEditable, UIElement);
	USE_EDITSTREAM;

public:
	UIEditable();

	virtual void setName(const std::string& name) { m_name = name; }
	const std::string name() const { return m_name; }

protected:
	std::string m_name;
};

class RSE_API PlaceableControl : public UIEditable
{
	USE_RTTI(PlaceableControl, UIEditable);

public:
	Dialog& dialog() const;
	virtual Dialog* dialogIfPresent() const;

protected:
	virtual std::string styleCategoryOverride() const;
};

EXCEPTIONRTTI(RSE_API, PlaceableControl);

#endif
