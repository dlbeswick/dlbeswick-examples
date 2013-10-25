// ------------------------------------------------------------------------------------------------
//
// DlgParticles
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "DlgParticles.h"

#include "AppBase.h"
#include "Render/D3DPainter.h"
#include "Render/FontElement.h"
#include "Render/ParticleFactory.h"
#include "Render/ParticleSystem.h"
#include "Render/ParticleEmitter.h"
#include "Render/ParticleRenderer.h"
#include "Render/SFont.h"
#include "Render/TextureAnimationManager.h"
#include "UI/Dialogs/DlgMessageBox.h"
#include "UI/Dialogs/DlgObjProperties.h"
#include "UI/Controls/UIPropertyEditor.h"
#include "UI/Controls/UICombo.h"
#include "UI/Controls/UIButton.h"

REGISTER_RTTI_NAME(DlgParticles, "Particles");

DlgParticles::DlgParticles() :
	m_config(AppBase().pathConfig("particles.ini"))
{
};

void DlgParticles::onActivate()
{
	setFont(*Font().get("smallarial"));

	m_anim = new TextureAnimationManager("animations.ini");

	const PtrGC<ParticleRenderer>& p = level().particleRoot();
	PtrGC<ParticleRenderer> r = p;
	PtrGC<Object> o = p;

	m_system = new ParticleSystem;
	m_system->setParent(level().particleRoot());
	m_system->bNeverDestroy = true;
	m_system->setPos(Point3(0, 0, 0));

	m_properties = addChild(new DlgObjPropertiesPtrGC<Object>);
	m_properties->setCanUserClose(false);
	m_properties->setSize(Point2(0.45f, 0.7f));
	m_properties->setColour(RGBA(0,0,0,0.5f));
	m_properties->setName("Particle Emitter Properties");
	m_properties->onCommit = delegate(&DlgParticles::onCommit);

	// position camera such that one unit equals one pixel
	Camera& camera = level().scene().camera();
	camera.setPos(Point3(-160, -640, 50));

	UICombo* c;
	UIButton* b;

	float startX = 0.45f;
	float x = startX;
	float y = 0;

	// add system controls
	UILabel* label;
	label = addChild(new UILabel("System:"));
	label->setPos(Point2(x, y));
	y += label->size().y;

	c = addChild(new UICombo);
	c->setPos(Point2(x, y));
	c->setSize(Point2(0.2f, 0.5f));
	c->onSelect = delegate(&DlgParticles::onSelectSystem);
	m_systems = c;
	updateSystems();

	x += 0.275f;

	b = addChild(new UIButton);
	b->setPos(Point2(x, y));
	b->setText("Save");
	b->setSize(Point2(0.06f, 0.03f));
	b->onClick = delegate(&DlgParticles::onSaveSystem);

	x += 0.075f;

	b = addChild(new UIButton);
	b->setPos(Point2(x, y));
	b->setText("Del");
	b->setHelp("Delete the system and remove it from the particles file.");
	b->setSize(Point2(0.06f, 0.03f));
	b->onClick = delegate(&DlgParticles::onDelSystem);

	x += 0.075f;

	b = addChild(new UIButton);
	b->setPos(Point2(x, y));
	b->setText("New");
	b->setHelp("Make a new blank particle system.");
	b->setSize(Point2(0.06f, 0.03f));
	b->onClick = delegate(&DlgParticles::onNewSystem);

	// add emitter list and add/remove buttons
	x = c->pos().x + c->size().x + 0.075f;
	y += Font()->height();
	
	b = addChild(new UIButton);
	b->setPos(Point2(x, y));
	b->setText("Props");
	b->setHelp("Edit additional properties for this particle system.");
	b->setSize(Point2(0.06f, 0.03f));
	b->onClick = delegate(&DlgParticles::onSystemProperties);

	x += 0.075f;

	b = addChild(new UIButton);
	b->setPos(Point2(x, y));
	b->setText("Copy");
	b->setHelp("Create a copy of this particle system with a new name.");
	b->setSize(Point2(0.06f, 0.03f));
	b->onClick = delegate(&DlgParticles::onCopySystem);

	y += Font()->height();
	x = startX;

	label = addChild(new UILabel("Emitter:"));
	label->setPos(Point2(x, y));
	y += label->size().y;

	c = addChild(new UICombo);
	c->setPos(Point2(x, y));
	c->setSize(Point2(0.2f, 0.2f));
	c->onSelect = delegate(&DlgParticles::onSelectEmitter);
	m_emitters = c;

	onNewSystem();

	x += 0.275f;

	b = addChild(new UIButton);
	b->setPos(Point2(x, y));
	b->setText("-");
	b->setSize(Point2(0.03f, 0.03f));
	b->onClick = delegate(&DlgParticles::onDelEmitter);

	x += 0.05f;

	b = addChild(new UIButton);
	b->setPos(Point2(x, y));
	b->setText("+");
	b->setSize(Point2(0.03f, 0.03f));
	b->onClick = delegate(&DlgParticles::onAddEmitter);
}

