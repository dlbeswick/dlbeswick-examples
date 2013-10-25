// ---------------------------------------------------------------------------------------------------------
//
// MeshObject
//
// ---------------------------------------------------------------------------------------------------------
#pragma once

#include "RSE/Game/Object.h"
#include <istream>

class RSE_API MeshObject : public Object
{
public:
	MeshObject();
	~MeshObject();

	typedef std::vector<Point3> Verts;
	typedef std::vector<Point3> Normals;
	typedef std::vector<Face> Faces;

	virtual void load(itextstream& s);

	virtual void draw();

	virtual void setColour(const RGBA& col) { m_material.Diffuse.r = col.r; m_material.Diffuse.g = col.g; m_material.Diffuse.r = col.b; m_material.Diffuse.a = col.a; }

	const Verts &verts() const		{ return m_verts; }
	const Faces &faces() const		{ return m_faces; }
	const Normals &normals() const	{ return m_normals; }

	virtual Point3 extent() const { return m_extent; }
	virtual Point3 geomCenter() const { return m_geomCenter; }

protected:
	void postLoadProcess();

	struct VERT
	{
		Point3 v;
		Point3 n;
	};

	Verts					m_verts;
	Faces					m_faces;
	Normals					m_normals;
	
	D3DMATERIAL9			m_material;

	Point3					m_extent;
	Point3					m_geomCenter;

	IDirect3DVertexBuffer9	*m_pVBuffer;
	IDirect3DIndexBuffer9	*m_pIBuffer;
};