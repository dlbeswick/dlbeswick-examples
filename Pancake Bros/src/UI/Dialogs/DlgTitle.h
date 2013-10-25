// ------------------------------------------------------------------------------------------------
//
// DlgTitle
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/UI/Dialogs/DlgStandardLevel.h"
#include "RSE/Render/Materials/Material.h"

class UIPic;
class SplitImage;

class DlgTitle : public DlgStandardLevel
{
	USE_RTTI(DlgTitle, DlgStandardLevel);
public:
	DlgTitle();
	~DlgTitle();

	virtual void construct();
	virtual void update(float delta);

protected:
	virtual Level* createLevel();

	void onStart();
	void onOptions();
	void onQuit();

	virtual void preBeginDraw();

	void fart();
	
	SmartPtr<class Parallax> _parallax;
	SmartPtr<class SplitImage> _splitImagePancake;
	SmartPtr<class Material> _logoBrosMaterial;
	UIPic* _picBros;
};