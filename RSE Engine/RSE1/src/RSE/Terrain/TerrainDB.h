// ---------------------------------------------------------------------------------------------------------
// 
// TerrainDB
// 
// ---------------------------------------------------------------------------------------------------------
#ifndef RSE_TERRAINDB_H
#define RSE_TERRAINDB_H

#include "Standard/Math.h"
class MeshObject;

class RSE_API TerrainDB
{
public:
	TerrainDB();
	~TerrainDB();

	typedef std::vector<const Tri*> PolyRefList;

	void create(MeshObject& obj);
	const Point3 &min() const	{ return m_terrainMin; }
	const Point3 &max() const	{ return m_terrainMax; }
	float cellWidth() const		{ return m_terrainCellWidth; }
	float cellHeight() const	{ return m_terrainCellHeight; }
	uint cellsX() const			{ return m_cellsX; }
	uint cellsY() const			{ return m_cellsY; }

	void polysAt(const Point3& p, PolyRefList& vec) const;
	void polysSphereSweep(const Point3& start, const Point3& vel, float radius, PolyRefList& vec) const;
	float heightAt(const Point3& p) const;

	void debugShow(class Scene& scene);

	MeshObject& mesh() const { return *m_pMeshObject; }

private:
	// terrain db is a multi-dimensional array of Tri pointer
	typedef std::vector<Tri*> TerrainList;
	typedef std::vector< std::vector< TerrainList > > DB;
	typedef std::vector<Tri> PolyList;
	DB					m_terrainDB;
	float				m_terrainCellWidth;
	float				m_terrainCellHeight;
	uint				m_cellsX;
	uint				m_cellsY;
	Point3				m_terrainMin;
	Point3				m_terrainMax;
	MeshObject*			m_pMeshObject;

	TerrainList			m_emptyTerrainList;
	PolyList			m_polys;

	// functions
	void polysFrom(const TerrainList& list, PolyRefList& vec) const;
	const TerrainList& cell(const Point3& p) const;
	void idxAt(const Point3& p, uint& x, uint& y) const;
	void cellCoords(const Point2ui& cell, Point2& min, Point2& max) const;
};

#endif