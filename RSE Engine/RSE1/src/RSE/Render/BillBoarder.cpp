// ---------------------------------------------------------------------------------------------------------
// 
// SBillBoarder
// 
// ---------------------------------------------------------------------------------------------------------
#include "pch.h"

#include "BillBoarder.h"
#include "SDeviceD3D.h"
#include "Camera.h"


// constructor
SBillBoarder::SBillBoarder() : m_pWorldCam(0)
{
}


// doXForm
void SBillBoarder::doXForm(const Point3 &translation)
{
	if (!m_pWorldCam)
		return;

//	Matrix4 world = m_pWorldCam->getWorld();
//	D3D().device().SetTransform(D3DTS_WORLD, world);
}