// ---------------------------------------------------------------------------------------------------------
// 
// XtremeExport
// 
// ---------------------------------------------------------------------------------------------------------
#include "stdafx.h"
#pragma hdrstop

#include "XtremeExport.h"
#include <Standard/Help.h>
#include <Standard/Ptr.h>
#include <vector>


#define LOG(x) MessageBox(0, x, #x, 0)


// Enum base
class ExportBase : public ITreeEnumProc
{
public:
	ExportBase(std::ostream& s, Interface& i) : m_file(s), m_interface(i) {}
	const std::string& report() { return m_report; }

	virtual int callback(INode* pNode) = 0;
	virtual const char* name() const = 0;

protected:
	TriObject* GetTriObjectFromObj(Object *obj);
	Modifier* FindModifier(IDerivedObject& obj, const TCHAR* name);

	std::string m_report;
	std::ostream& m_file;
	Interface& m_interface;
};


// Return a pointer to a TriObject given an Object or return NULL
// if the object cannot be converted to a TriObject
TriObject* ExportBase::GetTriObjectFromObj(Object* obj)
{
	if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0)))
	{ 
		TriObject *tri = (TriObject *) obj->ConvertToType(0, Class_ID(TRIOBJ_CLASS_ID, 0));
		// Note that the TriObject should only be deleted
		// if the pointer to it is not equal to the object
		// pointer that called ConvertToType()
		if (obj != tri)
		{
			MessageBox(0, "Error: Editable mesh did not have an existing TriObject", 0, 0);
			delete tri;
			return 0;
		}

		return tri;
	}
	else
	{
		return 0;
	}
}


// FindModifier
Modifier* ExportBase::FindModifier(IDerivedObject& obj, const TCHAR* name)
{
	for (int i = 0; i < obj.NumModifiers(); i++)
	{
		Modifier& m = *obj.GetModifier(i);

		if (stricmp(m.GetObjectName(), name) == 0)
		{
			return &m;
		}
	}

	return 0;
}


// Mesh Export
class ExportMesh : public ExportBase
{
public:
	ExportMesh(std::ostream& s, Interface& i) : ExportBase(s, i) {}
	virtual int callback(INode* pNode);
	virtual const char* name() const { return "Mesh Data Exporter"; }
};

int ExportMesh::callback(INode* pNode)
{
	if (!pNode->GetObjectRef())
		return TREE_CONTINUE;

	INode& node = *pNode;

	if (!node.GetObjectRef())
		return TREE_CONTINUE;

	// Objects with modifiers only
	Object* pObj = node.GetObjectRef();
	while (pObj)
	{
		if (stricmp(pObj->GetObjectName(), "Edit Mesh") == 0 || stricmp(pObj->GetObjectName(), "Editable Mesh") == 0)
			break;

		if (pObj->SuperClassID() != GEN_DERIVOB_CLASS_ID)
			return TREE_CONTINUE;

		IDerivedObject* pDObj = (IDerivedObject*)pObj;

		// Find Edit Mesh modifier
		if (FindModifier(*pDObj, "Edit Mesh") || FindModifier(*pDObj, "Editable Mesh"))
		{
			break;
		}

		// next derived object
		pObj = pDObj->GetObjRef();
	}

	if (!pObj)
		return TREE_CONTINUE;

	// Editable mesh objects will have an tri object - it will not need to be allocated
	TriObject* pTri = GetTriObjectFromObj(node.EvalWorldState(0).obj);
	if (!pTri)
		return TREE_CONTINUE;
	
	m_report += "Exporting mesh ";
	m_report += node.GetName();
	m_report += "\n";

	m_file << "Mesh " << node.GetName() << std::endl;

	Mesh& mesh = pTri->GetMesh();

	if (mesh.numVerts)
	{
		m_file << "\tVertices" << std::endl;
		
		for (int i = 0; i < mesh.numVerts; i++)
		{
			m_file << "\t\t" << mesh.verts[i] * node.GetObjectTM(0) << std::endl;
		}
	}

	if (mesh.numFaces)
	{
		m_file << "\tFaces" << std::endl;
		
		for (int i = 0; i < mesh.numFaces; i++)
		{
			m_file << "\t\t" << mesh.faces[i] << std::endl;
		}
	}

	if (mesh.numVerts)
	{
		m_file << "\tNormals" << std::endl;
		
		for (int i = 0; i < mesh.numVerts; i++)
		{
			m_file << "\t\t" << mesh.getNormal(i) << std::endl;
		}
	}

	return TREE_CONTINUE;
	//return TREE_ABORT;
	//return TREE_CONTINUE;
	//return TREE_IGNORECHILDREN;
}


// Animation Export
class ExportSkelAnim : public ExportBase
{
public:
	ExportSkelAnim(std::ostream& s, Interface& i) : ExportBase(s, i) {}
	virtual int callback(INode* pNode);
	virtual const char* name() const { return "Skeletal Animation Data Exporter"; }
};

