// ---------------------------------------------------------------------------------------------------------
//
// Collide
//
// ---------------------------------------------------------------------------------------------------------
#pragma once



#include "Standard/api.h"
#include "Math.h"

namespace Collide
{

using namespace DMath;

// points
STANDARD_API bool pointInTri(const Point3& p, const Tri& t);
STANDARD_API bool pointInBox2D(const Point2& p, const Point2& min, const Point2& max);

// rays
STANDARD_API bool rayPlaneDist(const Point3& start, const Point3& dir, const Plane& plane, float& dist);
STANDARD_API bool rayPlane(const Point3& start, const Point3& dir, const Plane& plane, Point3& hit);
STANDARD_API bool rayPolygon(const Tri& verts, const Point3& start, const Point3& dir, Point3& hit);
STANDARD_API bool raySphere(const Point3& start, const Point3& dir, const Point3& origin, float radius);
STANDARD_API bool rayAABB(const AABB& aabb, const Point3& start, const Point3& dir, Point3& hit);
STANDARD_API bool rayCylinderAA(const Point3& start, const Point3& dir, const Point3& cylinderOrigin, float height, float radius, Point3& hit);

// boxes
STANDARD_API bool boxBox2D(const Point2& min0, const Point2& max0, const Point2& min1, const Point2& max1);

// aabb
STANDARD_API bool aabbPoint(const AABB& aabb, const Point3& p);
STANDARD_API bool aabbAABB(const AABB& b0, const AABB& b1);
STANDARD_API AABB aabbAABBIntersection(const AABB& a0, const AABB& a1);
STANDARD_API bool aabbPlane(const AABB& b0, const Plane& plane);

// spheres
STANDARD_API bool spherePlane(const Point3& origin, float radius, const Plane& plane);
STANDARD_API bool sphereSphereSweep(const Point3& start0, const Point3& end0, float radius0, const Point3& start1, const Point3& end1, float radius1, float& u);
STANDARD_API bool spherePlaneSweep(const Point3& start, const Point3& end, float radius, const Plane& p, float& u);
STANDARD_API bool spherePlaneSweepTwoSide(const Point3& start, const Point3& vel, float radius, const Plane& p, float& u);
STANDARD_API bool sphereTriSweep(const Point3& start, const Point3& vel, float radius, const Tri& t, float& u);

// discs
STANDARD_API bool discAAPlane(const Point3& start0, float radius0, const Plane& plane);
STANDARD_API bool discAAPlaneSweep(const Point3& start0, const Point3& vel0, float radius0, const Plane& plane);

// cylinders
STANDARD_API bool cylinderAAPlane(const Point3& start0, float radius0, float height0, const Plane& plane);
STANDARD_API bool cylinderAAPlaneSweep(const Point3& start0, const Point3& vel0, float radius0, float height0, const Plane& plane, float& u);

};
