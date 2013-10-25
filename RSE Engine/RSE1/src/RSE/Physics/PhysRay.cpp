// ------------------------------------------------------------------------------------------------
//
// PhysRay
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"

#include "PhysRay.h"
//#include "Game/Level.h"
#include "Render/D3DPainter.h"
//#include "Standard/Collide.h"
//#include "Terrain/TerrainDB.h"

REGISTER_RTTI(PhysRay);

// draw
void PhysRay::draw()
{
	D3DPaint().setFill(RGBA(m_material.Diffuse));
	D3DPaint().line(pos(), pos() + dir() * 10000);
}