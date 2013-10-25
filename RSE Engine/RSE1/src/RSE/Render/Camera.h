// ---------------------------------------------------------------------------------------------------------
// 
// Camera
// 
// ---------------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/Game/Object.h"
#include "RSE/Sound/SoundListener.h"


class RSE_API Camera : public Object, public SoundListener
{
	USE_RTTI(Camera, Object);
public:
	Camera();
	void setProjection(const Matrix4& mat);
	void setView(const Matrix4& mat)						{ m_matFinalView.setDirty(); m_matView = mat; }
	void setViewport(const Point2& min, const Point2& max);
	void setBase(const Matrix4& mat)						{ m_matFinalView.setDirty(); m_matBase = mat; }
	virtual void setPos(const Point3& p);
	virtual void setRot(const Quat& q);

	// sometimes things appear to shift down-right as coordinates that fall near the center
	// of pixels. OpenGL docs recommend shifting back by 0.375 of a pixel to solve this.
	float perPixelShift() const { return -0.0005859375f; }

	const Matrix4& projection() const	{ return m_matProj; }
	const Matrix4& invProjection() const;
	const Matrix4& view() const;
	const Matrix4& base() const			{ return m_matBase; }
	const Matrix4& finalView() const;
	const Matrix4& invFinalView() const;

	void screenToRay(const Point2& p, Point3& origin, Point3& dir);
	Point3 screenToWorld(const Point3& p) const;
	Point2 worldToScreen(const Point3& p) const;

	void draw();

	// SoundListener
	virtual Point3 listenerOrigin() const;
	virtual Point3 listenerVelocity() const;
	virtual Quat listenerRotation() const;
	virtual const Matrix4& listenerReferenceFrame() const { return m_matBase; }

	static Matrix4 matrixProjectionPerspectiveFOV(float fovYRadians, float aspect, float zNear, float zFar);

private:
	Matrix4					m_matInvProj;
	Matrix4					m_matProj;
	mutable Dirty<Matrix4>	m_matView;
	Matrix4					m_matBase;

	Point2					m_viewMin;
	Point2					m_viewMax;
	Point2					m_d3dViewMin;
	Point2					m_d3dViewMax;

	//float					m_pixelFactor;

	mutable Dirty<Matrix4>	m_matFinalView;
	mutable Dirty<Matrix4>	m_matInvFinalView;
};
