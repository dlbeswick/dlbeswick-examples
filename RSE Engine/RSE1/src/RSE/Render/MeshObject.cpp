// ---------------------------------------------------------------------------------------------------------
//
// MeshObject
//
// ---------------------------------------------------------------------------------------------------------
#include "pch.h"
#include "MeshObject.h"
#include "SDeviceD3D.h"
#include <Standard/D3DHelp.h>
#include <Standard/Parser.h>
#include "Exception/Video.h"

#define FVF D3DFVF_XYZ | D3DFVF_NORMAL


// construct
MeshObject::MeshObject()
{
	m_pIBuffer = 0;
	m_pVBuffer = 0;

	zero(m_material);

	setColour(RGBA(1, 1, 1));
}


// destruct
MeshObject::~MeshObject()
{
	freeDX(m_pIBuffer);
	freeDX(m_pVBuffer);
}


// load
void MeshObject::load(itextstream& s)
{
	std::string str;
	Point3 p;

	// all whitespace matters
	s << itextstream::whitespace("");

	// process mesh file
	s >> "Mesh " >> str >> s.endl;
	setName(str);

	s >> "\tVertices" >> s.endl;

	while (s.has("\t\t"))
	{
		s >> "\t\t" >> p >> s.endl;
		m_verts.push_back(p);
	}

	s >> "\tFaces" >> s.endl;
	while (s.has("\t\t"))
	{
		s >> "\t\t" >> p >> s.endl;
		m_faces.push_back(Face((ushort)p.z, (ushort)p.y, (ushort)p.x));
	}

	s >> "\tNormals" >> s.endl;
	while (s.has("\t\t"))
	{
		s >> "\t\t" >> p >> s.endl;
		m_normals.push_back(p);
	}

	if (m_verts.size() != m_normals.size())
		throwf("Mesh load: vert and normal sizes mismatch");

	if (s.s().bad())
		throwf("Mesh load: stream fail");

	postLoadProcess();
}


// draw
void MeshObject::draw()
{
	D3DD().SetTransform(D3DTS_WORLD, worldXForm());

	DX_ENSURE(D3DD().SetTexture(0, 0));
	DX_ENSURE(D3DD().SetRenderState(D3DRS_ALPHABLENDENABLE, false));
	DX_ENSURE(D3DD().SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL));
	DX_ENSURE(D3DD().SetRenderState(D3DRS_LIGHTING, true));
	DX_ENSURE(D3DD().SetRenderState(D3DRS_COLORVERTEX, false));
	DX_ENSURE(D3DD().SetMaterial(&m_material));

	DX_ENSURE(D3DD().SetFVF(FVF));
	DX_ENSURE(D3DD().SetStreamSource(0, m_pVBuffer, 0, sizeof(VERT)));
	DX_ENSURE(D3DD().SetIndices(m_pIBuffer));
	D3DD().DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_verts.size(), 0, m_faces.size());
}

void MeshObject::postLoadProcess()
{
	// calc extents
	Point3 min(FLT_MAX, FLT_MAX, FLT_MAX);
	Point3 max(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	Point3 center(0, 0, 0);
	for (uint i = 0; i < m_verts.size(); i++)
	{
		Point3& p = m_verts[i];
		if (p.x < min.x) min.x = p.x;
		if (p.y < min.y) min.y = p.y;
		if (p.z < min.z) min.z = p.z;
		if (p.x > max.x) max.x = p.x;
		if (p.y > max.y) max.y = p.y;
		if (p.z > max.z) max.z = p.z;

		center += p;
	}

	m_extent = max - min;
	m_geomCenter = center / (float)m_verts.size();

	// setup d3d
	if (DXFAIL(D3DD().CreateVertexBuffer(m_verts.size() * sizeof(VERT), D3DUSAGE_WRITEONLY,
				FVF, D3DPOOL_DEFAULT, &m_pVBuffer, 0)))
	{
		throwf("Mesh load: vertex buffer create failed");
	}

	VERT *pBufVerts = 0;
	if (DXFAIL(m_pVBuffer->Lock(0, 0, (void**)&pBufVerts, 0)))
	{
		throwf("Mesh load: vertex buffer lock failed");
	}

	for (uint i = 0; i < m_verts.size(); i++)
	{
		pBufVerts[i].v = m_verts[i];
		pBufVerts[i].n = m_normals[i];
	}

	m_pVBuffer->Unlock();

	if (DXFAIL(D3DD().CreateIndexBuffer(m_faces.size() * sizeof(ushort) * 3, D3DUSAGE_WRITEONLY,
				D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pIBuffer, 0)))
	{
		throwf("Mesh load: index buffer create failed");
	}

	ushort *pBufIndex = 0;
	if (DXFAIL(m_pIBuffer->Lock(0, 0, (void**)&pBufIndex, 0)))
	{
		throwf("Mesh load: index buffer lock failed");
	}

	for (uint i = 0; i < m_faces.size(); i++)
	{
		*(pBufIndex++) = m_faces[i][0];
		*(pBufIndex++) = m_faces[i][1];
		*(pBufIndex++) = m_faces[i][2];
	}

	m_pIBuffer->Unlock();
}
