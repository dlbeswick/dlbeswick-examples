// ---------------------------------------------------------------------------------------------------------
// 
// TerrainDB
// 
// ---------------------------------------------------------------------------------------------------------
#include "pch.h"

#include "TerrainDB.h"
#include "Render/D3DPainter.h"
#include "Render/MeshObject.h"
#include "Render/Scene.h"
#include "Render/SDeviceD3D.h"
#include "UI/DialogMgr.h"
#include <Standard/Collide.h>
#include <Standard/DXInput.h>
#include <Standard/MathHelp.h>


// construct
TerrainDB::TerrainDB() :
	m_pMeshObject(0),
	m_cellsX(0),
	m_cellsY(0)
{
	m_emptyTerrainList.push_back(0);
}

// destruct
TerrainDB::~TerrainDB()
{
}

// create
void TerrainDB::create(MeshObject& obj)
{
	m_pMeshObject = &obj;

	const MeshObject::Verts &verts = obj.verts();
	const MeshObject::Faces &faces = obj.faces();

	// generate triangles from the mesh
	for (uint i = 0; i < faces.size(); ++i)
	{
		m_polys.push_back(Tri(verts[faces[i][0]], verts[faces[i][1]], verts[faces[i][2]]));
	}

	// 1. determine the extents of the terrain mesh
	m_terrainMin(FLT_MAX, FLT_MAX, FLT_MAX);
	m_terrainMax = -m_terrainMin;

	for (uint i = 0; i < verts.size(); ++i)
	{
		const Point3 &p = verts[i];

		m_terrainMin.x = std::min(m_terrainMin.x, p.x);
		m_terrainMin.y = std::min(m_terrainMin.y, p.y);
		m_terrainMin.z = std::min(m_terrainMin.z, p.z);

		m_terrainMax.x = std::max(m_terrainMax.x, p.x);
		m_terrainMax.y = std::max(m_terrainMax.y, p.y);
		m_terrainMax.z = std::max(m_terrainMax.z, p.z);
	}

	// 2. determine how big each 2D cell should be
	// refine?
	m_terrainCellWidth = 25;
	m_terrainCellHeight = 25;

	float terrainXSize = m_terrainMax.x - m_terrainMin.x;
	float terrainYSize = m_terrainMax.y - m_terrainMin.y;

	// 3. size DB accordingly
	m_cellsX = (uint)ceil(terrainXSize / m_terrainCellWidth);
	m_cellsY = (uint)ceil(terrainYSize / m_terrainCellHeight);
	m_terrainDB.resize(m_cellsY);
	for (uint i = 0; i < m_terrainDB.size(); i++)
	{
		m_terrainDB[i].resize(m_cellsX);
	}

	// 4. classify each face of the terrain mesh into a cell or cells
	for (uint i = 0; i < m_polys.size(); ++i)
	{
		Tri& tri = m_polys[i];
		Point2 min(FLT_MAX, FLT_MAX);
		Point2 max(-FLT_MAX, -FLT_MAX);

		for (uint j = 0; j < 3; j++)
		{
			const Point3& v = tri[j];

			min.x = std::min(min.x, v.x);
			min.y = std::min(min.y, v.y);
			max.x = std::max(max.x, v.x);
			max.y = std::max(max.y, v.y);
		}

		Point3 points[4] = {Point3(min.x, min.y, 0), Point3(max.x, min.y, 0), 
							Point3(max.x, max.y, 0), Point3(min.x, max.y, 0)};

		for (uint j = 0; j < 4; ++j)
		{
			Point3 triMin, triMax;
			uint targetX0, targetX1, targetY0, targetY1;

			triMin = MathHelp::minPoint(points, points + 4);
			triMax = MathHelp::maxPoint(points, points + 4);

			idxAt(triMin, targetX0, targetY0);
			idxAt(triMax, targetX1, targetY1);
			assert(targetX0 < m_terrainDB[0].size());
			assert(targetY0 < m_terrainDB.size());
			assert(targetX1 < m_terrainDB[0].size());
			assert(targetY1 < m_terrainDB.size());

			for (uint y = targetY0; y <= targetY1; ++y)
			{
				for (uint x = targetX0; x <= targetX1; ++x)
				{
					TerrainList& l = m_terrainDB[y][x];

					if (std::find(l.begin(), l.end(), &tri) == l.end())
						l.push_back(&tri);
				}
			}
		}
	}
}

