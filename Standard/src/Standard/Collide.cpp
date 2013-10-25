#include "Standard/pch.h"
#include "Collide.h"
#include "Algebra.h"

using namespace Collide;

// pointInTri
STANDARD_API bool Collide::pointInTri(const Point3& p, const Tri& t)
{
	//http://geometryalgorithms.com/Archive/algorithm_0105/algorithm_0105.htm#intersect_RayTriangle()
#if 0
	Point3 edge1, edge2, tvec, pvec, qvec;
	float det,inv_det;

	/* find vectors for two edges sharing vert0 */
	edge1 = t[1] - t[0];
	edge2 = t[2] - t[0];

	/* begin calculating determinant - also used to calculate U parameter */
	pvec = dir.rCross(edge2);

	/* if determinant is near zero, ray lies in plane of triangle */
	det = edge1 * pvec;

	if (det > -EPSILON && det < EPSILON)
		return false;
	inv_det = 1.0f / det;

	/* calculate distance from vert0 to ray origin */
	tvec = start - verts[0];

	/* calculate U parameter and test bounds */
	float u = (tvec * pvec) * inv_det;
	if (u < 0.0f || u > 1.0f)
		return false;

	/* prepare to test V parameter */
	qvec = tvec.rCross(edge1);

	/* calculate V parameter and test bounds */
	float v = (dir * qvec) * inv_det;
	if (v < 0.0f || u + v > 1.0f)
		return false;
#endif

	// http://www.blackpawn.com/texts/pointinpoly/default.html
	struct sSameSide
	{
		bool operator()(const Point3& p, const Point3& a, const Point3& b, const Point3& c)
		{
			Point3 cp1 = (b - a).rCross(p - a);
			Point3 cp2 = (b - a).rCross(c - a);
			if (cp1 * cp2 >= 0)
				return true;

			return false;
		}
	};

	sSameSide sameSide;

	return	sameSide(p, t[0], t[1], t[2]) &&
			sameSide(p, t[1], t[2], t[0]) &&
			sameSide(p, t[2], t[0], t[1]);
}

// rayPlaneDist
STANDARD_API bool Collide::rayPlaneDist(const Point3& start, const Point3& dir, const Plane& plane, float& dist)
{
	float denom = plane.n * dir;

	// if denom = 0 ray is parallel to the plane (right angle to the normal)
	if (denom == 0)
	{
		return false;
	}

	dist = (plane.d - plane.n * start) / denom;
	return true;
}

// rayPlane
STANDARD_API bool Collide::rayPlane(const Point3& start, const Point3& dir, const Plane& plane, Point3& hit)
{
	float t;

	if (!rayPlaneDist(start, dir, plane, t))
		return false;

	if (t > 0)
	{
		hit = start + dir * t;
		return true;
	}

	return false;
}

// rayPolygon
STANDARD_API bool Collide::rayPolygon(const Tri& verts, const Point3& start, const Point3& dir, Point3& hit)
{
	Point3 normal = verts.calcNormal();
	Plane plane(normal, verts[0]);

	if (!rayPlane(start, dir, plane, hit))
		return false;

#if 1
	return pointInTri(hit, verts);
#else

	float u,v,d;

	return D3DXIntersectTri((const D3DXVECTOR3*)&verts[0], (const D3DXVECTOR3*)&verts[1], (const D3DXVECTOR3*)&verts[2], (const D3DXVECTOR3*)&start, (const D3DXVECTOR3*)&dir, &u, &v, &d) != 0;
#endif
}

