// ---------------------------------------------------------------------------------------------------------
// 
// SkelMesh
// 
// ---------------------------------------------------------------------------------------------------------
#include "pch.h"

#include <Standard/Config.h>
#include <Standard/textstream.h>
#include <Standard/D3DHelp.h>
#include "SkelMesh.h"
#include "SDeviceD3D.h"
#include "AppBase.h"
#include "Exception/Video.h"

// Bone
// load
void Bone::load(itextstream& s)
{
	std::string str;
	Point3 p, min, max, pivot, scale;
	Bone b;
	Quat q;

	s >> "\t\tParent " >> str >> s.endl;
	setParentName(str);

	// bone transform
	s >> "\t\t(" >> min >> ", " >> max >> "), " >> p >> ", " >> q >> ", " >> scale >> s.endl;

	m_extent = max - min;
	setPos(p);
	setRot(q);
	setScale(scale);

	// pivot
	s >> "\t\tPivot " >> pivot >> s.endl;
	setPivot(pivot);

	// vertex refs
	s >> "\t\tVerts" >> s.endl;
	int vertRef;
	while (s.has("\t\t\t"))
	{
		s >> "\t\t\t" >> vertRef >> s.endl;
		m_vertRefs.push_back(vertRef);
	}
}

// calcInverseRefXForm
void Bone::calcInverseRefXForm()
{
	m_inverseRefXForm = worldXForm();
	m_inverseRefXForm.invertSimple();
}


// construct
SkelMesh::SkelMesh()
{
	m_pRoot = 0;
}

// load
void SkelMesh::load(itextstream& s)
{
	MeshObject::load(s);

	std::string str;
	Point3 p;
		
	if (0)
	{
		Bone* b;

		m_bones.resize(3);

		b = new Bone;
		// waist (root)
		b->setExtent(Point3(10, 5, 10));
		b->setPivot(Point3(0, 0, -5));
//			b.setRot(QuatAngleAxis(PI * 0.25f, Point3(0, 1, 0)));
		m_bones[0] = b;

		b = new Bone;
		// torso
		b->setExtent(Point3(10, 5, 20));
		b->setPivot(Point3(0, 0, -10));
//			b.setRot(QuatAngleAxis(PI * 0.25f, Point3(0, 1, 0)));
		b->setPos(Point3(0, 0, 5));
		b->setParent(m_bones[0].ptr());
		m_bones[1] = b;

		b = new Bone;
		// larm
		b->setExtent(Point3(20, 5, 5));
		b->setPivot(Point3(10, 0, 0));
		b->setPos(Point3(-5, 0, 10));
		b->setParent(m_bones[1].ptr());
		m_bones[2] = b;

		m_pRoot = m_bones[0];
	}
	else
	{
		// anim token
		if (s.has("Anim "))
			s >> "Anim " >> str >> s.endl;
		else
			return; // no animations

		float fps;
		// fps
		s >> "\tFPS " >> fps >> s.endl;
		m_animation.setFps(fps);

		// sequences
		s << itextstream::str_delimiter(" \n'");

		IntRange range;
		while (s.has("\t'"))
		{
			s >> "\t'" >> str >> " " >> range.min >> " to " >> range.max >> s.endl;
			m_animation.addSequence(str, range);
		}
		
		s << itextstream::str_delimiter();

		if (!m_animation.size())
			throwf("SkelMesh::load, 'Anim' token found but no sequences found");

		// bones
		while (s.has("\tBone"))
		{
			s >> "\tBone " >> str >> s.endl;
			m_bones.insert(m_bones.end(), new Bone);
			
			Bone& b = *m_bones.back();

			// load bone data
			b.setName(str);
			b.load(s);

			ObjectAnimTrack& t = m_animation.addTrack(&b);
			// load bone anim data
			s << itextstream::str_delimiter("\n");

			while (s.has("\t\t") && !s.has("\tBone"))
			{
				s >> "\t\t" >> str >> s.endl;

				if (str == "Linear Position Keys")
				{
					PosKey p;

					while (s.has("\t\t\t"))
					{
						s >> "\t\t\t" >> p.frame >> ": " >> p.pos >> s.endl;
						t.addPosKey(p);
					}
				}
				else if (str == "Linear Rotation Keys")
				{
					RotKey r;

					while (s.has("\t\t\t"))
					{
						s >> "\t\t\t" >> r.frame >> ": " >> r.rot >> s.endl;
						t.addRotKey(r);
					}
				}
			}

			s << itextstream::str_delimiter();
		}

		m_animation.calcLength();

		m_pRoot = 0;
		// resolve bone parents
		for (uint i = 0; i < m_bones.size(); i++)
		{
			for (uint j = 0; j < m_bones.size(); j++)
			{
				if (i != j && m_bones[i]->parentName() == m_bones[j]->name())
				{
					m_bones[i]->setParent(m_bones[j].ptr());
					break;
				}
			}

			if (m_bones[i]->parentName() == "None")
			{
				m_pRoot = m_bones[i];
			}
			else
			{
				if (!m_bones[i]->parent())
					throwf("SkelMesh::load, Bone " + m_bones[i]->name() + "'s parent could not be resolved");
			}
		}
	}

	// store reference transformation
	for (uint i = 0; i < m_bones.size(); i++)
	{
		m_bones[i]->calcInverseRefXForm();
	}

	if (m_bones.empty())
		throwf("SkelMesh::load, No bones found"); 

	if (!m_pRoot)
		throwf("SkelMesh::load, No root bone found"); 
}

