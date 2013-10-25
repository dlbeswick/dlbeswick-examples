// ------------------------------------------------------------------------------------------------
//
// DlgSpriteFX
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "DlgSpriteFX.h"
#include "AppBase.h"
#include <Game/SpriteFX.h>
#include <Game/Sprite.h>
#include <Render/D3DPainter.h>
#include <Render/FontElement.h>
#include <Render/SFont.h>
#include <Render/TextureAnimator.h>
#include <Render/TextureAnimationManager.h>
#include <Standard/Collide.h>
#include <Standard/Config.h>
#include <Standard/Exception/Filesystem.h>
#include <UI/DialogFrame.h>
#include <UI/DialogMgr.h>
#include <UI/Controls/UIButton.h>
#include <UI/Controls/UICombo.h>
#include <UI/Controls/UIListView.h>
#include "UI/Controls/UIShuttle.h"
#include <UI/Dialogs/DlgMessageBox.h>
#include <UI/Dialogs/DlgObjProperties.h>

class UISpriteDisplay : public UIElement
{
	USE_RTTI(UISpriteDisplay, UIElement);
public:
	UISpriteDisplay(TextureAnimationManager& mgr, SpriteFXFactory& factory);
	~UISpriteDisplay();

	virtual void setSize(const Point2& reqSize);

	void setAnim(const std::string& set, const std::string& sequence);
	void enableRotMarker(bool b) { m_bPosMarker = b; }
	void enablePosMarker(bool b) { m_bRotMarker = b; }
	void setMarkerPos(const Point3& pos) { m_markerPos = pos; }
	void setMarkerRot(const Quat& rot) { m_markerRot = rot; }

	const Point3& markerPos() const { return m_markerPos; }
	const Quat& markerRot() const { return m_markerRot; }
	Sprite& sprite() { return *m_sprite; }
	SpriteFXPlayer& spriteFX() { return ((SpriteFXAnimator&)m_sprite->animator()).spriteFX(); }

	Delegate onFrame;
	Delegate onMarker;

protected:
	virtual void onDraw();
	virtual void update(float delta);
	virtual bool onMouseDown(const Point2& p, int button) { captureMouse(); return true; }
	virtual bool onMousePressed(const Point2& p, int button);
	virtual bool onMouseUp(const Point2& p, int button) { releaseMouse(); return true; }
	void onMagIn();
	void onMagOut();
	void onFirst();
	void onBack();
	void onPlay();
	void onPause();
	void onForward();
	void onLast();
	void resetCamera();

	Sprite* m_sprite;
	SpriteFXFactory& m_factory;
	TextureAnimationManager& m_mgr;
	UIButton* m_magIn;
	UIButton* m_magOut;
	UIShuttle* m_shuttle;
	Level* m_level;
	bool m_bPosMarker;
	bool m_bRotMarker;
	Point3 m_markerPos;
	Quat m_markerRot;

	int m_oldFrame;
};

IMPLEMENT_RTTI(UISpriteDisplay);

UISpriteDisplay::UISpriteDisplay(TextureAnimationManager& mgr, SpriteFXFactory& factory) :
	m_factory(factory),
	m_mgr(mgr),
	m_markerPos(0, 0, 0),
	m_markerRot(1, 0, 0, 0)
{
	setFont(*Font().get("smallarial"));

	setHelp("Left-click on the sprite to set the effect's position. Right-click to affect rotation.");
	
	m_magOut = addChild(new UIButton);
	m_magOut->setText("Zoom -");
	m_magOut->onClick = delegate(&UISpriteDisplay::onMagOut);
	m_magIn = addChild(new UIButton);
	m_magIn->setText("Zoom +");
	m_magIn->onClick = delegate(&UISpriteDisplay::onMagIn);
	
	m_shuttle = addChild(new UIShuttle);
	m_shuttle->onFirst = delegate(&UISpriteDisplay::onFirst);
	m_shuttle->onBack = delegate(&UISpriteDisplay::onBack);
	m_shuttle->onPlay = delegate(&UISpriteDisplay::onPlay);
	m_shuttle->onPause = delegate(&UISpriteDisplay::onPause);
	m_shuttle->onForward = delegate(&UISpriteDisplay::onForward);
	m_shuttle->onLast = delegate(&UISpriteDisplay::onLast);

	m_level = new Level;
	AppBase().addLevel(*m_level);
	m_level->scene().setClear(false);
	m_sprite = new Sprite(mgr.set(""));
	m_sprite->setParent(*m_level);
	m_sprite->setDrawScale(512.0f);
	m_sprite->animator().stop();

	m_bPosMarker = false;
	m_bRotMarker = false;

	resetCamera();
}

UISpriteDisplay::~UISpriteDisplay()
{
	delete m_level;
}

