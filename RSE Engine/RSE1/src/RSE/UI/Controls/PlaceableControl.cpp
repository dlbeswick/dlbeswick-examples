// ------------------------------------------------------------------------------------------------
//
// PlaceableControl
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "PlaceableControl.h"
#include "UI/Dialog.h"
#include "UI/Controls/UIPropertyEditor.h"

IMPLEMENT_RTTI(PlaceableControl);

#define METADATA UIEditable
IMPLEMENT_RTTI(UIEditable);

EDITSTREAM
	EDITSTREAMVAR(m_name, "");
}

UIEditable::UIEditable()
{
}

std::string PlaceableControl::styleCategoryOverride() const
{
	Dialog* dlg = dialogIfPresent();

	if (!dlg)
	{
		return Super::styleCategoryOverride();
	}
	else
	{
		return dlg->styleNameForControl(rtti());
	}
}

Dialog* PlaceableControl::dialogIfPresent() const
{
	for (PtrGC<UIElement> parent = this->parent(); parent; parent = parent->parent())
	{
		Dialog* dlg = Cast<Dialog>(parent).ptr();

		if (dlg)
			return dlg;
	}

	return 0;
}

Dialog& PlaceableControl::dialog() const
{
	Dialog* result = dialogIfPresent();

	if (!result)
		EXCEPTIONSTREAM(ExceptionPlaceableControl(*this), "No Dialog was a parent of this control.");

	return *result;
}