// draw
void SkelMesh::draw()
{
	updateMesh();

	// draw mesh
	MeshObject::draw();

	// visualise bones
	if (AppBase().options().get<bool>("Debug", "ShowBones"))
	{
		D3D().wireframe(true);
		for (uint i = 0; i < m_bones.size(); i++)
		{
			Bone& b = *m_bones[i];

			DX_ENSURE(D3DD().SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE));
			DX_ENSURE(D3DD().SetTexture(0, 0));
			DX_ENSURE(D3DD().SetRenderState(D3DRS_LIGHTING, false));

			struct VERT
			{
				float pos[3];
				D3DCOLOR c;
			};

			Point3 e = b.extent() * 0.5f;

			VERT verts[8];
			Point3 p[8] = {
				Point3(e.x, e.y, -e.z),
				Point3(-e.x, e.y, -e.z),
				Point3(-e.x, e.y, e.z),
				Point3(e.x, e.y, e.z),
				Point3(e.x, -e.y, -e.z),
				Point3(-e.x, -e.y, -e.z),
				Point3(-e.x, -e.y, e.z),
				Point3(e.x, -e.y, e.z)
			};

			// transform verts
			for (uint i = 0; i < 8; i++)
			{
				p[i] *= b.worldXForm();

				verts[i].pos[0] = p[i].x;
				verts[i].pos[1] = p[i].y;
				verts[i].pos[2] = p[i].z;
				verts[i].c = D3DCOL(1,0,0,1);
			}

			ushort idxs[36] = {
				2, 1, 0, // rear
				0, 3, 2,
				4, 5, 6, // front
				6, 7, 4,
				4, 0, 1, // bottom
				1, 5, 4,
				2, 3, 7, // top
				7, 6, 2,
				1, 2, 6, // left
				6, 5, 1,
				7, 3, 0, // right
				0, 4, 7
			};

			if (DXFAIL(D3DD().DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 8, 12, idxs, D3DFMT_INDEX16, verts, sizeof(VERT))))
				return;
		}
		D3D().wireframe(false);
	}
}

// updateMesh
void SkelMesh::updateMesh()
{
	// move mesh vertices
	VERT *pBufVerts = 0;
	if (DXFAIL(m_pVBuffer->Lock(0, 0, (void **)&pBufVerts, 0)))
	{
		throwf("Skeletal mesh update: vertex buffer lock failed");
	}

	// for each bone, update related list of verts
	for (uint i = 0; i < m_bones.size(); i++)
	{
		Bone& b = *m_bones[i];

		// get xform relative to bone's reference xform
		Matrix4 boneXForm = b.inverseRefXForm() * b.worldXForm();

		for (uint j = 0; j < b.refs().size(); j++)
		{
			int vertIdx = b.refs()[j];
			Point3& newVert = pBufVerts[vertIdx].v;
			
			newVert = m_verts[vertIdx];

			newVert *= boneXForm;
		}
	}

	m_pVBuffer->Unlock();
}
