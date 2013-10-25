// ---------------------------------------------------------------------------------------------------------
// 
// Database2D
// 
// ---------------------------------------------------------------------------------------------------------
#include "pch.h"

#include "Database2D.h"
#include "ObjectQuadtree.h"
#include "RSE/Terrain/TerrainDB.h"


// construct
Database2D::Database2D() :
	m_pTerrainDB(new TerrainDB),
	m_pObjectDB(new ObjectQuadtree(Point2(-1000, -1000), Point2(1000, 1000)))
{
}


// destruct
Database2D::~Database2D()
{
	freePtr(m_pTerrainDB);
	freePtr(m_pObjectDB);
}
