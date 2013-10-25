// ------------------------------------------------------------------------------------------------
//
// TransitionPieces
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "UITransition.h"

#if _DEBUG
#define DIVISIONS 8
#else
//#define DIVISIONS 14
#define DIVISIONS 8
#endif


class RSE_API TransitionPieces : public TransitionRenderToTexture
{
	USE_RTTI(TransitionPieces, TransitionRenderToTexture);
public:
	TransitionPieces(const PtrGC<UIElement>& src, float time, class TransitionPiecesFunction* function = 0, int _divisions = DIVISIONS);
	~TransitionPieces();

	virtual void draw();
	virtual void update(float delta);

	virtual bool finished();
	virtual bool needsPerFrameRenders();

	Point3 min;
	Point3 max;
	Point3 pieceSize;
	int divisions;

protected:
	virtual void onRendered();
//	virtual bool needsPerFrameRenders() { return false; }

	TransitionPiecesFunction* m_function;
	class Level* m_level;
	class MaterialTexture* m_material;

	typedef std::vector<class Piece*> Pieces;
	Pieces m_pieces;
};

class RSE_API TransitionPiecesFunction : public Base
{
	USE_RTTI(TransitionPiecesFunction, Base);
public:
	virtual void apply(int i, class PhysicsObject& o) = 0;
	virtual bool needsPerFrameRenders() { return false; }
	virtual Point3 pieceSize(Point3 min, Point3 max, float divisions);

	TransitionPieces* owner;
};

class RSE_API TransitionPiecesExplode : public TransitionPiecesFunction
{
	USE_RTTI(TransitionPiecesExplode, TransitionPiecesFunction);
public:
	TransitionPiecesExplode();
	TransitionPiecesExplode(const Point3& origin, float force, float radius, const Point3& additionalImpulse);

	virtual void apply(int i, class PhysicsObject& o);

protected:
	Point3 m_origin;
	float m_force;
	float m_radius;
	Point3 m_additionalImpulse;
};

class RSE_API TransitionPiecesShitpixels : public TransitionPiecesFunction
{
	USE_RTTI(TransitionPiecesShitpixels, TransitionPiecesFunction);
public:
	TransitionPiecesShitpixels(int variation = -1);
	virtual void apply(int i, class PhysicsObject& o);

protected:
	int m_variation;
};

class RSE_API TransitionPiecesSlivers : public TransitionPiecesFunction
{
	USE_RTTI(TransitionPiecesSlivers, TransitionPiecesFunction);
public:
	virtual void apply(int i, class PhysicsObject& o);
	virtual Point3 pieceSize(Point3 min, Point3 max, float divisions);

private:
};