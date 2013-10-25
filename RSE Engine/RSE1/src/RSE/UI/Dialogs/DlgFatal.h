// ---------------------------------------------------------------------------------------------------------
// 
// DlgFatal
// 
// ---------------------------------------------------------------------------------------------------------
#pragma once


#include "UI/Dialog.h"


class RSE_API DlgFatal : public Dialog
{
	USE_RTTI(DlgFatal, Dialog);
public:
	DlgFatal(const std::vector<std::string>& callstack, const std::string& message);

protected:
	virtual void construct();
	virtual void onDraw();
	virtual void onPostDraw();

	void onExit();

	PtrD3D<IDirect3DTexture9> m_pTex;
	std::string m_message;
	class UITextBox* m_pBox;
	class UIButton* m_pButton;
};