// ---------------------------------------------------------------------------------------------------------
// 
// Camera
// 
// ---------------------------------------------------------------------------------------------------------
#include "pch.h"

#include "AppBase.h"
#include "Camera.h"
#include "SDeviceD3D.h"
#include "BillBoarder.h"
#include "UI/DialogMgr.h"
#include <Standard/Mapping.h>
#include <Standard/Rect.h>

IMPLEMENT_RTTI(Camera);

// constructor
Camera::Camera()
{
	m_matView->identity();
	setPos(Point3(0, 0, 0));
	setRot(Quat(1, Point3(0, 0, 0)));

	setProjection(matrixProjectionPerspectiveFOV(PI / 4, 640.0f / 480.0f, 1.0f, 1000.0f));

	// reference frame (MAX coord system)
	Matrix4 mat(Matrix4::IDENTITY);
	mat(0, 0) = 1; mat(0, 1) = 0; mat(0, 2) = 0;
	mat(1, 0) = 0; mat(1, 1) = 0; mat(1, 2) = 1;
	mat(2, 0) = 0; mat(2, 1) = 1; mat(2, 2) = 0;
	setBase(mat);

	setViewport(Point2(0, 0), Point2(1, 1));
};

Matrix4 Camera::matrixProjectionPerspectiveFOV(float fovYRadians, float aspect, float zNear, float zFar)
{
	assert(tan(fovYRadians / 2) != 0);

	const float yScale = 1.0f / tan(fovYRadians / 2);
	const float xScale = aspect * yScale;

	Matrix4 mat(Matrix4::IDENTITY);
	mat(0, 0) = xScale;
	mat(1, 1) = yScale;
	mat(2, 2) = zFar / (zFar - zNear);
	mat(3, 2) = -zNear * zFar / (zFar - zNear);

	return mat;
}

// setPos
void Camera::setPos(const Point3& p)
{
	m_matFinalView.setDirty();
	m_matView.setDirty();

	Super::setPos(p);
}


// setRot
void Camera::setRot(const Quat& q)
{
	m_matFinalView.setDirty();
	m_matView.setDirty();

	Super::setRot(q);
}

// draw
void Camera::draw()
{
	D3D().viewport(David::Rect(m_viewMin, m_viewMax));
	D3DD().SetTransform(D3DTS_VIEW, finalView());
	D3DD().SetTransform(D3DTS_PROJECTION, m_matProj);
}

// finalView
const Matrix4& Camera::finalView() const
{
	if (m_matFinalView.dirty())
	{
		m_matFinalView = view();
		*m_matFinalView *= base();
		m_matInvFinalView.setDirty();
	}
	
	return *m_matFinalView;
}

// invFinalView
const Matrix4& Camera::invFinalView() const
{
	if (m_matInvFinalView.dirty())
	{
		m_matInvFinalView = finalView();
		//D3DXMatrixInverse(*m_matInvFinalView, 0, *m_matInvFinalView);
		if (!m_matInvFinalView->invert())
			throwf("Couldn't invert final view matrix.");
	}

	return *m_matInvFinalView;
}

// view
const Matrix4& Camera::view() const
{
	if (m_matView.dirty())
	{
		m_matView->identity();
		m_matView->translation(-pos());

		Matrix4 rotMat;
		Quat curRot = rot();
		curRot.invert();
		curRot.toMatrix(rotMat);
		m_matView = *m_matView * rotMat;
	}
	
	return *m_matView;
}

// setProjection
void Camera::setProjection(const Matrix4& mat)
{
	m_matProj = mat;
	m_matInvProj = m_matProj;
	//D3DXMatrixInverse(m_matInvProj, 0, m_matInvProj);
	if (!m_matInvProj.invert())
		throwf("Couldn't invert projection matrix.");
}

Point2 Camera::worldToScreen(const Point3& p) const
{
	Point4 p4(p.x, p.y, p.z, 1.0f);
	p4 *= finalView();
	p4 *= projection();
	p4 *= DialogMgr().camera().invProjection();
	return Point2(p4.x / p4.w, p4.y / p4.w);
}

Point3 Camera::screenToWorld(const Point3& p) const
{
	Point4 p4;
	p4.x = p.x;
	p4.y = p.y;
	p4.z = p.z;
	p4.w = 1.0f;

	Point4 thisProjP = p4;
	thisProjP *= projection();

	p4 *= DialogMgr().camera().projection();
	p4.z = thisProjP.z;
	p4.w = thisProjP.w;
	p4.x *= p4.w;
	p4.y *= p4.w;

	// projection space
	p4 *= invProjection();

	p4 *= invFinalView();

	Point3 finalPoint(p4.x, p4.y, p4.z);
	//finalPoint *= base();
	
	return finalPoint;
}

// screenToRay
void Camera::screenToRay(const Point2& p, Point3& origin, Point3& dir)
{
	origin = screenToWorld(Point3(p.x, p.y, 0));
	dir = (screenToWorld(Point3(p.x, p.y, 1)) - origin).normal();
}

void Camera::setViewport(const Point2& min, const Point2& max)
{
	m_viewMin = min;
	m_viewMax = max;
	m_d3dViewMin = Point2(m_viewMin.x, 1.0f - m_viewMin.y) * 2.0f - Point2(1.0f, 1.0f);
	m_d3dViewMax = Point2(m_viewMax.x, 1.0f - m_viewMax.y) * 2.0f - Point2(1.0f, 1.0f);
}

Point3 Camera::listenerOrigin() const
{
	return pos();
}

Point3 Camera::listenerVelocity() const
{
	return Point3::ZERO;
}

Quat Camera::listenerRotation() const
{
	return rot();
}

const Matrix4& Camera::invProjection() const
{
	return m_matInvProj;
}