int ExportSkelAnim::callback(INode* pNode)
{
	if (!pNode->GetObjectRef())
		return TREE_CONTINUE;

	INode& node = *pNode;

	if (!node.GetObjectRef())
		return TREE_CONTINUE;

	// Objects with modifiers only
	Object* pObj = node.GetObjectRef();
	if (pObj->SuperClassID() != GEN_DERIVOB_CLASS_ID)
		return TREE_CONTINUE;

	IDerivedObject* pDObj = (IDerivedObject*)pObj;

	// Find Physique modifier
	Modifier* pMod = FindModifier(*pDObj, "Physique");
	if (!pMod)
		return TREE_CONTINUE;

	IPhysiqueExport* pPhysExport = (IPhysiqueExport*)pMod->GetInterface(I_PHYINTERFACE);
	if (!pPhysExport)
		return TREE_CONTINUE;

	IPhyContextExport* pPhysContext = pPhysExport->GetContextInterface(&node);
	if (!pPhysContext)
		return TREE_CONTINUE;
	
	m_report += "Exported anim for ";
	m_report += node.GetName();
	m_report += "\n";

	// write anim marker and mesh owner name
	m_file << "Anim " << node.GetName() << std::endl;
	
	// write fps
	m_file << "\tFPS " << GetFrameRate() << std::endl;

	// All rigid verts
	pPhysContext->ConvertToRigid(true);

	// Find root bone and create vertices map
	INode* pRootBone = 0;
	std::set<std::string> boneSet;
	std::multimap<std::string, int> vertMap;
	for (int i = 0; i < pPhysContext->GetNumberVertices(); i++)
	{
		IPhyVertexExport* pVertExport = (IPhyVertexExport*)pPhysContext->GetVertexInterface(i);

		if (pVertExport->GetVertexType() & BLENDED_TYPE)
		{
			char buf[128];
			sprintf(buf, "%d", i);
			m_report += "Skipped non-rigid vertex ";
			m_report += buf;
			m_report += "\n";
			continue;
		}

		IPhyRigidVertex* pRigid = (IPhyRigidVertex*)pVertExport;

		if (!pRigid->GetNode())
		{
			char buf[128];
			sprintf(buf, "%d", i);
			m_report += "No node for vertex ";
			m_report += buf;
			m_report += "\n";
			continue;
		}

		// check for root bone
		INode* pBone = pRigid->GetNode();
		if (!pRootBone && (stricmp(pBone->GetParentNode()->GetName(), "Scene Root") == 0 || stricmp(node.GetParentNode()->GetName(), pBone->GetName()) == 0))
		{
			pRootBone = pBone;
		}

		vertMap.insert(std::multimap<std::string, int>::value_type(pBone->GetName(), i));
	}

	if (!pRootBone)
	{
		m_report += "No root bone was found\nThere is most likely an error in your hierarchy. Re-link and try again.";
		return TREE_CONTINUE;
	}

	// Get list of bones
	struct FindBonesFunctor
	{
		FindBonesFunctor(std::set<std::string>& boneSet, INode* pNode)
		{
			for (int i = 0; i < pNode->NumberOfChildren(); i++)
			{
				INode* n = pNode->GetChildNode(i);
				if (n->NumberOfChildren())
					FindBonesFunctor(boneSet, n);
			}

			boneSet.insert(pNode->GetName());
		}
	};
	FindBonesFunctor(boneSet, pRootBone);

	// Write list of anims from note track
	if (pRootBone->GetNoteTrack(0))
	{
		DefNoteTrack* pNote = (DefNoteTrack*)pRootBone->GetNoteTrack(0);
		for (int i = 0; i < pNote->keys.Count(); i++)
		{
			NoteKey& key = *pNote->keys[i];
			m_file << "\t'" << key.note << "' ";
			
			if (i == pNote->keys.Count() - 1)
			{
				m_file << (float)key.time / GetTicksPerFrame() << " to " << pRootBone->GetTimeRange(TIMERANGE_ALL | TIMERANGE_CHILDNODES | TIMERANGE_CHILDANIMS).End() / GetTicksPerFrame() << std::endl;
			}
			else
			{
				m_file << (float)key.time / GetTicksPerFrame() << " to " << (pNote->keys[i+1]->time / GetTicksPerFrame() - 1) << std::endl;
			}
		}
	}
	else
	{
		m_report += "No note track found for root bone " + std::string(pRootBone->GetName());

		Interval duration = pRootBone->GetTimeRange(TIMERANGE_ALL);
		m_file << "\t'default' " << duration.Start() << " to " << duration.End() << std::endl;
	}

	// Write bones with transform relative to model space, plus associated verts
	for (std::set<std::string>::iterator it = boneSet.begin(); it != boneSet.end(); it++)
	{
		m_file << "\tBone " << *it << std::endl;

		INode* pBone = m_interface.GetINodeByName((*it).c_str());
		if (!pBone)
		{
			m_report += "Node " + *it + " not found by name";
			continue;
		}

		std::string parentName;
		INode* pBoneParent = pBone->GetParentNode();
		if (pBoneParent && pBone != pRootBone)
		{
			parentName = pBoneParent->GetName();
		}
		else
		{
			parentName = "None";
		}
		m_file << "\t\tParent " << parentName << std::endl;

		Box3 OBB;

		// Get local transform
		Point3 localPos;
		Quat localRot;
		Point3 localScale;
		Matrix3 parentNodeTM;
		Point3 pivotPos;

		if (pBoneParent)
		{
			parentNodeTM = pBoneParent->GetNodeTM(0);
		}
		else
		{
			parentNodeTM.IdentityMatrix();
		}

		Object* pBoneObj = pBone->GetObjectRef();
		if (!pBoneObj)
		{
			m_report += "Node " + *it + " had no object ref";
			continue;
		}

		// get local bounding box
		pBoneObj->GetDeformBBox(0, OBB);
		// get translation
		{
			Control* c = pBone->GetTMController()->GetPositionController();
			Interval range = FOREVER;
			Point3 p;
			c->GetValue(0, &p, range);
			localPos = p;
		}
		// get rotation
		{
			Control* c = pBone->GetTMController()->GetRotationController();
			Interval range = FOREVER;
			Quat quat;
			c->GetValue(0, &quat, range);
			localRot = quat;
		}
		// get scale
		{
			Control* c = pBone->GetTMController()->GetScaleController();
			Interval range = FOREVER;
			ScaleValue s ;
			c->GetValue(0, &s, range);
			localScale = s.s;
		}

		// get pivot point
		pivotPos = -(OBB.Center() + pBone->GetObjOffsetPos());
		
		m_file << "\t\t" << OBB << ", " << localPos << ", " << localRot << ", " << localScale << std::endl;

		m_file << "\t\tPivot " << pivotPos << std::endl;

		// write vert indexes
		m_file << "\t\tVerts" << std::endl;
		for (std::multimap<std::string, int>::iterator mIt = vertMap.lower_bound(*it); mIt != vertMap.upper_bound(*it); mIt++)
		{
			m_file << "\t\t\t" << mIt->second << std::endl;
		}

		// Write animation data

		// Write keys
		Control* pControl;
		IKeyControl* pIKeys;
		
		// position
		pControl = pBone->GetTMController()->GetPositionController();
		if (!pControl)
			continue;

		pIKeys = GetKeyControlInterface(pControl);
		if (!pIKeys)
			continue;
		
		if (pIKeys->GetNumKeys())
		{
			if (pControl->ClassID() == Class_ID(LININTERP_POSITION_CLASS_ID, 0))
			{
				m_file << "\t\tLinear Position Keys" << std::endl;
				for (int ki = 0; ki < pIKeys->GetNumKeys(); ki++)
				{
					ILinPoint3Key key;
					pIKeys->GetKey(ki, &key);

					m_file << "\t\t\t" << (float)key.time / GetTicksPerFrame() << ": " << key.val << std::endl;
				}
			}
			else
			{
				m_report += "Unsupported position controller type for bone " + std::string(pBone->GetName()) + "\n";
			}
		}

		// rotation
		pControl = pBone->GetTMController()->GetRotationController();
		if (!pControl)
			continue;

		pIKeys = GetKeyControlInterface(pControl);
		if (!pIKeys)
			continue;
		
		if (pIKeys->GetNumKeys())
		{
			if (pControl->ClassID() == Class_ID(LININTERP_ROTATION_CLASS_ID, 0))
			{
				m_file << "\t\tLinear Rotation Keys" << std::endl;
				for (int ki = 0; ki < pIKeys->GetNumKeys(); ki++)
				{
					Quat rotVal;
					ILinRotKey key;
					pIKeys->GetKey(ki, &key);

					rotVal = key.val;

					m_file << "\t\t\t" << (float)key.time / GetTicksPerFrame() << ": " << rotVal << std::endl;
				}
			}
			else
			{
				m_report += "Unsupported rotation controller type for bone " + std::string(pBone->GetName()) + "\n";
			}
		}
	}

	return TREE_CONTINUE;
	//return TREE_ABORT;
	//return TREE_CONTINUE;
	//return TREE_IGNORECHILDREN;
}


// DoExport
int XtremeExport::DoExport(const TCHAR *name, ExpInterface *ei, Interface *interf, BOOL suppressPrompts, DWORD options)
{
	std::ofstream s(name, std::ios::out | std::ios::trunc);
	std::ofstream report((std::string(name).substr(0, strlen(name) - 2) + "log").c_str(), std::ios::out | std::ios::trunc);
	s.precision(7);

	if (!ei->theScene)
	{
		alert("Error: no scene");
		return 1;
	}

	IScene& scene = *ei->theScene;

	int result;
	std::vector<Ptr<ExportBase> > operations;
	operations.push_back(new ExportMesh(s, *interf));
	operations.push_back(new ExportSkelAnim(s, *interf));

	for (uint i = 0; i < operations.size(); i++)
	{
		ExportBase& export = *operations[i];

		result = scene.EnumTree(&export);
		
		if (export.report().empty())
			alert("Nothing was done", export.name());
		else
			alert(export.report().c_str(), export.name());

		report << export.report();

		if (result == TREE_ABORT)
		{
			alertf("Tool %s reported an error", export.name());
			return 0;
		}
	}

	s.close();
	report.close();

	return 1;
}


