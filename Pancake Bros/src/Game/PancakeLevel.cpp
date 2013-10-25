// ------------------------------------------------------------------------------------------------
//
// PancakeLevel
//
// ------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "RSEApp.h"
#include "PancakeLevel.h"
#include "Physical.h"
#include "Man.h"
#include "Player.h"
#include "Hammer.h"
#include "AI/AI.h"
#include "Wave.h"
#include "Game.h"
#include "RSE/PathResource.h"
#include "RSE/Game/ObjectMgr.h"
#include "RSE/Game/Sprite.h"
#include "RSE/Physics/PhysPlane.h"
#include "RSE/Physics/PhysSphere.h"
#include "RSE/Physics/PhysicsMgr.h"
#include "RSE/Render/SDeviceD3D.h"
#include "RSE/Render/Scene.h"
#include "RSE/Render/SplitImage.h"
#include "RSE/Render/TextureAnimationManager.h"
#include "RSE/Render/ParticleFactory.h"
#include "RSE/Render/ParticleSystem.h"
#include "RSE/Render/ParticleEmitter.h"
#include "RSE/Sound/IMusic.h"
#include "RSE/Sound/ISoundProvider.h"
#include "RSE/Sound/MusicManager.h"
#include "Standard/FileMgr.h"
#include "Standard/Mapping.h"
#include "Standard/Rand.h"
#include "Standard/TGA.h"
#include "Standard/Exception/Filesystem.h"

IMPLEMENT_RTTI(PancakeLevel);

PancakeLevel::PancakeLevel(const std::string& name) :
	particles(App().particles()),
	configGame(App().configGame()),
	configLevel(App().configLevel()),
	configAI(App().configAI()),
	options(App().options()),
	m_done(false)
{
	setConfig(configLevel, name);

	scene().setClearCol(RGBA(0,0,0));

	dlog << "Loading level " << name << dlog.endl;

	if (!configLevel.exists(name))
		throwf("No level named '" + name + "' exists");

	if (!configLevel.exists(name, "Enemies"))
		throwf("No 'Enemies' data for level '" + name + "'");

	std::string waveData = configLevel.get<std::string>(name, std::string("Enemies")); // compilation error hack
	m_wave = new Wave(waveData);

	m_ordered.reserve(500);

	if (configLevel.exists(name, "Gravity"))
		m_gravity = configLevel.get<Point3>(name, "Gravity");
	else
		m_gravity = options.get<Point3>("World", "Gravity");

	if (configLevel.exists(name, "BoundMin"))
		m_boundMin = configLevel.get<Point3>(name, "BoundMin");
	else
		m_boundMin = options.get<Point3>("World", "BoundMin");

	if (configLevel.exists(name, "BoundMax"))
		m_boundMin = configLevel.get<Point3>(name, "BoundMin");
	else
		m_boundMax = options.get<Point3>("World", "BoundMax");

	m_bound = m_boundMax - m_boundMin;
	m_bound2D = m_bound;
	m_bound2D.z = 0;
	m_boundLength = m_bound.length();
	m_boundLength2D = m_bound2D.length();

	PhysPlane* plane;

	m_deadGroup = physicsMgr().addCollisionGroup();

	// floor
	plane = new PhysPlane(Point3(0, 0, 1), Point3::ZERO);
	plane->setParent(*this);
	plane->setMass(FLT_MAX);
	collisionGroup().add(plane);
	m_deadGroup->add(plane);
	floor = plane;

	// walls
	leftWall = new PhysPlane(Point3(1, 0, 0), boundMin());
	leftWall->setParent(*this);
	leftWall->setName("Left Wall");
	leftWall->setMass(FLT_MAX);
	rightWall = new PhysPlane(Point3(-1, 0, 0), boundMax());
	rightWall->setParent(*this);
	rightWall->setName("Right Wall");
	rightWall->setMass(FLT_MAX);
	backWall = new PhysPlane(Point3(0, -1, 0), boundMax());
	backWall->setParent(*this);
	backWall->setName("Back Wall");
	backWall->setMass(FLT_MAX);
	frontWall = new PhysPlane(Point3(0, 1, 0), boundMin());
	frontWall->setParent(*this);
	frontWall->setName("Front Wall");
	frontWall->setMass(FLT_MAX);
	collisionGroup().add(leftWall);
	m_deadGroup->add(leftWall);
	collisionGroup().add(rightWall);
	m_deadGroup->add(rightWall);
	collisionGroup().add(frontWall);
	m_deadGroup->add(frontWall);
	collisionGroup().add(backWall);
	m_deadGroup->add(backWall);

	// camera
	Camera& cam = scene().camera();
	if (configLevel.exists(name, "CameraPos"))
		cam.setPos(configLevel.get<Point3>(name, "CameraPos"));
	else
		cam.setPos(options.get<Point3>("Render", "CameraPos"));
	
	//float ffar = App().options().get<Point3>("World", "BoundMax").y;
	//float fnear = App().options().get<Point3>("World", "BoundMin").y;

	float zNear = -10000.0f;
	float zFar = 10000.0f;

	// set up projection so that 1 unit = 1 pixel at 0,0,0
	Matrix4 proj;
	proj(0, 0) = 2.0f * (1.0f / 640.0f);
	proj(1, 0) = 0.0f;
	proj(2, 0) = 0.0f;
	proj(3, 0) = 0.0f;
	proj(0, 1) = 0.0f;
	proj(1, 1) = 2.0f * (1.0f / 480.0f);
	proj(2, 1) = options.get("Render", "YScaleFactor", 0.0015f); // multiply resulting screen y coordinate by z * scalefactor (flattens the projection)
	proj(3, 1) = options.get("Render", "YShift", -0.45f); // shift y down
	proj(0, 2) = 0.0f;
	proj(1, 2) = 0.0f;
	proj(2, 2) = 1.0f / (zFar - zNear);
	proj(3, 2) = Mapping::linear(0.0f, zNear, zFar, 0.0f, 1.0f);
	proj(0, 3) = 0.0f;
	proj(1, 3) = 0.0f;
	proj(2, 3) = options.get("Render", "ScaleFactor", 0.0025f);
	proj(3, 3) = 1.0f; // orthographic: z coordinate does not affect x or y positions, so w should always be 1.0f

	// make matrix w-friendly
	/*proj(0, 0) /= proj(2, 3);
	proj(1, 1) /= proj(2, 3);
	proj(2, 2) /= proj(2, 3);
	proj(3, 2) /= proj(2, 3);
	proj(2, 3) = 1;*/

	cam.setProjection(proj);
	cam.setRot(QuatAngleAxis(degToRad(App().options().get<float>("Render", "CameraPitch")), Point3(-1, 0, 0)));

	PtrGC<PlayerMan> m = App().game().player(0);
	m->setParent(objectRoot());
	m->physics()->setName("Player");
	m->reset();

	Controller* p = new Player;
	p->setParent(*this);
	p->possess(m.ptr());

	try
	{
		PathResource background("media\\textures\\background\\" + configLevel.get<std::string>(name, "Background"));

		ibinstream stream(background.open(std::ios::in | std::ios::binary));

		TGA b;
		b.read(*background, stream);

		m_background = new SplitImage(b);
	}
	catch(ExceptionFilesystem&)
	{
		m_background = 0;
	}

	m_nextSpawnTime = (float)time() + 4.0f;

	// autolists
	registerAutoList<Physical>();
}

