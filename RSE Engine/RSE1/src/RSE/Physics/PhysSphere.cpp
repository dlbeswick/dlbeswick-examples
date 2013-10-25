// ------------------------------------------------------------------------------------------------
//
// PhysSphere
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"

#include "PhysSphere.h"
#include "Game/Level.h"
#include "Render/D3DPainter.h"
#include <Standard/Config.h>
#include "Standard/Collide.h"
#include "Terrain/TerrainDB.h"

REGISTER_RTTI(PhysSphere);

// draw
void PhysSphere::draw()
{
	Super::draw();

	D3DPaint().setFill(RGBA(m_material.Diffuse));
	D3DPaint().sphere(worldPos(), m_radius);
	D3DPaint().draw();
}

// getTriList
void PhysSphere::getTriList(const Point3& pos, const Point3& vel, TriRefList& list)
{
	level().database2D().terrainDB().polysSphereSweep(pos, vel, m_radius, list);
}

// collideTri
bool PhysSphere::collideTri(const Point3& pos, const Point3& vel, const Tri& t, float &u)
{
	return Collide::sphereTriSweep(pos, vel, m_radius, t, u);
}
