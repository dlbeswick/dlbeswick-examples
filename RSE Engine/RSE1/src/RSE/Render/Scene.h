// ---------------------------------------------------------------------------------------------------------
// 
// Scene
// 
// ---------------------------------------------------------------------------------------------------------
#pragma once


#include "Camera.h"


class RSE_API Scene
{
public:
	Scene();

	Camera& camera()							{ return *m_pCamera; }
	const Camera& camera() const				{ return *m_pCamera; }
	void setCamera(const PtrGC<Camera>& camera);

	void clear();
	void setClearCol(const RGBA& c)				{ m_clearCol = c; }
	void setClear(bool b)						{ m_clear = b; }

protected:
private:
	PtrGC<Camera> m_pCamera;
	RGBA		m_clearCol;
	bool		m_clear;
};