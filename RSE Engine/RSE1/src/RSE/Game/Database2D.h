// ---------------------------------------------------------------------------------------------------------
// 
// Database2D
// 
// ---------------------------------------------------------------------------------------------------------
#pragma once



class TerrainDB;
class ObjectQuadtree;


class RSE_API Database2D
{
public:
	Database2D();
	~Database2D();

	TerrainDB& terrainDB() { return *m_pTerrainDB; }
	ObjectQuadtree& objectDB() { return *m_pObjectDB; }

private:
	TerrainDB*		m_pTerrainDB;
	ObjectQuadtree* m_pObjectDB;
};