void UISpriteDisplay::onDraw()
{
	D3DPaint().setFill(colour());
	D3DPaint().quad2D(0, 0, size().x, size().y);
	D3DPaint().draw();

	m_level->draw();
	// draw pos marker
	if (m_bPosMarker)
	{
		D3DPaint().setFill(RGBA(0,1,0,0.75f));
		D3DPaint().line(m_markerPos + Point3(-8, 0, 0), m_markerPos + Point3(8, 0, 0));
		D3DPaint().line(m_markerPos + Point3(0, 0, 8), m_markerPos + Point3(0, 0, -8));
		D3DPaint().draw();
	}

	DialogMgr().beginDraw();

	// draw rot marker
	if (m_bRotMarker)
	{
		Point3 verts[4];
		verts[0] = Point3::ZERO;
		verts[1] = Point3(0, 0, 16);
		verts[2] = Point3(-4, 0, 12);
		verts[3] = Point3(4, 0, 12);

		Matrix4 xform;
		markerRot().toMatrix(xform);
		xform.translation(m_markerPos);

		for (uint i = 0; i < 4; ++i)
			verts[i] *= xform;

		D3DPaint().setFill(RGBA(0,1,0,0.75f));
		D3DPaint().line(verts[0], verts[1]);
		D3DPaint().line(verts[1], verts[2]);
		D3DPaint().line(verts[1], verts[3]);
		D3DPaint().draw();
	}

	font().write(D3DPaint(), std::string("Frame ") + (int)m_sprite->animator().frameIdx(), 0, 0);
	D3DPaint().draw();
}

void UISpriteDisplay::update(float delta)
{
	Super::update(delta);
	m_level->update();
}

void UISpriteDisplay::resetCamera()
{
	Camera& c = m_level->scene().camera();

	c.setProjection(Camera::matrixProjectionPerspectiveFOV(PI/2, 1.0f, 1.0f, 1000.0f));
	
	float spriteSize = std::max(m_sprite->size().x, m_sprite->size().z * 1.3333f);
	spriteSize = std::max(spriteSize, 128.0f); // tbd: use correct sprite display metric function (which is also tbd)

	c.setPos(Point3(0, -spriteSize, 0));
}

void UISpriteDisplay::setSize(const Point2& reqSize)
{
	m_magIn->setPos(Point2(0, reqSize.y - m_magIn->size().y));
	m_magOut->setPos(m_magIn->pos() + Point2(m_magIn->size().x, 0));
	m_level->scene().camera().setViewport(screenPos(), Point2(screenPos().x + reqSize.x, screenPos().y + reqSize.y * 1.3333f));
	m_shuttle->setSize(Point2(reqSize.x * 0.5f, reqSize.y * 0.1f));
	m_shuttle->setPos(reqSize - m_shuttle->size());
	Super::setSize(reqSize);
}

void UISpriteDisplay::setAnim(const std::string& set, const std::string& sequence)
{
	bool wasPlaying = m_sprite->animator().playing();

	m_sprite->animator().useSet(m_mgr.set(set));
	m_sprite->animator().play(sequence, true);

	// update spritefx so that new spritefx set is loaded
	spriteFX().update(m_sprite->animator());

	if (!wasPlaying)
		m_sprite->animator().stop();

	resetCamera();
}

void UISpriteDisplay::onMagIn()
{
	m_level->scene().camera().setPos(m_level->scene().camera().pos() + Point3(0, 50, 0));
}

void UISpriteDisplay::onMagOut()
{
	m_level->scene().camera().setPos(m_level->scene().camera().pos() + Point3(0, -50, 0));
}

void UISpriteDisplay::onFirst()
{
	m_sprite->animator().setFrame(0);
	onFrame();
}

void UISpriteDisplay::onBack()
{
	m_sprite->animator().setFrame((int)m_sprite->animator().frameIdx() - 1);
	onFrame();
}

void UISpriteDisplay::onPlay()
{
	m_oldFrame = (int)m_sprite->animator().frameIdx();
	m_sprite->animator().start();
}

void UISpriteDisplay::onPause()
{
	m_sprite->animator().stop();
	m_sprite->animator().setFrame(m_oldFrame);
	onFrame();
}

void UISpriteDisplay::onForward()
{
	m_sprite->animator().setFrame((int)m_sprite->animator().frameIdx() + 1);
	onFrame();
}

void UISpriteDisplay::onLast()
{
	m_sprite->animator().setFrame(m_sprite->animator().frames());
	onFrame();
}

bool UISpriteDisplay::onMousePressed(const Point2& p, int button)
{
	if (!m_bPosMarker && !m_bRotMarker)
		return true;

	Camera& cam = m_level->scene().camera();
	Point3 newPoint = cam.screenToWorld(Point3(p.x / size().x, p.y / size().y * 0.75f, -cam.pos().y));

	if (button == 0)
	{
		setMarkerPos(newPoint);
	}
	else if (button == 1)
	{
		setMarkerRot(QuatFromVector(Point3(0, 0, 1), newPoint.normal()));
	}
	
	onMarker();

	return true;
}

