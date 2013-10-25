// ---------------------------------------------------------------------------------------------------------
//
// SFont
//
// ---------------------------------------------------------------------------------------------------------
#include "pch.h"
#include "RSE/AppBase.h"
#include "SFont.h"
#include "SDeviceD3D.h"
#include "BillBoarder.h"
#include "FontElement.h"
#include "FontElementSystem.h"
#include "FontElementTexture.h"
#include "Standard/Config.h"
#include "Standard/Exception.h"
#include "Standard/STLHelp.h"
#include "Standard/textstringstream.h"

// construct
SFont::SFont() : m_pCurFont(0)
{
}

// destruct
SFont::~SFont()
{
	freeHash(m_fonts);
}

// createFont
bool SFont::createSystemFont(const std::string &face, const std::string &name, uint size, uint texSize, uint weight, 
					   bool bItalic)
{
	if (m_fonts.find(name) != m_fonts.end())
		return false;

	FontElement* font = new FontElementSystem(face, name, size, texSize, weight, bItalic);
	font->construct();

	m_fonts[name] = font;

	return true;
}

bool SFont::copyFont(const std::string& srcName, const std::string& destName, float destScale)
{
	FontMap::iterator i = m_fonts.find(srcName);
	if (i == m_fonts.end())
		return false;

	m_fonts[destName] = i->second->clone(destName);
	m_fonts[destName]->setScale(destScale);

	return true;
}

// set
void SFont::set(const std::string &name)
{
	m_pCurFont = get(name);
}

FontElement* SFont::get(const std::string& name)
{
	FontMap::iterator i = m_fonts.find(name);
	if (i == m_fonts.end())
	{
		// tbd: make font-specific exception
		throwf("No font available called '" + name + "'");
	}

	return i->second;
}

bool SFont::has(const std::string& name) const
{
	return m_fonts.find(name) != m_fonts.end();
}

void SFont::makeTextureFonts()
{
	Config& ui = AppBase().ui();

	Config::Categories::const_iterator categoryIt = 
		std::find(ui.categories().begin(), ui.categories().end(), "Fonts");

	if (categoryIt != ui.categories().end())
	{
		for (Config::Vars::const_iterator varsIt = categoryIt->vars.begin();
				varsIt != categoryIt->vars.end();
				++varsIt)
		{
			const ConfigService::ValueData& var = **varsIt;

			FontElement* font = new FontElementTexture(var.name);

			itextstringstream configDataStream(var.get<std::string>());

			font->read(configDataStream);
			font->construct();

			m_fonts[font->name()] = font;
		}
	}
}