// debugShow
void TerrainDB::debugShow(Scene& scene)
{
	if (!m_pMeshObject)
		return;

	// show quadtree divisions
	Point3 p1;
	Point3 p2;

	Matrix4 id;
	id.identity();
	D3D().device().SetTransform(D3DTS_WORLDMATRIX(0), id);

	p1.z = m_terrainMin.z;
	p2.z = m_terrainMin.z;

	D3DPaint().setFill(RGBA(0,1,0,1));

	for (uint y = 0; y < m_cellsY; y++)
	{
		p1.x = m_terrainMin.x;
		p2.x = m_terrainMax.x;
		p1.y = m_terrainMin.y + m_terrainCellHeight * y;
		p2.y = p1.y;
		D3DPaint().line(p1, p2);
	}

	for (uint x = 0; x < m_cellsX; x++)
	{
		p1.y = m_terrainMin.y;
		p2.y = m_terrainMax.y;
		p1.x = m_terrainMin.x + m_terrainCellHeight * x;
		p2.x = p1.x;
		D3DPaint().line(p1, p2);
	}

	// show polys contained in cell currently pointed to by the mouse
	Point3 origin, dir, hit;
	
	scene.camera().screenToRay(DialogMgr().pointerPos(), origin, dir);

	Plane pl(Point3(0, 0, 1), -m_terrainMin.z);
	if (Collide::rayPlane(origin, dir, pl, hit))
	{
		PolyRefList list;
		polysAt(hit, list);

		for (uint i = 0; i < list.size(); i++)
		{
			if (Collide::rayPolygon(*list[i], origin, dir, hit))
			{
				D3DPaint().setFill(RGBA(1, 0, 0));
				D3DPaint().tri(*list[i]);
			}
			else
			{
				D3DPaint().setFill(RGBA(0.5f, 0, 0));
				D3DPaint().tri(*list[i]);
			}
		}
	}
}

// heightAt
float TerrainDB::heightAt(const Point3& p) const
{
	float height = -FLT_MAX;
	Point3 start(p);
	Point3 dir(0, 0, -1);
	Point3 hit;

	start.z = 99999;

	PolyRefList vec;
	polysAt(p, vec);

	for (uint i = 0; i < vec.size(); i++)
	{
		const Tri& t = *vec[i];

		if (Collide::rayPolygon(t, start, dir, hit))
		{
			height = std::max(hit.z, height);
		}
	}

	if (height == -FLT_MAX)
		height = 0;

	return height;
}

// cell
const TerrainDB::TerrainList& TerrainDB::cell(const Point3& p) const
{
	uint cellX;
	uint cellY;

	idxAt(p, cellX, cellY);

	if (cellX >= cellsX() || cellY >= cellsY())
		return m_emptyTerrainList;

	return m_terrainDB[cellY][cellX];
}

// idxAt
void TerrainDB::idxAt(const Point3& p, uint& x, uint& y) const
{
	Point3 origin = p - min();
	x = (uint)((int)origin.x / cellWidth());
	y = (uint)((int)origin.y / cellHeight());

	clamp(x, 0U, cellsX() - 1);
	clamp(y, 0U, cellsY() - 1);
}

// polysFrom
void TerrainDB::polysFrom(const TerrainList& list, PolyRefList& vec) const
{
	for (uint i = 0; i < list.size(); i++)
	{
		vec.push_back(list[i]);
	}
}

// cellCoords
// world coordinates of the cell
void TerrainDB::cellCoords(const Point2ui& cell, Point2& min, Point2& max) const
{
	min.x = m_terrainMin.x + m_terrainCellWidth * cell.x;
	min.y = m_terrainMin.y + m_terrainCellWidth * cell.y;
	max.x = min.x + m_terrainCellWidth;
	max.y = min.y + m_terrainCellWidth;
}

// polysAt
void TerrainDB::polysAt(const Point3& p, PolyRefList& vec) const
{
	const TerrainList& list = cell(p);
	
	polysFrom(list, vec);
}

// polysSphereSweep
void TerrainDB::polysSphereSweep(const Point3& start, const Point3& vel, float radius, TerrainDB::PolyRefList& vec) const
{
	Point3 end = start + vel;

    // first pass, get rectangular extent of sweep
	Point3 pRad(radius, radius, radius);
	Point3 extent[4] = { start + pRad, start - pRad, end + pRad, end - pRad };
	
	extent[0].z = extent[1].z = extent[2].z = extent[3].z = 0;

	Point3 min, max;

	min = MathHelp::minPoint(extent, extent + 4);
	max = MathHelp::maxPoint(extent, extent + 4);
	
	uint minIdxX, minIdxY, maxIdxX, maxIdxY;
	idxAt(min, minIdxX, minIdxY);
	idxAt(max, maxIdxX, maxIdxY);

	for (uint y = minIdxY; y <= maxIdxY; ++y)
	{
		for (uint x = minIdxX; x <= maxIdxX; ++x)
		{
			polysFrom(m_terrainDB[y][x], vec);
		}
	}
}
