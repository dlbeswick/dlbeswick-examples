// ------------------------------------------------------------------------------------------------
//
// TransitionPieces
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "TransitionPieces.h"
#include "Game/Level.h"
#include "Render/D3DPainter.h"
#include "Render/SDeviceD3D.h"
#include "Render/Materials/MaterialTexture.h"
#include "Physics/PhysicsMgr.h"
#include "Physics/PhysSphere.h"
#include <Standard/Mapping.h>
#include <Standard/Rand.h>

IMPLEMENT_RTTI(TransitionPieces);
IMPLEMENT_RTTI(TransitionPiecesFunction);

////

class Piece : public PhysicsObject
{
public:
	Piece() {}
	~Piece() {}

	Point2 uv0;
	Point2 uv1;
	Point2 pieceSize;

	virtual void draw()
	{
		D3DD().SetTransform(D3DTS_WORLD, worldXForm());

		D3DPaint().uv0 = uv0;
		D3DPaint().uv1 = uv1;

		Point3 size = Point3(pieceSize.x * 0.5f, 0, pieceSize.y * 0.5f);

		Quad q(Point3(-size.x, 0, size.z),
				Point3(size.x, 0, size.z),
				Point3(size.x, 0, -size.z),
				Point3(-size.x, 0, -size.z));

		D3DPaint().quad(q);
		D3DPaint().draw();
	}

	virtual void getTriList(const Point3& pos, const Point3& vel, TriRefList& list) {};
	virtual bool collideTri(const Point3& pos, const Point3& vel, const Tri& t, float &u) { return false; }
};

////

TransitionPieces::TransitionPieces(const PtrGC<UIElement>& src, float time, class TransitionPiecesFunction* function, int _divisions) :
	Super(src, time),
	m_function(function)
{
	m_material = new MaterialTexture(
			m_tex,
			Point2(0, 0),
			Point2(1, 1),
			Point2i(0, 0),
			RGBA(1, 1, 1),
			RGBA(1, 1, 1),
			RGBA(1, 1, 1),
			RGBA(1, 1, 1)
		);

	if (!m_function)
	{
		RegistrantsList l;
		registrantsOfClass(TransitionPiecesFunction::static_rtti(), l);
		if (!l.empty())
			m_function = (TransitionPiecesFunction*)l[clamp((int)Rand(0, (float)l.size()), 0, (int)l.size()-1)]->func->construct();
	}

	m_function->owner = this;

	divisions = _divisions;
	m_pieces.resize(divisions * divisions);

	m_level = new Level;

	m_level->physicsMgr().setUseCollisions(false);
	m_level->scene().setClear(false);

	Camera& cam = m_level->scene().camera();
	cam.setPos(Point3(0, 0, 0));

	// use reverse xform to find corresponding world extent for pieces screen extent
    min = cam.screenToWorld(Point3(0.0f, 0.0f, 500.0f));
    max = cam.screenToWorld(Point3(1.0f, 0.75f, 500.0f));
	Point3 worldSize = max - min;
	pieceSize = m_function->pieceSize(min, max, (float)divisions);

	for (uint i = 0; i < m_pieces.size(); ++i)
	{
		Piece& p = *new Piece;
		p.setParent(*m_level);

		p.pieceSize.x = pieceSize.x;
		p.pieceSize.y = pieceSize.z;

		m_function->apply(i, p);

		p.uv0 = Point2(
					Mapping::linear(p.pos().x - pieceSize.x * 0.5f, min.x, max.x, 0.0f, 1.0f),
					Mapping::linear(p.pos().z + pieceSize.z * 0.5f, min.z, max.z, 0.0f, 1.0f)
				);
		p.uv1 = Point2(
					Mapping::linear(p.pos().x + pieceSize.x * 0.5f, min.x, max.x, 0.0f, 1.0f),
					Mapping::linear(p.pos().z - pieceSize.z * 0.5f, min.z, max.z, 0.0f, 1.0f)
				);

		m_level->collisionGroup().add(&p);
	}
}

TransitionPieces::~TransitionPieces()
{
	delete m_material;
}

void TransitionPieces::onRendered()
{
}

void TransitionPieces::draw()
{
	Super::draw();

	D3D().zbuffer(false);
	D3DPaint().setFill(*m_material);

	m_level->draw();
}

void TransitionPieces::update(float delta)
{
	Super::update(delta);

	m_level->update();
}

bool TransitionPieces::needsPerFrameRenders() 
{ 
	return m_function->needsPerFrameRenders();
}

bool TransitionPieces::finished() 
{ 
	return Super::finished() || !m_function; 
}

////

Point3 TransitionPiecesFunction::pieceSize(Point3 min, Point3 max, float divisions)
{
	Point3 size = (max - min) / divisions;
	size.x = abs(size.x);
	size.y = abs(size.y);
	size.z = abs(size.z);
	return size;
}

////

REGISTER_RTTI(TransitionPiecesExplode);

TransitionPiecesExplode::TransitionPiecesExplode()
{
	m_origin = Point3::MAX;
	m_force = 500.0f;
	m_radius = m_force;
	m_additionalImpulse = Point3::MAX;
}

