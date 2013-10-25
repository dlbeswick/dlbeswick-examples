// ------------------------------------------------------------------------------------------------
//
// UIStatusBar
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/UI/UIElement.h"

class UIStatusBar : public UIElement
{
	USE_RTTI(UIStatusBar, UIElement);
public:
	UIStatusBar();

	void setMin(float f);
	void setMax(float f);
	void setValue(const float* f) { m_value = f; }

	// style
	UISTYLE
		TStyle()
		{
			borderMaterial = 0;
			fillMaterial = 0;
		}

		float		borderWidth;
		
		const Material*	borderMaterial;
		const Material*	fillMaterial;

	protected:
		std::string borderMaterialName;
		std::string fillMaterialName;

		virtual void onPostRead();
	UISTYLEEND

protected:
	virtual void onDraw();

	float m_max;
	float m_min;
	const float* m_value;
};