void PancakeLevel::construct()
{
	Super::construct();

	m_fakeShadowScaleY = get("FakeShadowScaleY", 0.5f);
	m_fakeShadowSkew = get("FakeShadowSkew", Point2(1,1));
	m_maxActiveEnemies = get("MaxActiveEnemies", 2);
	m_projectedShadows = get("ProjectedShadows", false);
	m_shadowColour = get("ShadowColour", RGBA(0, 0, 0, 0.5f));
	m_sunPosition = get("SunPosition", Point3(0, 1000, 0));
}


PancakeLevel::~PancakeLevel()
{
	delete m_wave;
	delete m_background;
}

struct ZSorter
{
	ZSorter(std::list<Object*>& _unordered, std::vector<Object*>& _ordered) :
		unordered(_unordered),
		ordered(_ordered)
	{
	}

	bool operator() (const Object* lhs, const Object* rhs) const
	{
		// reverse ordering
		return lhs->worldPos().y >= rhs->worldPos().y;
	}

	void recurse(Object* node)
	{
		if (Cast<Sprite>(node))
		{
			ordered.push_back(node);
		}
		else
		{
			unordered.push_back(node);
		}

		for (Object::ObjectList::const_iterator i = node->children().begin(); i != node->children().end(); ++i)
		{
			recurse(i->ptr());
		}
	}

	void order(Object* root)
	{
		for (Object::ObjectList::const_iterator i = root->children().begin(); i != root->children().end(); ++i)
		{
			recurse(i->ptr());
		}
	}

	std::list<Object*>& unordered;
	std::vector<Object*>& ordered;
};

void PancakeLevel::draw()
{
	Level::draw();
}

void PancakeLevel::drawScene()
{
	D3D().reset();
	D3D().texFilter(false, false);
	D3D().zbuffer(false);
	D3D().xformPixel();
	D3D().lighting(false);

	// draw background
	if (m_background)
		m_background->draw(Point2(0, 0));

	D3D().setContext(*m_context);
	scene().camera().draw();

	// make a list of objects that must be z-ordered, and other objects
	std::list<Object*> stdMethod;
	m_ordered.resize(0);
	ZSorter sorter(stdMethod, m_ordered);
	
	sorter.order(objectRoot().ptr());

	std::sort(m_ordered.begin(), m_ordered.end(), sorter);

	// draw unordered objects
	for (std::list<Object*>::iterator i = stdMethod.begin(); i != stdMethod.end(); ++i)
	{
		if (*i && (*i)->visible())
			(*i)->draw();
	}

	// draw ordered objects
	for (std::vector<Object*>::iterator j = m_ordered.begin(); j != m_ordered.end(); ++j)
	{
		if (*j && (*j)->visible())
			(*j)->draw();
	}
}

