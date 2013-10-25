// ------------------------------------------------------------------------------------------------
//
// D3DPainter
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "D3DPainter.h"
#include "SDeviceD3D.h"
#include "VertexBuffer.h"
#include "FontElement.h"
#include "Game/Level.h"
#include "Materials/MaterialTexture.h"
#include "Standard/Interpolation.h"
#include "Standard/Spline.h"

class D3DPainter& D3DPaint() { return *SDeviceD3D::instance().painter; }
static const int FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0) | D3DFVF_DIFFUSE;

D3DPainter::D3DPainter()
{
	m_defaultMaterial = new MaterialTexture;

	m_flags = D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC;
	
	m_vbLine = 0;
	m_vbTri = 0;
	
	reset();
}

VertexBufferStream& D3DPainter::vbTri()
{
	if (!m_vbTri)
		m_vbTri = new VertexBufferStream(sizeof(VERT), FVF, 1024 * sizeof(VERT), m_flags);

	return *m_vbTri;
}

VertexBufferStream& D3DPainter::vbLine()
{
	if (!m_vbLine)
		m_vbLine = new VertexBufferStream(sizeof(VERT), FVF, 1024 * sizeof(VERT), m_flags);

	return *m_vbLine;
}

D3DPainter::~D3DPainter()
{
	reset();
	delete m_vbLine;
	delete m_vbTri;
	delete m_defaultMaterial;
}

void D3DPainter::reset()
{
	setFill(RGBA(1,1,1));
	uv0 = Point2::ZERO;
	uv1 = Point2(1, 1);
	m_useNormal = false;
}

void D3DPainter::setFill(const RGBA& col, const PtrD3D<IDirect3DTexture9>& tex, bool emissive)
{
	m_defaultMaterial->diffuse = col;

	if (emissive)
		m_defaultMaterial->emissive = col;
	else
		m_defaultMaterial->emissive = RGBA(0,0,0);

	m_defaultMaterial->texture = tex;

	// tbd: don't do this here. it creates hidden behaviour mandating uv changes after setFill, which is not strictly necessary.
	uv0 = Point2::ZERO;
	uv1 = Point2(1,1);
	setFill(*m_defaultMaterial);
}

void D3DPainter::setFill(const Material& col)
{
	m_fill = &col;
}

D3DPainter::VERT D3DPainter::vert(const Point3& pos, int uvIdxX, int uvIdxY)
{
	D3DPainter::VERT v;
	v.pos = pos;
	if (uvIdxX == 0)
		v.tex.x = uv0.x;
	else
		v.tex.x = uv1.x;
	if (uvIdxY == 0)
		v.tex.y = uv0.y;
	else
		v.tex.y = uv1.y;
	v.c = RGBA(1,1,1);
	
	return v;
}

// line
void D3DPainter::line(const Point3 &p0, const Point3 &p1)
{
	VERT verts[2] = {
		vert(p0, 0, 0),
		vert(p1, 1, 1),
	};

	vbLine().insert(verts, 2);
}


// tri
void D3DPainter::tri(const Tri& p)
{
	VERT verts[3] = {
		vert(Point3(p[0]), 0, 0),
		vert(Point3(p[1]), 1, 1),
		vert(Point3(p[2]), 0, 1),
	};

	calcNormals(verts, 1);
	vbTri().insert(verts, 3);
}


// quad
void D3DPainter::quad(const Quad& p)
{
	VERT verts[6] = {
		vert(Point3(p[0]), 0, 0),
		vert(Point3(p[1]), 1, 0),
		vert(Point3(p[2]), 1, 1),
		vert(Point3(p[0]), 0, 0),
		vert(Point3(p[2]), 1, 1),
		vert(Point3(p[3]), 0, 1),
	};

	calcNormals(verts, 2);
	vbTri().insert(verts, 6);
}

void D3DPainter::quad2D(float x0, float y0, float x1, float y1)
{
	const float z = 1.0f;
	quad(Quad(Point3(x0, y0, z), Point3(x1, y0, z), Point3(x1, y1, z), Point3(x0, y1, z)));
}