// DlgSpriteFX ///////////////////////////////////////////////////////////////////////////

REGISTER_RTTI_NAME(DlgSpriteFX, "Sprite FX Editor");

DlgSpriteFX::DlgSpriteFX()
{
}

void DlgSpriteFX::construct()
{
	Super::construct();

	setName("Sprite FX");

	setFont("smallarial");

	m_display = addChild(new UISpriteDisplay(AppBase().textureAnimation(), AppBase().spriteFX()));
	m_display->setPos(Point2(0.001f, 0.001f));
	m_display->setSize(Point2(0.45f, 0.45f));
	m_display->onFrame = delegate(&DlgSpriteFX::onFrame);
	m_display->onMarker = delegate(&DlgSpriteFX::onDisplayMarker);

	float x = 0.5f;
	float y = 0;

	m_animSets = addChild(new UICombo);
	m_animSets->setPos(Point2(x, y));
	m_animSets->setSize(Point2(1 - x, 0.35f));
	m_animSets->onSelect = delegate(&DlgSpriteFX::onAnimSet);

	y += font().height() + 0.01f;

	m_animSequences = addChild(new UICombo);
	m_animSequences->setPos(Point2(x, y));
	m_animSequences->setSize(Point2(1 - x, 0.35f));
	m_animSequences->onSelect = delegate(&DlgSpriteFX::onAnimSequence);

    // fill animsets
	std::vector<std::string> elements;
	const TextureAnimationManager::Sets& sets = AppBase().textureAnimation().sets();
	for (TextureAnimationManager::Sets::const_iterator i = sets.begin(); i != sets.end(); ++i)
		elements.push_back(i->first);
	std::sort(elements.begin(), elements.end());
	for (uint i = 0; i < elements.size(); ++i)
		m_animSets->add(new ListText(elements[i]));

	y += font().height() + 0.025f;

	m_fx = addChild(new UIListView);
	m_fx->setPos(Point2(x, y));
	m_fx->setSize(Point2(1 - x, 0.2f));
	m_fx->onSelect = delegate(&DlgSpriteFX::onFX);

	y += 0.205f;

	m_addFX = addChild(new UIButton);
	m_addFX->setPos(Point2(x, y));
	m_addFX->setText("Add Effect");
	m_addFX->onClick = delegate(&DlgSpriteFX::onAddFX);
	m_delFX = addChild(new UIButton);
	m_delFX->setPos(Point2(x + m_addFX->size().x + 0.01f, y));
	m_delFX->setText("Del Effect");
	m_delFX->onClick = delegate(&DlgSpriteFX::onDelFX);

	y += m_addFX->size().y + 0.025f;

	m_fxProperties = addChild(new DlgObjProperties);
	m_fxProperties->setPos(Point2(x, y));
	m_fxProperties->setSize(Point2(1 - x, 0.6f - y - 0.01f));
	m_fxProperties->setColour(RGBA(0,0,0,0.5f));
	m_fxProperties->setFrame(DialogFrameNaked::static_rtti());
	m_fxProperties->onCommit = delegate(&DlgSpriteFX::onCommit);

	y += m_fxProperties->size().y + 0.025f;

	m_save = addChild(new UIButton);
	m_save->setText("Save");
	m_save->onClick = delegate(&DlgSpriteFX::onSave);
	m_save->setPos(Point2(x, y));

	// initial values
	m_animSets->selectText(get<std::string>("CurrentSet"));
	onAnimSet(m_animSets->selected());
	m_animSequences->selectText(get<std::string>("CurrentSequence"));
	onAnimSequence(m_animSequences->selected());
	m_display->sprite().animator().setFrame(get<int>("CurrentFrame"));
	onFrame();
}

DlgSpriteFX::~DlgSpriteFX()
{
	set("CurrentSet", m_animSets->selected()->text());
	set("CurrentSequence", m_animSequences->selected()->text());
	set("CurrentFrame", (int)m_display->sprite().animator().frameIdx());
}

void DlgSpriteFX::onAnimSet(const PtrGC<UIElement>& u)
{
	fillSequence(Cast<ListText>(u)->text());
	onAnimSequence(m_animSequences->selected());
}

void DlgSpriteFX::onAnimSequence(const PtrGC<UIElement>& u)
{
	std::string set = m_animSets->selected()->text();
	std::string seq = u.downcast<ListText>()->text();

	m_display->setAnim(set, seq);
	m_display->sprite().update(0.00001f);

	// load sprite fx objects
	if (m_display->spriteFX().current())
		m_currentFX = m_display->spriteFX().current();
	else
		m_currentFX = 0;

	onFrame();
}