// rayCylinder
STANDARD_API bool Collide::rayCylinderAA(const Point3& start, const Point3& dir, const Point3& cylinderOrigin, float height, float radius, Point3& hit)
{
	Point2 dir2D = Point2(dir.x, dir.y);
	dir2D.normalise();

	float A = sqr(dir2D.x) + sqr(dir2D.y);
	float B = 2 * (dir2D.x * (start.x - cylinderOrigin.x) + dir2D.y * (start.y - cylinderOrigin.y));
	float C = -sqr(radius) + sqr(start.x - cylinderOrigin.x) + sqr(start.y - cylinderOrigin.y);

	float u0, u1;
	if (!Algebra::solveQuadratic(A, B, C, u0, u1))
		return false;

	if (u0 >= 0)
	{
		if (u1 < 0 || u0 < u1)
			hit = start + u0 * dir;
		else
			hit = start + u1 * dir;
	}
	else
	{
		if (u0 < 0 || u1 < u0)
			hit = start + u1 * dir;
		else
			hit = start + u0 * dir;
	}

	return (u0 >= 0 || u1 >= 0) && abs(hit.z - cylinderOrigin.z) <= height;
}

// boxBox2D
STANDARD_API bool Collide::boxBox2D(const Point2& min0, const Point2& max0, const Point2& min1, const Point2& max1)
{
	return (min1.x >= min0.x && min1.y >=min0.y && min1.x <= max0.x && min1.y <= max0.y) ||
			(min1.x >= min0.x && min1.y >= min0.y && max1.x <= max0.x && max1.y <= max0.y) ||
			(max1.x >= min0.x && max1.y >= min0.y && min1.x <= max0.x && min1.y <= max0.y) ||
			(max1.x >= min0.x && max1.y >= min0.y && max1.x <= max0.x && max1.y <= max0.y);
}

// boxBox3D
STANDARD_API bool Collide::aabbAABB(const AABB& b0, const AABB& b1)
{
	Point3 size0 = (b0.extent) * 0.5f;
	Point3 size1 = (b1.extent) * 0.5f;
	Point3 v = b1.origin - b0.origin;
	return abs(v.x) <= abs(size0.x + size1.x) && abs(v.y) <= abs(size0.y + size1.y) && abs(v.z) <= abs(size0.z + size1.z);
}

// sphereSphereSweep
// http://www.gamasutra.com/features/19991018/Gomez_2.htm
STANDARD_API bool Collide::sphereSphereSweep(const Point3& start0, const Point3& end0, float radius0, const Point3& start1, const Point3& end1, float radius1, float& u)
{
    const Point3 AB = start1 - start0;
    //Point3 from A0 to B0

    const Point3 vab = (end1 - start1) - (end0 - start0);
    //relative velocity (in normalized time)

    const float rab = radius0 + radius1;

    const float a = vab * vab;
    //u*u coefficient

    const float b = 2 * (vab * AB);
    //u coefficient

    const float c = AB * AB - rab*rab;
    //constant term

    //check if they're currently overlapping
    if( AB * AB <= rab*rab )
    {
        u = 0;
        return true;
    }

	float testU0, testU1;

    //check if they hit each other
    // during the frame
    if( a != 0.0f && Algebra::solveQuadratic( a, b, c, testU0, testU1 ) )
    {
		u = std::min(testU0, testU1);
        if( u >= 0 && u <= 1 )
		{
	        return true;
		}
    }

    return false;
}

// spherePlaneSweep
STANDARD_API bool Collide::spherePlaneSweep(const Point3& start, const Point3& end, float radius, const Plane& p, float& u)
{
	// check if start point is penetrating
	if (abs(p.distance(start)) <= radius)
	{
		u = 0;
		return true;
	}

	u = radius + p.d - start.x * p.n.x - start.y * p.n.y - start.z * p.n.z;
	u /= (end - start) * p.n;

	if (u >= 0 && u <= 1)
	{
		return true;
	}

	return false;
}

// spherePlaneSweepTwoSide
STANDARD_API bool Collide::spherePlaneSweepTwoSide(const Point3& start, const Point3& vel, float radius, const Plane& p, float& u)
{
	float main = p.d - start.x * p.n.x - start.y * p.n.y - start.z * p.n.z;

	u = (radius + main) / (vel * p.n);

	if (u >= 0 && u <= 1)
	{
		return true;
	}

	u = (-radius + main) / (vel * p.n);

	if (u >= 0 && u <= 1)
	{
		return true;
	}

	return false;
}

