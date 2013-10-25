// ---------------------------------------------------------------------------------------------------------
// 
// XtremeExport
// 
// ---------------------------------------------------------------------------------------------------------
#pragma once

#include "stdafx.h"


class XtremeExport : public SceneExport
{
public:
	virtual int ExtCount() { return 1; }
	virtual const TCHAR* Ext(int i) { return "!x"; }
	virtual const TCHAR* LongDesc() { return "XXXXXXtreme David File"; }
#ifdef _DEBUG
	virtual const TCHAR* ShortDesc() { return "XXXXXtreme David File (DEBUG)"; }
#else
	virtual const TCHAR* ShortDesc() { return "XXXXXtreme David File"; }
#endif
	virtual const TCHAR* AuthorName() { return "David"; }
	virtual const TCHAR* CopyrightMessage() { return "8==D"; }
	virtual const TCHAR* OtherMessage1() { return "(_*_)"; }
	virtual const TCHAR* OtherMessage2() { return "(.Y.)"; }
	virtual uint Version() { return 1; }
	virtual void ShowAbout(HWND hWnd) { MessageBox(hWnd, "Don't be a passenger", "Life is a highway", 0); }

	virtual int DoExport(const TCHAR *name, ExpInterface *ei, Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0);
	virtual BOOL SupportsOptions(int ext, DWORD options) { return options | SCENE_EXPORT_SELECTED; }
};
