// ------------------------------------------------------------------------------------------------
//
// PancakeLevel
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/Game/Level.h"

class SplitImage;
class ParticleFactory;
class Config;
class Man;
class Wave;

class PancakeLevel : public Level
{
	USE_RTTI(PancakeLevel, Level);
public:
	PancakeLevel(const std::string& name);
	virtual ~PancakeLevel();

	virtual void construct();

	virtual void draw();

	virtual void update();

	virtual void begin();
	void gameOver();

	ParticleFactory& particles;
	Config& options;
	Config& configGame;
	Config& configLevel;
	Config& configAI;

	int activeEnemies();

	const Point3& boundMin() const { return m_boundMin; }
	const Point3& boundMax() const { return m_boundMax; }
	const Point3& bound() const { return m_bound; }
	const Point3& bound2D() const { return m_bound2D; }
	float boundLength() const { return m_boundLength; }
	float boundLength2D() const { return m_boundLength2D; }
	bool inBound(const Point3& p) const;
	const Point2& fakeShadowSkew() const { return m_fakeShadowSkew; }
	float fakeShadowScaleY() const { return m_fakeShadowScaleY; }
	bool projectedShadows() const { return m_projectedShadows; }
	const RGBA& shadowColour() const { return m_shadowColour; }
	const Point3& sunPosition() const { return m_sunPosition; }

	virtual bool hasMusic();

	virtual void setTemporaryWave(Wave* w, bool pauseLevelWave);

	// game-specific particle functions
	virtual void processParticle(class ParticleEmitter& e, class Particle& p);

	void spawnCharacter(const std::string& type, int idx, const Point3& pos = Point3::MAX);

	// environment
	class PhysPlane* floor;
	class PhysPlane* backWall;
	class PhysPlane* frontWall;
	class PhysPlane* leftWall;
	class PhysPlane* rightWall;

	std::vector<PhysPlane*> boundaryPlanes() const;

	CollisionGroup& deadGroup() const { return *m_deadGroup; }

protected:
	virtual void drawScene();
	void nextLevel();
	void onLevelOver();
	bool shouldLevelProgress();

	Point3 m_boundMin;
	Point3 m_boundMax;
	Point3 m_bound;
	Point3 m_bound2D;
	float m_boundLength;
	float m_boundLength2D;
	float m_nextSpawnTime;
	bool m_done;
	float m_fakeShadowScaleY;
	Point2 m_fakeShadowSkew;
	int m_maxActiveEnemies;
	bool m_projectedShadows;
	RGBA m_shadowColour;
	Point3 m_sunPosition;
	
	Dirty<int> m_activeEnemies;

	std::vector<class PhysPlane*> _boundaries;
	EmbeddedPtr<CollisionGroup>	m_deadGroup;
	EmbeddedPtr<CollisionGroup>	m_playersGroup;
	
	SmartPtr<Wave> m_temporaryWave; // The wave employed by the SpriteFXSpawnBaddyWave action.
	Wave* m_wave;
	SplitImage* m_background;
	std::vector<Object*> m_ordered;
};