// sphereTriSweep
STANDARD_API bool Collide::sphereTriSweep(const Point3& start, const Point3& vel, float radius, const Tri& t, float& u)
{
	Plane plane(t.calcNormal(), t[0]);
    if (!spherePlaneSweep(start, vel, radius, plane, u))
		return false;

	// get point on plane that sphere hit
	Point3 hit = start + vel * u;
	hit -= plane.distance(hit) * plane.n;

	return pointInTri(hit, t);
}

// pointInBox2D
STANDARD_API bool Collide::pointInBox2D(const Point2& p, const Point2& min, const Point2& max)
{
	return p.x >= min.x && p.x <= max.x && p.y >= min.y && p.y <= max.y;
}

STANDARD_API AABB Collide::aabbAABBIntersection(const AABB& a0, const AABB& a1)
{
	AABB intersection;

	Point3 v = a1.origin - a0.origin;

	Point3 minSize;
	Point3 maxSize;
	Point3 minOrigin;

	// find the larger and smaller of the two boxes, and take origin components at each axis from the box with the smallest extent at that axis
	if (a0.extent.x < a1.extent.x)
	{
		minSize.x = a0.extent.x;
		maxSize.x = a1.extent.x;
		minOrigin.x = a0.origin.x;
	}
	else
	{
		minSize.x = a1.extent.x;
		maxSize.x = a0.extent.x;
		minOrigin.x = a1.origin.x;
	}
	if (a0.extent.y < a1.extent.y)
	{
		minSize.y = a0.extent.y;
		maxSize.y = a1.extent.y;
		minOrigin.y = a0.origin.y;
	}
	else
	{
		minSize.y = a1.extent.y;
		maxSize.y = a0.extent.y;
		minOrigin.y = a1.origin.y;
	}
	if (a0.extent.z < a1.extent.z)
	{
		minSize.z = a0.extent.z;
		maxSize.z = a1.extent.z;
		minOrigin.z = a0.origin.z;
	}
	else
	{
		minSize.z = a1.extent.z;
		maxSize.z = a0.extent.z;
		minOrigin.z = a1.origin.z;
	}

	// the maximum extent of the intersection never exceeds that of the smaller box
	intersection.extent.x = std::min(minSize.x, (minSize.x + maxSize.x) * 0.5f - abs(v.x));
	intersection.extent.y = std::min(minSize.y, (minSize.y + maxSize.y) * 0.5f - abs(v.y));
	intersection.extent.z = std::min(minSize.z, (minSize.z + maxSize.z) * 0.5f - abs(v.z));

	intersection.origin.x = minOrigin.x + (intersection.extent.x - minSize.x) * sign(v.x) * 0.5f;
	intersection.origin.y = minOrigin.y + (intersection.extent.y - minSize.y) * sign(v.y) * 0.5f;
	intersection.origin.z = minOrigin.z + (intersection.extent.z - minSize.z) * sign(v.z) * 0.5f;

	return intersection;
}

STANDARD_API bool Collide::raySphere(const Point3& start, const Point3& dir, const Point3& origin, float radius)
{
	// get perp dist to origin
	return dir.rCross(origin - start).sqrLength() <= sqr(radius);
}