// plane
void D3DPainter::plane(const Plane& p, float size)
{
	Point3 origin = p.n * p.d;

	Point3 x(1, 0, 0);
	Point3 y(0, 1, 0);
	Point3 z(0, 0, 1);

	Quat baseToPlane(QuatFromVector(Point3(0, 0, 1), p.n));

	x = baseToPlane * x;
	y = baseToPlane * y;
	z = baseToPlane * z;

	D3DD().SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	Quad corner;

	corner[0] = origin + (-x + y) * size;
	corner[1] = origin + (x + y) * size;
	corner[2] = origin + (x - y) * size;
	corner[3] = origin + (-x - y) * size;

	quad(corner);

	/*line(corner[0], corner[1], col);
	line(corner[1], corner[2], col);
	line(corner[2], corner[3], col);
	line(corner[3], corner[0], col);*/

	D3DD().SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

void D3DPainter::aabb(const AABB& aabb)
{
	Point3 e = aabb.extent * 0.5f;

	VERT verts[8] = {
		vert(Point3(e.x, e.y, -e.z) + aabb.origin, 0, 0),
		vert(Point3(-e.x, e.y, -e.z) + aabb.origin, 0, 0),
		vert(Point3(-e.x, e.y, e.z) + aabb.origin, 0, 0),
		vert(Point3(e.x, e.y, e.z) + aabb.origin, 0, 0),
		vert(Point3(e.x, -e.y, -e.z) + aabb.origin, 0, 0),
		vert(Point3(-e.x, -e.y, -e.z) + aabb.origin, 0, 0),
		vert(Point3(-e.x, -e.y, e.z) + aabb.origin, 0, 0),
		vert(Point3(e.x, -e.y, e.z) + aabb.origin, 0, 0),
	};

	ushort idxs[36] = {
		2, 1, 0, // rear
		0, 3, 2,
		4, 5, 6, // front
		6, 7, 4,
		4, 0, 1, // bottom
		1, 5, 4,
		2, 3, 7, // top
		7, 6, 2,
		1, 2, 6, // left
		6, 5, 1,
		7, 3, 0, // right
		0, 4, 7
	};

//	calcNormals(verts, 12);
	vbTri().ensure(36);
	for (uint i = 0; i < 36; ++i)
		vbTri().insert(verts + idxs[i], 1);

	line(aabb.origin + Point3(0, 0, -5), aabb.origin + Point3(0, 0, 5));
	line(aabb.origin + Point3(-5, 0, 0), aabb.origin + Point3(5, 0, 0));
	line(aabb.origin + Point3(0, -5, 0), aabb.origin + Point3(0, 5, 0));
}

// obb
void D3DPainter::obb(const OBB& obb, const Matrix4& xform)
{
	Point3 e = obb.extent * 0.5f;

	VERT verts[8] = {
		vert(Point3(e.x, e.y, -e.z), 0, 0),
		vert(Point3(-e.x, e.y, -e.z), 0, 0),
		vert(Point3(-e.x, e.y, e.z), 0, 0),
		vert(Point3(e.x, e.y, e.z), 0, 0),
		vert(Point3(e.x, -e.y, -e.z), 0, 0),
		vert(Point3(-e.x, -e.y, -e.z), 0, 0),
		vert(Point3(-e.x, -e.y, e.z), 0, 0),
		vert(Point3(e.x, -e.y, e.z), 0, 0),
	};

	// transform verts
	for (uint i = 0; i < 8; i++)
		verts[i].pos *= xform;

	ushort idxs[36] = {
		2, 1, 0, // rear
		0, 3, 2,
		4, 5, 6, // front
		6, 7, 4,
		4, 0, 1, // bottom
		1, 5, 4,
		2, 3, 7, // top
		7, 6, 2,
		1, 2, 6, // left
		6, 5, 1,
		7, 3, 0, // right
		0, 4, 7
	};

//	calcNormals(verts, 12);
//	vbTri().ensure(36);
	for (uint i = 0; i < 36; ++i)
		vbTri().insert(verts + idxs[i], 1);

	tick(obb.origin, 5.0f);
}

// tick
void D3DPainter::tick(const Point3& p, float mag)
{
	mag *= 0.5f;

	line(p + Point3(0, 0, -mag), p + Point3(0, 0, mag));
	line(p + Point3(-mag, 0, 0), p + Point3(mag, 0, 0));
	line(p + Point3(0, -mag, 0), p + Point3(0, mag, 0));
}

// splinePath
void D3DPainter::splinePath(const SplinePath& p)
{
	// draw spline
	int steps = 10 * p.splineSize();
	float delta = 1.0f / steps;
	float t = 0;
	Point3 prevPoint = p.eval(t);
	Point3 nextPoint;

	t += delta;
	for (int i = 1; i < steps; i++)
	{
		nextPoint = p.eval(t);
		line(prevPoint, nextPoint);

		prevPoint = nextPoint;
		t += delta;
	}
}

static const int SPHERESEGS = 8;

// sphere
void D3DPainter::sphere(const Point3& p, float radius)
{
	// make verts

	const int numVerts = SPHERESEGS * SPHERESEGS;
	VERT verts[numVerts];

	VERT* curVert = verts;
	for (int i = 0; i < SPHERESEGS; i++)
	{
		float v = (float)i / SPHERESEGS;

		for (int j = 0; j < SPHERESEGS; j++)
		{
			float u = (float)j / SPHERESEGS;

			*(curVert++) = vert(Point3(p.x + radius * sin(v * PI) * cos(u * 2 * PI),
										p.y + radius * sin(v * PI) * sin(u * 2 * PI),
										p.z + radius * cos(v * PI)), 0, 0);
		}
	}

	const int numTris = (SPHERESEGS - 1) * (SPHERESEGS) * 2;
	ushort idxs[numTris * 3];
	ushort* x = idxs;
	for (int i = 0; i < SPHERESEGS - 1; ++i)
	{
		for (int j = 0; j < SPHERESEGS - 1; ++j)
		{
			*(x++) = i * SPHERESEGS + (j + 1);
			*(x++) = (i + 1) * SPHERESEGS + (j + 1);
			*(x++) = i * SPHERESEGS + j;
			*(x++) = (i + 1) * SPHERESEGS + (j + 1);
			*(x++) = (i + 1) * SPHERESEGS + j;
			*(x++) = i * SPHERESEGS + j;
		}

		*(x++) = i * SPHERESEGS + 0;
		*(x++) = (i + 1) * SPHERESEGS + 0;
		*(x++) = i * SPHERESEGS * 2;
		*(x++) = (i + 1) * SPHERESEGS + 0;
		*(x++) = (i + 1) * SPHERESEGS * 2;
		*(x++) = i * SPHERESEGS * 2;
	}

//	calcNormals(verts, numTris);
	vbTri().ensure(numTris * 3);
	for (uint i = 0; i < numTris * 3; ++i)
		vbTri().insert(verts + idxs[i], 1);
}

void D3DPainter::cylinder(const Point3& p, float radius, float height)
{
	// make verts

	const int numVerts = SPHERESEGS * 2 + 2;
	VERT verts[numVerts];

	float z = p.z + height * 0.5f;

	VERT* curVert = verts;
	for (int i = 0; i < 2; ++i)
	{
		for (int j = 0; j < SPHERESEGS; ++j)
		{
			float u = (float)j / SPHERESEGS;

			*(curVert++) = vert(Point3(p.x + radius * sin(u * 2 * PI),
											p.y + radius * cos(u * 2 * PI),
											z), 0, 0);
		}

		z -= height;
	}

	*(curVert++) = vert(Point3(p.x, p.y, p.z + height * 0.5f), 0, 0);
	*(curVert++) = vert(Point3(p.x, p.y, p.z - height * 0.5f), 0, 0);

	int topVert = curVert - verts - 2;
	int bottomVert = curVert - verts - 1;

	const int numTris = SPHERESEGS * 2 + SPHERESEGS * 2;
	ushort idxs[numTris * 3];
	ushort* x = idxs;
	// make caps
	for (int i = 0; i < SPHERESEGS - 1; ++i)
	{
		*(x++) = i;
		*(x++) = i + 1;
		*(x++) = topVert;
	}

	for (int i = 0; i < SPHERESEGS - 1; ++i)
	{
		*(x++) = (i + SPHERESEGS);
		*(x++) = (i + SPHERESEGS + 1);
		*(x++) = bottomVert;
	}

	// make body
	for (int i = 0; i < SPHERESEGS - 1; ++i)
	{
		*(x++) = i;
		*(x++) = i + 1;
		*(x++) = i + 1 + SPHERESEGS;
		*(x++) = i;
		*(x++) = i + SPHERESEGS + 1;
		*(x++) = i + SPHERESEGS;
	}

	for (uint i = 0; i < numTris * 3; ++i)
		vbTri().insert(verts + idxs[i], 1);
}

// apply
void D3DPainter::applyFill() const
{
	if (m_fill)
		m_fill->apply();
}

void D3DPainter::apply()
{
	D3DD().SetFVF(FVF);
	D3DD().SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	D3DD().SetRenderState(D3DRS_LIGHTING, true);
	D3DD().SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	D3DD().SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	D3DD().SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	D3DD().SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	D3DD().SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_CURRENT);
	D3DD().SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	D3DD().SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	D3DD().SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
	D3DD().SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);

	applyFill();
	//border.apply(*this);
}

