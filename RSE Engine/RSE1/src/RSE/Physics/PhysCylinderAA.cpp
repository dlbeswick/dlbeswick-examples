// ------------------------------------------------------------------------------------------------
//
// PhysCylinderAA
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"

#include "PhysCylinderAA.h"
#include "Game/Level.h"
#include "Render/D3DPainter.h"
#include "Standard/Collide.h"
#include "Terrain/TerrainDB.h"

REGISTER_RTTI(PhysCylinderAA);

// draw
void PhysCylinderAA::draw()
{
	D3DPaint().setFill(RGBA(m_material.Diffuse));
	D3DPaint().cylinder(pos(), m_radius, m_height);
}