DlgParticles::~DlgParticles()
{
	if (AppBase().isRunning())
		AppBase().particles().reload();
}

void DlgParticles::setup(ParticleEmitter& emitter)
{
	m_lastAnim = "";

	m_emitter = &emitter;
	m_system->setScale(Point3(1, 1, 1));

	m_properties->clear();

	PropertyEditorList variables;

    // get list of animations
	std::vector<std::string> animations;
	animations.push_back("");
	TextureAnimationSet& set = m_anim->set("particles");
	for (TextureAnimationSet::Sequences::iterator i = set.sequences.begin(); i != set.sequences.end(); ++i)
		animations.push_back(i->first);

    // get list of systems
	std::vector<std::string> systemNames;
	systemNames.push_back("");
	const Config::Categories& c = m_config.categories();
	for (Config::Categories::const_iterator i = c.begin(); i != c.end(); ++i)
	{
		systemNames.push_back(i->category->name);
	}

	// fix: old style code
	variables.push_back(propertyCombo("Anim Name", emitter.animationName, "Name of the texture animation used to make the particles in this emitter.", animations));
	variables.push_back(propertyCombo("Impact Spawn System", emitter.impactSpawnSystemName, "If not blank, the given particle system will be spawned when a particle from this emitter impacts something (i.e. the world floor).", systemNames));
    emitter.editableProperties(variables);

	m_properties->concatenate(variables);
	// gross -- in the interest of reducing dependencies
	((DlgObjPropertiesPtrGC<Object>&)*m_properties).set(&emitter, false);

	std::string systemName = m_systemName;
	if (systemName.empty())
		systemName = "Unnamed System";

	std::string emitterName = emitter.description;
	if (emitterName.empty())
		emitterName = std::string("Emitter ") + m_emitters->selectedIdx();

	m_properties->setTitle(systemName + " - " + emitterName);
}

void DlgParticles::onAnim(const std::string& s)
{
	m_emitter->setAnim(m_anim->set("particles"), s);
}

void DlgParticles::onDrawScene()
{
	Super::onDrawScene();
	/*D3DPaint().setFill(RGBA(0.0f, 1.0f, 0.0f));
	D3DPaint().plane(Plane(Point3(0, 0, -1), Point3(0, 0, 0)), 250);
	D3DPaint().draw();*/
}

void DlgParticles::onAddEmitter()
{
	m_emitter = new ParticleEmitter;
	m_emitter->setParent(m_system);

	onAnim("blood1");
	setup(*m_emitter);
	updateEmitters();
	m_emitters->select(m_emitters->list().numItems());
}

void DlgParticles::onDelEmitter()
{
	if (m_emitters->selected())
	{
		m_emitter->setParent(0);
		m_emitter.destroy();
	}

	updateEmitters();
}

void DlgParticles::updateEmitters()
{
	int lastSel = m_emitters->selectedIdx();

	m_emitters->clear();

	uint idx = 0;
	const_cast<ParticleSystem::ObjectList&>(m_system->children()).flush();
	for (ParticleSystem::ObjectList::const_iterator i = m_system->children().begin(); i != m_system->children().end(); ++i, ++idx)
	{
		ParticleEmitter& e = *Cast<ParticleEmitter>(*i);
		std::string description = e.description;
		if (description.empty())
			description = "Emitter";

		m_emitters->add(new ListText(std::string("(") + idx + std::string(") ") + description, itostr(idx)));
	}

	m_emitters->select(lastSel);
	onSelectEmitter(0);
}

void DlgParticles::onSelectEmitter(const PtrGC<UIElement>&)
{
	if (m_system->children().size())
		setup(*Cast<ParticleEmitter>(m_system->children()[m_emitters->selectedIdx()]));
}

void DlgParticles::onCopySystem()
{
	addChild(new DlgInputBox<std::string>("Copy " + m_systemName + " -- enter name of new system", "NewSystem", makeDelegate<DlgParticles, std::string>(this, &DlgParticles::onCopySystemOk)));
}

