// ---------------------------------------------------------------------------------------------------------
//
// Font
//
// ---------------------------------------------------------------------------------------------------------
#ifndef RSE_FONT_H
#define RSE_FONT_H

#include "RSE/RSE.h"
#include "Standard/Singleton.h"

class FontElement;

class RSE_API SFont : public Singleton<SFont>
{
public:
	SFont();
	~SFont();

	FontElement* operator ->() const { return m_pCurFont; }

	void makeTextureFonts();

	bool createSystemFont(const std::string& face, const std::string& name, uint size, uint texSize = 512, 
					uint weight = FW_NORMAL, bool bItalic = false);
	bool copyFont(const std::string& srcName, const std::string& destName, float destScale = 1.0f);

	void set(const std::string& name);
	void set(FontElement* font) { m_pCurFont = font; }

	FontElement* activeFont() const { return m_pCurFont; }
	FontElement* get(const std::string& name);
	bool has(const std::string& name) const;

private:
	typedef stdext::hash_map<std::string, FontElement*> FontMap;
	FontMap				m_fonts;
	FontElement*		m_pCurFont;
};

inline SFont& Font() { return SFont::instance(); }

#endif