void D3DPainter::draw()
{
	apply();

	if (m_vbTri)
	{
		if (needsFinish())
			m_vbTri->finish();
		for (VertexBufferStream::Buffers::const_iterator i = m_vbTri->begin(); i != m_vbTri->renderEnd(); ++i)
		{
			assert((i->finalUtilised / sizeof(VERT)) % 3 == 0);

			D3DD().SetStreamSource(0, i->buffer->buffer(), 0, sizeof(VERT));
			D3DD().DrawPrimitive(D3DPT_TRIANGLELIST, 0, i->finalUtilised / sizeof(VERT) / 3);
		}
	}

	if (m_vbLine)
	{
		if (needsFinish())
			m_vbLine->finish();
		for (VertexBufferStream::Buffers::const_iterator i = m_vbLine->begin(); i != m_vbLine->renderEnd(); ++i)
		{
			assert((i->finalUtilised / sizeof(VERT)) % 2 == 0);

			D3DD().SetStreamSource(0, i->buffer->buffer(), 0, sizeof(VERT));
			D3DD().DrawPrimitive(D3DPT_LINELIST, 0, i->finalUtilised / sizeof(VERT) / 2);
		}
	}
}

void D3DPainter::calcNormals(D3DPainter::VERT* verts, int tris)
{
	Point3 normal;

	if (m_useNormal)
		normal = m_normal;

	for (int i = 0; i < tris * 3; i+=3)
	{
		if (!m_useNormal)
			normal = (verts[0].pos - verts[1].pos).rCross(verts[2].pos - verts[1].pos).normal();

		verts[i].normal = normal;
		verts[i+1].normal = normal;
		verts[i+2].normal = normal;
	}
}

