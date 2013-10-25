// ------------------------------------------------------------------------------------------------
//
// DlgDebugLevel
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/UI/Dialogs/DlgDebug.h"

class Level;


class RSE_API DlgDebugLevel : public DlgDebug
{
	USE_RTTI(DlgDebugLevel, DlgDebug);
public:
	DlgDebugLevel();

	virtual bool mouseMove(const Point2& p, const Point2& delta);

protected:
	// fix: add once dlgdebug level member is removed
	//Level& level() const;

	PtrGC<class Object> m_hoverObject;
};