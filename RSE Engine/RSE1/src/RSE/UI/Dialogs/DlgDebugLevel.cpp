// ------------------------------------------------------------------------------------------------
//
// DlgDebugLevel
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "DlgDebugLevel.h"
#include "AppBase.h"
#include "DlgStandardLevel.h"
#include "Game/Level.h"
#include "Game/Object.h"

IMPLEMENT_RTTI(DlgDebugLevel);

DlgDebugLevel::DlgDebugLevel()
{
}

/*Level& DlgDebugLevel::level() const
{
	return Cast<DlgStandardLevel>(parent())->level();
}*/

bool DlgDebugLevel::mouseMove(const Point2& p, const Point2& delta)
{
	m_hoverObject = level->objectAtScreenPoint(clientToScreen(p));

	return true;
}