void DlgParticles::onCopySystemOk(std::string s)
{
	onSaveSystemOk(s);
	m_systems->selectText(m_systemName);
	onSelectSystem(m_systems->selected());
}

void DlgParticles::onSaveSystem()
{
	if (m_systemName.empty())
		addChild(new DlgInputBox<std::string>("System Name", "NewSystem", delegate(&DlgParticles::onSaveSystemOk)));
	else
		onSaveSystemOk(m_systemName);
}

void DlgParticles::onSaveSystemOk(std::string s)
{
	m_systemName = s;
	if (m_systemName.empty())
	{
		addChild(new DlgMessageBox("Error", "You must enter the \"System Name\" before the particle system can be saved"));
		return;
	}

	m_system->setName(s);
	m_system->saveToConfig(m_config);

	otextstream particlesIniOut(AppBase().pathConfig("particles.ini").open(std::ios::out));
	m_config.services()[0]->save(particlesIniOut);

	updateSystems();
}

void DlgParticles::onDelSystem()
{
	m_config.remove(m_systemName);
	updateSystems();
}

void DlgParticles::onNewSystem()
{
	m_system.destroy();
	m_system = new ParticleSystem;
	m_system->setParent(level().particleRoot());
	m_system->bNeverDestroy = true;
	m_system->setPos(Point3(0, 0, 1));

	m_systemName = "";

	m_systems->select(0);

	onAddEmitter();
}

void DlgParticles::onSelectSystem(const PtrGC<UIElement>& item)
{
	if (!item || Cast<ListText>(item)->text() == "")
		return;

	m_system.destroy();
	
	m_systemName = Cast<ListText>(item)->text();

	if (!m_systemName.empty())
	{
		m_system = new ParticleSystem(m_config, m_anim->set("particles"), m_systemName);
	}
	else
	{
		m_system = new ParticleSystem;
	}

	m_system->setParent(level().particleRoot());
	
	m_system->bNeverDestroy = true;
	m_system->setPos(Point3(0, 0, 1));

	updateEmitters();
	m_emitters->select(0);
	onSelectEmitter(m_emitters->list().item(0));

	if (m_systemProperties)
		// gross -- in the interest of reducing dependencies
		((DlgObjPropertiesPtrGC<Object>&)*m_systemProperties).set(m_system);
}

void DlgParticles::updateSystems()
{
	std::string sel;
	if (m_systems->selected())
		sel = m_systems->selected()->text();

	m_systems->clear();
	m_systems->add(new ListText(""));

	int count = 0;

	const Config::Categories& c = m_config.categories();
	for (Config::Categories::const_iterator i = c.begin(); i != c.end(); ++i)
	{
		m_systems->add(new ListText(i->category->name));
		++count;
	}

	m_systems->selectText(sel);
}

void DlgParticles::update(float delta)
{
	DlgStandard::update(delta);

	// fix?
	if (m_emitter && m_lastAnim != m_emitter->animationName)
		onAnim(m_emitter->animationName);

	// do splat callback
	if (m_system)
	{
		ParticleSystem& s = *m_system;
		for (ParticleSystem::ObjectList::const_iterator j = s.children().begin(); j < s.children().end(); ++j)
		{
			PtrGC<ParticleEmitter> e = Cast<ParticleEmitter>(*j);
			if (e && !e->impactSpawnSystemName.empty())
			{
				for (uint k = 0; k < e->particles().size(); ++k)
				{
					Particle& p = (Particle&)e->particles()[k];
					if (p.pos.z < 0)
					{
						p.pos.z = 0;
						e->deactivateParticle(p);
						if (!e->impactSpawnSystemName.empty())
						{
							ParticleSystem* newSystem = new ParticleSystem(m_config, m_anim->set("particles"), e->impactSpawnSystemName);
							newSystem->setParent(level().particleRoot());
							newSystem->setPos(p.pos);
							newSystem->setScale(Point3(1, 1, 1));
						}
					}
				}
			}
		}
	}
}

void DlgParticles::onCommit()
{
	if (m_properties->changed("Desc"))
		updateEmitters();
}

void DlgParticles::onSystemProperties()
{
	if (!m_systemProperties)
	{
		m_systemProperties = addChild(new DlgObjPropertiesPtrGC<Object>);
		m_systemProperties->setName("System Properties");
		m_systemProperties->setSize(Point2(0.5f, 0.5f));
	}

	// gross -- in the interest of reducing dependencies
	((DlgObjPropertiesPtrGC<Object>&)*m_systemProperties).set(m_system);
}
