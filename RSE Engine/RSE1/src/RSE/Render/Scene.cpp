// ---------------------------------------------------------------------------------------------------------
// 
// Scene
// 
// ---------------------------------------------------------------------------------------------------------
#include "pch.h"

#include "AppBase.h"
#include "Scene.h"
#include "BillBoarder.h"
#include "Render/SDeviceD3D.h"
#include "Sound/ISoundProvider.h"

// construct
Scene::Scene()
{
	m_pCamera = new Camera;
	m_clearCol = RGBA(0,0,1);
	m_clear = true;
}

// setCamera
void Scene::setCamera(const PtrGC<Camera>& camera)
{
	m_pCamera = camera;
	BillBoarder().setWorldCam(m_pCamera.ptr());
}

void Scene::clear()
{
	if (m_clear)
		D3DD().Clear( 0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, m_clearCol, 1.0f, 0 );
}