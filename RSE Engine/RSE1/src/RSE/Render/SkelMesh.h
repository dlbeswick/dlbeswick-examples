// ---------------------------------------------------------------------------------------------------------
// 
// SkelMesh
// 
// ---------------------------------------------------------------------------------------------------------
#pragma once


#include "MeshObject.h"
#include "Animation.h"


class RSE_API Bone : public Object
{
public:
	typedef SortedVec<int> VertRefs;

	void addRef(int idx) { m_vertRefs.insert(idx); } 
	void removeRef(int idx) { m_vertRefs.remove(idx); }

	VertRefs& refs() { return m_vertRefs; } 

	void calcInverseRefXForm();
	const Matrix4& inverseRefXForm() { return m_inverseRefXForm; }
	Point3 extent() const { return m_extent; }
	void setExtent(const Point3& p) { m_extent = p; }

	const std::string& parentName() const { return m_parentName; }
	void setParentName(const std::string& name) { m_parentName = name; }

	virtual void load(itextstream& s);

protected:
	std::string m_parentName;
	Point3		m_extent;
	VertRefs	m_vertRefs;	// affected verts
	Matrix4		m_inverseRefXForm;
};


class RSE_API SkelMesh : public MeshObject
{
public:
	typedef std::vector<SmartPtr<Bone> > BoneList;

	SkelMesh();

	virtual void load(itextstream& s);
	virtual void draw();

	virtual void updateMesh();

	Animation& animation() { return m_animation; }

protected:

	Animation	m_animation;
	BoneList	m_bones;
	SmartPtr<Bone>	m_pRoot;
};