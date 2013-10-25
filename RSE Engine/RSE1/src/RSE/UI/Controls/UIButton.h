// ---------------------------------------------------------------------------------------------------------
// 
// UIButtonBase
// 
// ---------------------------------------------------------------------------------------------------------
#pragma once


#include "PlaceableControl.h"

class ButtonData;

// Data types
class RSE_API ButtonData : public UIContainer
{
	USE_RTTI(ButtonData, UIContainer);

public:
	virtual void calcSize(const Point2& size) {};
};

// UIButtonBase
class RSE_API UIButtonBase : public PlaceableControl
{
	USE_RTTI(UIButtonBase, PlaceableControl);

public:
	UIButtonBase();

	Delegate onClick;

	virtual void setSize(const Point2& s);

	void setData(ButtonData* pData, bool bFit = true);
	void fitToData();

	virtual void write(obinstream& s) const;
	virtual void read(ibinstream& s);

	virtual void mouseOff();
	virtual void keyChar(int key);
	virtual void keyDown(int key);
	virtual void keyUp(int key);

	virtual bool usesFocus() const { return true; }

	// style
	UISTYLE
		std::string materialHighlight;
		std::string materialShadow;
		std::string materialFace;
		
		const Material* materialHighlightPtr;
		const Material* materialShadowPtr;
		const Material* materialFacePtr;

	protected:
		virtual void onPostRead();
	UISTYLEEND

protected:
	virtual void onDraw();

	virtual void _onFontChange();

	// input
	virtual bool onMousePressed(const Point2& pos, int button) { return true; }
	virtual bool onMouseUp(const Point2 &pos, int button);
	virtual bool onMouseDown(const Point2 &pos, int button);

	virtual void recalcDataPos(const Point2& reqSize);
	void setPressed(bool bPressed);

	PtrGC<ButtonData> m_pData;
	bool _fitData;

private:
	bool m_bPressed;	// use setPressed
};


// Data types
class RSE_API ButtonText : public ButtonData
{
	USE_RTTI(ButtonText, ButtonData);

public:
	ButtonText(const std::string& text = "");

	virtual void write(obinstream& s) const;
	virtual void read(ibinstream& s);

	virtual void calcSize(const Point2& size);

	std::string m_text;

protected:
	virtual void onDraw();
};


class RSE_API ButtonPic : public ButtonData
{
	USE_RTTI(ButtonPic, ButtonData);

public:
	ButtonPic(const std::string& fname, const char* resType = 0);

	virtual void write(obinstream& s) const;
	virtual void read(ibinstream& s);
	
	virtual void calcSize(const Point2& size)
	{
		setSize(size);
	}

protected:
	void load();

	PtrD3D<IDirect3DTexture9> m_tex;
	std::string m_fname;
	const char* m_resType;

	virtual void onDraw();
};

// UIButton
class RSE_API UIButton : public UIButtonBase
{
	USE_RTTI(UIButton, UIButtonBase);

public:
	UIButton(const std::string& text = std::string());

	void setText(const std::string& text) { setData(new ButtonText(text)); }
};

// UIButtonPic
class RSE_API UIButtonPic : public UIButtonBase
{
	USE_RTTI(UIButtonPic, UIButtonBase);

public:
	UIButtonPic();

	void setPic(const char* fname, const char* resType) { setData(new ButtonPic(fname, resType)); }
};
