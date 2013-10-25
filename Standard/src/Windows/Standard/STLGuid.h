#ifndef STL_GUID_H
#define STL_GUID_H

// compare GUIDs
inline bool operator < (const GUID& lhs, const GUID& rhs)
{
	return lhs.Data1 < rhs.Data1 || lhs.Data2 < rhs.Data2 || lhs.Data3 < rhs.Data3
			|| memcmp(lhs.Data4, rhs.Data4, 8) < 0;
}

#endif