void D3DPainter::setNormal(const Point3& normal)
{
	m_useNormal = true;
	m_normal = normal;
}

void D3DPainter::parabola(Interpolation::Parabola& p, uint steps, bool screenSpace)
{
	float delta = 1.0f/steps;
	Point3 last;

	for (float t = 0.0f; t < 1.0f; t += delta)
	{
		Point3 point = p(t);

		if (screenSpace)
		{
			std::swap(point.y, point.z);
		}

		if (t)
			line(last, point);

		last = point;
	}
}

void D3DPainter::projectile(Point3 p, Point3 vel, const Point3& gravity, float seconds, uint steps)
{
	float delta = seconds/steps;
	Point3 last;

	for (float t = 0; t < seconds; t += delta)
	{
		if (t)
			line(last, p);
		last = p;
		p += vel * delta;
		vel += gravity * delta;
	}
}

////

DeferredPainter::DeferredPainter(class Level& level) :
	m_level(level)
{
	m_worker = new DeferredPainterWorker;
}

DeferredPainter::~DeferredPainter()
{
	m_worker->finish();
	m_level.addDeferredPainter(*m_worker);
}

void DeferredPainterWorker::finish()
{
	if (m_vbLine)
		m_vbLine->finish();

	if (m_vbTri)
		m_vbTri->finish();
}
