// ------------------------------------------------------------------------------------------------
//
// PhysPlane
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "PhysPlane.h"
#include "Render/D3DPainter.h"

REGISTER_RTTI(PhysPlane);

void PhysPlane::draw()
{
	Super::draw();

	D3DPaint().setFill(RGBA(m_material.Diffuse));
	D3DPaint().plane(Plane(normal(), pos()), 5.0f);
}

const Point3& PhysPlane::normal() const
{
	if (m_worldNormal.dirty())
	{
		m_worldNormal = worldXForm() * m_normal;
	}

	return *m_worldNormal;
}

Plane PhysPlane::plane() const
{
	return Plane(normal(), worldPos());
}