void DlgSpriteFX::fillSequence(const std::string& setName)
{
	if (!m_animSets->selected())
		return;

	m_animSequences->clear();

	const TextureAnimationSet& s = *AppBase().textureAnimation().sets().find(setName)->second;

	std::vector<std::string> elements;
	const TextureAnimationSet::Sequences& sequences = s.sequences;
	for (TextureAnimationSet::Sequences::const_iterator i = sequences.begin(); i != sequences.end(); ++i)
		elements.push_back(i->first);
	std::sort(elements.begin(), elements.end());
	for (uint i = 0; i < elements.size(); ++i)
		m_animSequences->add(new ListText(elements[i]));
}

void DlgSpriteFX::onFX(const PtrGC<UIElement>& u)
{
	if (m_fx->selectedIdx() == -1)
	{
		m_fxProperties->clear();
		m_display->enablePosMarker(false);
		m_display->enableRotMarker(false);
	}
	else
	{
		SpriteFX* s = (*m_currentFX)[atoi(u.downcast<ListText>()->data().c_str())];
		PropertyEditorList l;
		s->editableProperties(l);
		m_fxProperties->set(l);

		if (s)
		{
			m_display->enablePosMarker(s->pos() != Point2::MAX);
			m_display->enableRotMarker(s->rot() != Quat::NONE);
			m_display->setMarkerPos(Point3(s->pos().x, 0, s->pos().y));
			m_display->setMarkerRot(s->rot());
		}
		else
		{
			m_display->enablePosMarker(false);
			m_display->enableRotMarker(false);
		}
	}
}

void DlgSpriteFX::onFrame()
{
	m_fx->clear();

	if (!m_currentFX || m_currentFX->empty())
	{
		onFX(0);
		return;
	}

	for (uint i = 0; i < m_currentFX->size(); ++i)
	{
		SpriteFX& fx = *(*m_currentFX)[i];
		if (fx.frame == (int)m_display->sprite().animator().frameIdx())
		{
			ListText* text = new ListText(fx.rtti().className(), std::string("")+ i);
			text->setHelp(fx.help());
			m_fx->add(text);
		}
	}
	onFX(m_fx->selected());
}

void DlgSpriteFX::onSave()
{
	if (m_currentFX)
	{
		try
		{
			AppBase().spriteFX().update(*m_currentFX, m_animSets->selected()->text(), m_animSequences->selected()->text());
			AppBase().spriteFX().writeToConfig();
		}
		catch (ExceptionFilesystem& e)
		{
			addChild(new DlgMessageBox("Couldn't save config file.", e.what()));
		}
	}
}

void DlgSpriteFX::onCommit()
{
	m_display->spriteFX().retrigger(m_display->sprite().animator());
}

void DlgSpriteFX::onAddFX()
{
	if (!m_currentFX)
		return;

	RegistrantsList classes;
	std::vector<std::string> names;
	registrantsOfClass(SpriteFX::static_rtti(), classes);
	for (RegistrantsList::const_iterator i = classes.begin(); i != classes.end(); ++i)
	{
		names.push_back((*i)->name);
	}
	addChild(new DlgInputListBox("Select effect class", names, delegate(&DlgSpriteFX::onConstructFX)));
}

void DlgSpriteFX::onDelFX()
{
	if (!m_currentFX || !m_fx->selected())
		return;

	int idx = atoi(m_fx->selected()->data().c_str());

	(*m_currentFX)[idx]->stop(m_display->spriteFX());
	delete (*m_currentFX)[idx];
	m_currentFX->erase(m_currentFX->begin() + idx);
	m_display->spriteFX().retrigger(m_display->sprite().animator());
	m_fx->remove(idx);
}

void DlgSpriteFX::onConstructFX(std::string desc)
{
	if (!m_currentFX)
		return;

	SpriteFX* s = Cast<SpriteFX>(Base::newObjectByDesc(desc.c_str()));
	s->frame = (int)m_display->sprite().animator().frameIdx();

	m_currentFX->push_back(s);
	m_display->spriteFX().retrigger(m_display->sprite().animator());
	onFrame();

	m_fx->select(m_fx->numItems()-1);
	onFX(m_fx->selected());
}

void DlgSpriteFX::onDisplayMarker()
{
	if (m_fx->selected())
	{
		SpriteFX* s = (*m_currentFX)[atoi(m_fx->selected()->data().c_str())];
		s->setPos(Point2(m_display->markerPos().x, m_display->markerPos().z));
		s->setRot(m_display->markerRot());
		m_fxProperties->refresh();
		s->stop(m_display->spriteFX());
		s->start(m_display->spriteFX());
	}
}