TransitionPiecesExplode::TransitionPiecesExplode(const Point3& origin, float force, float radius, const Point3& additionalImpulse)
{
	m_origin = origin;
	m_force = force;
	m_radius = radius;
	m_additionalImpulse = additionalImpulse;
}

void TransitionPiecesExplode::apply(int i, PhysicsObject& o)
{
	float y = (float)(int)(i / owner->divisions);
	float x = (float)(i % owner->divisions);
	o.setPos(Point3(Mapping::linear(x, 0.0f, (float)owner->divisions, owner->min.x, owner->max.x) + owner->pieceSize.x * 0.5f, owner->max.y, Mapping::linear(y, 0.0f, (float)owner->divisions, owner->min.z, owner->max.z) - owner->pieceSize.z * 0.5f));

	if (m_additionalImpulse == Point3::MAX)
		m_additionalImpulse = -o.level().gravity() * 0.5f;

	if (m_origin == Point3::MAX)
		m_origin = Point3(0, o.pos().y + 10.0f, 0.0f);

	Point3 toPiece = o.pos() - m_origin;
	float dist = std::max(toPiece.length(), 1.0f);
	Point3 toPieceNormal = toPiece / dist;

	float appliedForce = Mapping::exp(dist, 0.0f, m_radius, m_force, 0.0f);
	o.setVel(toPieceNormal * appliedForce + m_additionalImpulse);
	
    Point3 rotAxis = toPieceNormal.rCross(Point3(0, 1, 0) * o.rot());
	o.setAngularVel(rotAxis * (appliedForce / 50.0f));

//	o.setVel(Point3(cos(Mapping::linear(uv.x, 0.0f, 1.0f, PI, 0.0f)), 1.0f, cos(Mapping::linear(uv.y, 0.0f, 1.0f, 0.0f, PI))).normal().rComponentMul(Point3(100.0f, o.pos().y * -1.5f, 100.0f)) + -o.level().gravity() * 0.25f);
//	o.setAngularVel(PI * Rand(1.0f, 5.0f) * Point3(Rand(), Rand(), Rand()).normal());
}

////

IMPLEMENT_RTTI(TransitionPiecesShitpixels);
//REGISTER_RTTI(TransitionPiecesShitpixels);

TransitionPiecesShitpixels::TransitionPiecesShitpixels(int variation)
{
	if (variation == -1)
		m_variation = (int)Rand(0, 5);
	else
		m_variation = variation;
}

void TransitionPiecesShitpixels::apply(int i, class PhysicsObject& o)
{
	float y = (float)(int)(i / owner->divisions);
	float x = (float)(i % owner->divisions);
	o.setPos(Point3(Mapping::linear(x, 0.0f, (float)owner->divisions, owner->min.x, owner->max.x) + owner->pieceSize.x * 0.5f, owner->max.y, Mapping::linear(y, 0.0f, (float)owner->divisions, owner->min.z, owner->max.z) - owner->pieceSize.z * 0.5f));

	o.setGravityScale(0);

	float maxDist = (owner->max - owner->min).length() + owner->pieceSize.length() * 0.5f;

	Point3 vel;

	if (m_variation == 0)
	{
		vel = Point3(maxDist * sign(Rand(-1.0f, 1.0f)), 0, 0);
	}
	else if (m_variation == 1)
	{
		vel = Point3(0, 0, maxDist * sign(Rand(-1.0f, 1.0f)));
	}
	else if (m_variation == 2)
	{
		vel = Point3(maxDist * sign(Rand(-1.0f, 1.0f)), 0, maxDist * sign(Rand(-1.0f, 1.0f)));
	}
	else if (m_variation == 3)
	{
		vel = Point3(maxDist * sign(Rand(-1.0f, 1.0f)), 0, maxDist);
	}
	else if (m_variation == 4)
	{
		vel = Point3(maxDist, 0, maxDist * sign(Rand(-1.0f, 1.0f)));
	}

	o.setVel(vel / owner->duration());
}

////

IMPLEMENT_RTTI(TransitionPiecesSlivers);
//REGISTER_RTTI(TransitionPiecesSlivers);

void TransitionPiecesSlivers::apply(int i, class PhysicsObject& o)
{
	o.setPos(Point3(0, owner->max.y, Mapping::linear((float)i, 0.0f, (float)owner->divisions * owner->divisions, owner->max.z, owner->min.z)));

	o.setGravityScale(0);

	float delta = sign(Rand(-1.0f, 1.0f));

	float maxDist = (abs(owner->max.x - owner->min.x) + owner->pieceSize.x * 0.5f) + owner->pieceSize.length() * 0.5f;

	o.setVel(Point3(maxDist / owner->duration() * delta, 0, 0));
}

Point3 TransitionPiecesSlivers::pieceSize(Point3 min, Point3 max, float divisions)
{
	return Point3(abs(max.x - min.x), 0, abs(max.z - min.z) / (divisions * divisions));
}

