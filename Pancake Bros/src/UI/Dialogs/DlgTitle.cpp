// ------------------------------------------------------------------------------------------------
//
// DlgTitle
//
// ------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "DlgTitle.h"
#include "RSEApp.h"
#include "Parallax.h"
#include "Game/Game.h"
#include "RSE/Render/D3DPainter.h"
#include "RSE/Render/SDeviceD3D.h"
#include "RSE/Render/SplitImage.h"
#include "RSE/Render/TextureAnimator.h"
#include "RSE/Render/Materials/MaterialAnimation.h"
#include "RSE/Sound/MusicManager.h"
#include "RSE/UI/UILayout.h"
#include "RSE/UI/Controls/UIButton.h"
#include "RSE/UI/Controls/UIKeyboardMenu.h"
#include "RSE/UI/Controls/UITextBox.h"
#include "RSE/UI/Controls/UIPic.h"
#include "RSE/UI/DialogMgr.h"
#include "RSE/UI/Dialogs/DlgDefaultOptions.h"
#include "RSE/UI/Transitions/TransitionFade.h"
#include "RSE/UI/Transitions/TransitionPieces.h"
#include "Standard/TGA.h"

REGISTER_RTTI(DlgTitle);

DlgTitle::DlgTitle()
{
}

void DlgTitle::construct()
{
	Super::construct();

	setFrame(registrant("DialogFrameNaked")->rtti);

	counters.set(0.1f, delegate(&DlgTitle::fart));

	App().music().loadPlaylist("title", true);
	App().music().startPlaylist();

	UIKeyboardMenu* menu = new UIKeyboardMenu;
	addChild(menu);

	UILayoutTable* layout = new UILayoutTable;
	addChild(layout);
	layout->setRowSpacing(0.05f);
	layout->setPos(Point2(0.0f, 0.45f));
	layout->setSize(Point2(1.0f, 0.4f));

	UIButton* b;
	b = new UIButton;
	layout->addChild(b);
	b->setText("Start");
	b->onClick = delegate(&DlgTitle::onStart);
	menu->add(b);

	b = new UIButton;
	layout->addChild(b);
	b->setText("Options");
	b->onClick = delegate(&DlgTitle::onOptions);
	menu->add(b);

	b = new UIButton;
	layout->addChild(b);
	b->setText("Quit");
	b->onClick = delegate(&DlgTitle::onQuit);
	menu->add(b);

	setColour(RGBA(0,0,0,0));

	setPointer(0);

	_parallax = new Parallax(get<std::string>("Parallax"));

	const TGA src(PathResource(get<std::string>("LogoTexturePath")));
	TGA tga;
	tga.create(src.size().x, src.size().y, Image::ARGB32);

	src.copy(tga);
	tga.colourKey(RGBA(1, 0, 1), RGBA(0, 0, 0, 0));

	_splitImagePancake = new SplitImage(tga);

	_logoBrosMaterial = get<SmartPtr<Material> >("LogoBrosMaterial");
	_picBros = new UIPic(*_logoBrosMaterial);
	addChild(_picBros);
	_picBros->setWholePixelPositioning(true);
}

DlgTitle::~DlgTitle()
{
}

void DlgTitle::fart()
{
	static int j = 0;

	++j;
	if (j > 60 / 0.1)
	{
		//s = "MY COCK\nSTINKS";
	}

	counters.set(0.1f, delegate(&DlgTitle::fart));
}

void DlgTitle::onStart()
{
	App().game().startGame();
	self().destroy();
}

void DlgTitle::onQuit()
{
	App().exit();
}

void DlgTitle::onOptions()
{
	DlgDefaultOptionsKeyboard* dlg = uiBranch()->addChild(new DlgDefaultOptionsKeyboard);
	dlg->setColour(RGBA(0.5f, 0.5f, 1.0f));

	new UITransition(new TransitionPieces(this, 1.0f), 0, true, false);
}

Level* DlgTitle::createLevel()
{
	Level* l = Super::createLevel();

	l->scene().setClearCol(RGBA(1,1,1));

	return l;
}

void DlgTitle::update(float delta)
{
	Super::update(delta);

	_parallax->update(delta);
}

void DlgTitle::preBeginDraw()
{
	_parallax->draw();

	D3D().reset();
	D3D().xformPixel();
	D3D().alpha(true);
	D3D().zbuffer(false);
	D3D().texFilter(false, false);

	_splitImagePancake->draw(Point2(0.5f * (D3D().screenSize().x - _splitImagePancake->size().x), 0));
	_picBros->align(ALIGN_CENTER, VALIGN_NONE);
	_picBros->setPos(Point2(_picBros->pos().x, _splitImagePancake->size().y / 640.0f));
}
