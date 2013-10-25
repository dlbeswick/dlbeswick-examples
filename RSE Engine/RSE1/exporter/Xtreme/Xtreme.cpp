// ---------------------------------------------------------------------------------------------------------
// 
// Xtreme
// 
// ---------------------------------------------------------------------------------------------------------
#include "stdafx.h"
#pragma hdrstop

#include "XtremeExport.h"


// Class description
class XtremeClassDesc : public ClassDesc
{
public:
	int 			IsPublic() { return TRUE; }
	void * 			Create(BOOL loading=FALSE) { return new XtremeExport; }
	const TCHAR * 	ClassName() { return _T("David XTreme Exporter"); }
	SClass_ID 		SuperClassID() { return SCENE_EXPORT_CLASS_ID; }
	Class_ID 		ClassID() { return Class_ID(0x4d97c83, 0x7e8a0ed2); }
	const TCHAR* 	Category() { return _T("XXXXTreme");  }
};

static XtremeClassDesc XtremeCD;


// LibNumberClasses
DLL int LibNumberClasses() { return 1; }


// LibClassDesc
DLL ClassDesc *LibClassDesc(int i)
{
	return &XtremeCD;
}


// LibDescription
DLL const TCHAR *LibDescription()
{
#ifdef _DEBUG
	return _T("David Export To The XXXtreme (DEBUG)");
#else
	return _T("David Export To The XXXtreme");
#endif
}


// LibVersion
DLL ULONG LibVersion() { return VERSION_3DSMAX; }


int bControlsInit = FALSE;
HINSTANCE hInstance;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, ULONG fdwReason, LPVOID lpvReserved)
{	
	// Hang on to this DLL's instance handle.
	hInstance = hinstDLL;
	if (!bControlsInit)
	{
		bControlsInit = TRUE;
	
		// Initialize MAX's custom controls
		InitCustomControls(hInstance);
	
		// Initialize Win95 controls
		InitCommonControls();
	}
	
	return(TRUE);
}