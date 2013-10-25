// ---------------------------------------------------------------------------------------------------------
// 
// Global functions to stream max data types
// 
// ---------------------------------------------------------------------------------------------------------

#pragma once

// Point3
inline std::ostream& operator << (std::ostream& s, const Point3& o)
{
	return s << "(" << o.x << ", " << o.y << ", " << o.z << ")";
}


// Face
inline std::ostream& operator << (std::ostream& s, const Face& o)
{
	return s << "(" << o.v[0] << ", " << o.v[1] << ", " << o.v[2] << ")";
}


// Quat
inline std::ostream& operator << (std::ostream& s, Quat& o)
{
	// as far as I can tell, max quats are the wrong way around (clockwise around axis rather than counter-clockwise)
	return s << "(" << o.w << ", " << -o.x << ", " << -o.y << ", " << -o.z << ")";
}


// Box3
inline std::ostream& operator << (std::ostream& s, const Box3& o)
{
	return s << "(" << o.pmin << ", " << o.pmax << ")";
}


// Matrix3
inline std::ostream& operator << (std::ostream& s, const Matrix3& o)
{
	float* m = (float*)o.GetAddr();

	s << "(";

	for (int i = 0; i < 4 * 3 - 1; i++, m++)
	{
		s << (float)(*m) << ", ";
	}

	s << *m << ")";

	return s;
}