STANDARD_API bool Collide::rayAABB(const AABB& aabb, const Point3& start, const Point3& dir, Point3& hit)
{
	if (!aabb.extent.x || !aabb.extent.y || !aabb.extent.z)
		return false;

	// form line equation
	// transform ray into OBB's coordinate space

	// form planes from cube sides
	Plane planes[6];
	aabb.toPlanes(planes);

	// reject backfacing planes
	bool bReject[6];
	for (int i = 0; i < 6; i++)
	{
		bReject[i] = (planes[i].n * dir) > 0;
	}

	// out of the remaining planes, get the closest intersection
	float dist = FLT_MAX;
	int planeHit = 0;
	for (int i = 0; i < 6; i++)
	{
		if (bReject[i])
			continue;

		float newDist;

		if (!rayPlaneDist(start, dir, planes[i], newDist))
			continue;

		if (newDist >= 0 && newDist < dist)
		{
			dist = newDist;
			planeHit = i;
		}
	}

	if (dist != FLT_MAX)
	{
		// use the distance found to get an intersection point
		hit = start + dir * dist;

		// move the hit point inside the box a little
		assert(aabb.extent.x > 0.00001f);
		assert(aabb.extent.y > 0.00001f);
		assert(aabb.extent.z > 0.00001f);
		hit += planes[planeHit].n * 0.00001f;

		// return true if intersection point is in the box
		return aabbPoint(aabb, hit);
	}

	return false;
}

STANDARD_API bool Collide::aabbPoint(const AABB& aabb, const Point3& p)
{
	return p.x >= aabb.origin.x - aabb.extent.x * 0.5f &&
		p.x <= aabb.origin.x + aabb.extent.x * 0.5f &&
		p.y >= aabb.origin.y - aabb.extent.y * 0.5f &&
		p.y <= aabb.origin.y + aabb.extent.y * 0.5f &&
		p.z >= aabb.origin.z - aabb.extent.z * 0.5f &&
		p.z <= aabb.origin.z + aabb.extent.z * 0.5f;
}

STANDARD_API bool Collide::discAAPlane(const Point3& start0, float radius0, const Plane& plane)
{
	Point3 discNormal = Point3(0, 0, 1);

	// if disc is parallel to plane, then just check distance is zero
	if (discNormal * plane.n == 1.0f)
		return plane.distance(start0) == 0;

	Point3 discToPlane = plane.distance(start0) * -plane.n;

	// project discToPlane onto disc plane
	discToPlane = discToPlane - discNormal * (discToPlane * discNormal);

	return discToPlane.length() < radius0;
}

STANDARD_API bool Collide::discAAPlaneSweep(const Point3& start0, const Point3& vel0, float radius0, const Plane& plane)
{
	assert(0);
	/*Point3 discNormal = Point3(0, 0, 1);

	// if disc is parallel to plane, then just check distance is zero
	if (discNormal * plane.n == 1.0f)
		return plane.distance(start0 + vel0 * u) == 0;

	Point3 discToPlane = plane.distance(start0 + vel0 * u) * -plane.n;

	// project discToPlane onto disc plane
	discToPlane = discToPlane - discNormal * (discToPlane * discNormal);

	return discToPlane.length() < radius0;*/
	return false;
}

STANDARD_API bool Collide::cylinderAAPlane(const Point3& start0, float radius0, float height0, const Plane& plane)
{
	assert(0); // i assume this function is incomplete? can't remember.
	// top cap
	return discAAPlane(Point3(start0.x, start0.y, start0.z + radius0), radius0, plane);
}

STANDARD_API bool Collide::cylinderAAPlaneSweep(const Point3& start0, const Point3& vel0, float radius0, float height0, const Plane& plane, float& u)
{
	assert(0);
	return false;
}

STANDARD_API bool Collide::aabbPlane(const AABB& b0, const Plane& plane)
{
	// not tested!!
	float dist;

	if (rayPlaneDist(b0.origin, Point3(1, 0, 0), plane, dist))
	{
		if (abs(dist) <= b0.extent.x * 0.5f)
			return true;
	}
	else if (rayPlaneDist(b0.origin, Point3(0, 1, 0), plane, dist))
	{
		if (abs(dist) <= b0.extent.y * 0.5f)
			return true;
	}
	else if (rayPlaneDist(b0.origin, Point3(0, 0, 1), plane, dist))
	{
		if (abs(dist) <= b0.extent.z * 0.5f)
			return true;
	}

	return false;
}

STANDARD_API bool Collide::spherePlane(const Point3& origin, float radius, const Plane& plane)
{
	return abs(plane.distance(origin)) <= radius;
}
