// ------------------------------------------------------------------------------------------------
//
// DlgTileEditor
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "DlgStandardLevel.h"
#include "Game/Level.h"


class DlgTileEditor : public DlgStandardLevel
{
	USE_RTTI(DlgTileEditor, DlgStandardLevel);

public:
	DlgTileEditor();

protected:
	virtual void navigation();
private:
};