void PancakeLevel::update()
{
	if (AppBase().hasCmdLineOption("nobaddies"))
		m_wave->pause(FLT_MAX);

	m_activeEnemies.setDirty();

	// put this here to fix autolist update
	if (activeEnemies() < m_maxActiveEnemies)
	{
		if (m_temporaryWave)
		{
			m_temporaryWave->update((float)time().frame(), *this);
			
			if (m_temporaryWave->finished())
			{
				m_temporaryWave = 0;
				m_wave->play();
			}
		}

		m_wave->update((float)time().frame(), *this);
	}

	Level::update();

	particles.update();

	if (m_done)
		return;

	// player can be nulled because it actually is owned from outside the level, in game.
	// this is because the players are shared between levels.
	if (App().game().player(0) && App().game().player(0)->dead())
	{
		counters.set(App().configLevel().get("Options", "GameOverWait", 5.0f), delegate(&PancakeLevel::gameOver));
		m_done = true;
	}
	else
	{
		if (shouldLevelProgress())
		{
			counters.set(App().configLevel().get("Options", "EndLevelPancakeCollectionWait", 5.0f), delegate(&PancakeLevel::onLevelOver));
			m_done = true;
		}
	}
}

void PancakeLevel::onLevelOver()
{
	if (!App().options().get("Pancake Bros", "LevelProgression", true))
		return;

	// re-check condition -- maybe some new enemies have spawned
	if (shouldLevelProgress())
		App().game().levelOver();
	else
		m_done = false;
}

int PancakeLevel::activeEnemies()
{
	int n = 0;

	const Level::ObjectList& c = autoList<Physical>();
	for (Level::ObjectList::const_iterator i = c.begin(); i != c.end(); ++i)
	{
		Man* m = Cast<Man>(i->ptr());
		if (m && !m->dead() && m->enemy(App().game().player(0)))
			++n;
	}

	return n;
}

bool PancakeLevel::inBound(const Point3& p) const
{
	return p.x >= boundMin().x
		&& p.y >= boundMin().y
		&& p.z >= boundMin().z
		&& p.x <= boundMax().x
		&& p.y <= boundMax().y
		&& p.z <= boundMax().z;
}

void PancakeLevel::processParticle(ParticleEmitter& e, Particle& p)
{
	if (p.pos.y > boundMax().y)
	{
		p.pos.y = boundMax().y;
		p.vel.y *= -0.1f;
	}

	if (p.pos.z < 0)
	{
		p.pos.z = 0;
		p.lifeTime = 10;

		e.onImpact(particles, p);
		// draw splat here
	}
}

void PancakeLevel::gameOver()
{
	App().game().gameOver();
}

void PancakeLevel::nextLevel()
{
	App().game().nextLevel();
}

bool PancakeLevel::hasMusic()
{
	return !get<std::string>("Music").empty();
}

void PancakeLevel::begin()
{
	std::string music = get<std::string>("Music");
	if (!music.empty())
		App().music().play(PathResource("media\\music\\" + music));
}

void PancakeLevel::spawnCharacter(const std::string& type, int idx, const Point3& pos)
{
	if (!configGame.exists("character " + type))
		dlog << "No such enemy defined in game.ini: " << type << dlog.endl;
	else
	{
		Man* m = new BaddyMan(type);
		m->setParent(*this);
		m->setName(std::string("Baddy ") + idx + " (" + type + ")");
		m->physics()->setName(std::string("Baddy ") + idx);

		if (pos != Point3::MAX)
		{
			m->physics()->setPos(Point3(pos.x, pos.y, m->physics()->radius()));
		}
		else
		{
			if (!App().options().get<bool>("Pancake Bros", "EnableAI", true))
			{
				if (Rand() > 0.5f)
					m->physics()->setPos(Point3(boundMin().x * 0.5f - m->extent().x, Rand(boundMin().y, boundMax().y), m->physics()->pos().z));
				else
					m->physics()->setPos(Point3(boundMax().x * 0.5f + m->extent().x, Rand(boundMin().y, boundMax().y), m->physics()->pos().z));
			}
			else
			{
				// spawn on left or right side
				if (Rand() > 0.5f)
					m->physics()->setPos(Point3(boundMin().x - m->extent().x, Rand(boundMin().y, boundMax().y), m->physics()->pos().z));
				else
					m->physics()->setPos(Point3(boundMax().x + m->extent().x, Rand(boundMin().y, boundMax().y), m->physics()->pos().z));
			}
		}
	}
}

bool PancakeLevel::shouldLevelProgress()
{
	return !m_temporaryWave && m_wave->finished() && activeEnemies() == 0;
}

void PancakeLevel::setTemporaryWave(Wave* w, bool pauseLevelWave)
{
	m_temporaryWave = w;
	
	if (pauseLevelWave)
	{
		m_wave->pause(1000);
	}
	else
	{
		m_wave->play();
	}
}

std::vector<PhysPlane*> PancakeLevel::boundaryPlanes() const
{
    std::vector<PhysPlane*> result;
	
	result.push_back(floor);
	result.push_back(backWall);
	result.push_back(frontWall);
	result.push_back(leftWall);
	result.push_back(rightWall);
	
	return result;
}
