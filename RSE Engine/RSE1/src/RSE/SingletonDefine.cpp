// ---------------------------------------------------------------------------------------------------------
// 
// SingletonDefine
// 
// ---------------------------------------------------------------------------------------------------------
#include "pch.h"

#include "Render/BillBoarder.h"
#include "Render/Camera.h"
#include "Render/SDeviceD3D.h"
#include "Render/SFont.h"
#include "Render/MeshObject.h"
#include "UI/DialogMgr.h"
#include "Standard/DXInput.h"
#include "Standard/Profiler.h"

#ifdef USE_PTR_SMARTCHECKS
	IMPLEMENT_PTR_SMARTCHECKS;
#endif

static SProfiler profiler;

static SFont font;
static SDeviceD3D device;
static DXInput input;
static SBillBoarder billBoarder;