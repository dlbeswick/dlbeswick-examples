// ------------------------------------------------------------------------------------------------
//
// D3DPainter
//
// ------------------------------------------------------------------------------------------------
#ifndef RSE_D3DPAINTER_H
#define RSE_D3DPAINTER_H

#include "RSE/RSE.h"
#include "Standard/Math.h"
#include "Standard/PtrD3D.h"

class SplinePath;
class VertexBufferStream;
class Material;

namespace Interpolation
{
	class Parabola;
};

class RSE_API D3DPainter
{
public:
	struct VERT
	{
		DMath::Point3 pos;
		DMath::Point3 normal;
		D3DCOLOR c;
		DMath::Point2 tex;
	};

	Point2 uv0;
	Point2 uv1;

	D3DPainter();
	~D3DPainter();

	virtual void draw();

	void reset();

	void setFill(const RGBA& col, const PtrD3D<IDirect3DTexture9>& tex = 0, bool emissive = false);
	void setFill(const Material& col);

	void setNormal(const Point3& normal);
	void calcNormals(D3DPainter::VERT* verts, int tris);

	VertexBufferStream& vbTri();
	VertexBufferStream& vbLine();

	void tri(const Tri& p);
	void quad2D(float x0, float y0, float x1, float y1);
	void quad(const Quad& p);
	void line(const Point3& p0, const Point3& p1);
	void plane(const Plane& p, float size = 100);
	void obb(const OBB& obb, const Matrix4& xform);
	void aabb(const AABB& obb);
	void tick(const Point3& p, float mag);
	void splinePath(const SplinePath& p);
	void sphere(const Point3& p, float radius);
	void cylinder(const Point3& p, float radius, float height);
	void parabola(Interpolation::Parabola& p, uint steps=100, bool screenSpace = false);
	void projectile(Point3 p, Point3 vel, const Point3& gravity, float seconds=1.0f, uint steps=100);

protected:
	virtual void apply();
	void applyFill() const;

	class MaterialTexture* m_defaultMaterial;
	const class Material* m_fill;
	VertexBufferStream* m_vbTri;
	VertexBufferStream* m_vbLine;
	uint m_flags;
	bool m_useNormal;
	Point3 m_normal;

	D3DPainter::VERT vert(const Point3& pos, int uvIdxX, int uvIdxY);

	virtual bool needsFinish() const { return true; }

};

RSE_API class D3DPainter& D3DPaint();

////

class RSE_API DeferredPainterWorker : public D3DPainter
{
protected:
	friend class DeferredPainter;

	void finish();
	virtual bool needsFinish() const { return false; }
};

class RSE_API DeferredPainter
{
public:
	DeferredPainter(class Level& level);
	~DeferredPainter();

	D3DPainter& operator* () { return *m_worker; }
	D3DPainter* operator -> () { return m_worker; }

protected:
	DeferredPainterWorker* m_worker;
	class Level& m_level;
};

#endif
