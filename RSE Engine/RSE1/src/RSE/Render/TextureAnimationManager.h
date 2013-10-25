// ------------------------------------------------------------------------------------------------
//
// TextureAnimationManager
// A class for storing a set of 2D texture animations
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "TextureAnimationTypes.h"
#include "Standard/Path.h"

class TGA;
class Config;


class RSE_API TextureAnimationManager
{
public:
	typedef stdext::hash_map<std::string, TextureAnimationSet*> Sets;

	TextureAnimationManager(const std::string& fname, int texSize = 512);
	~TextureAnimationManager();

	TextureAnimationSet& set(const std::string& name)
	{
		Sets::iterator i = m_sets.find(name);
		if (i != m_sets.end())
			return *i->second;
		else 
			return *m_sets[""];
	}

	const Sets& sets() const { return m_sets; }

	struct PicturePos
	{
		//std::vector<Point2i> pos;
		//std::vector<Point2i> size;
		Point2i pos;
		Point2i size;
		int compilationNum;
	};

	int texSize() { return m_texSize; }

	Config& config() { return *m_config; }

private:
	void cacheString(std::string& str, Path path = Path());
	bool rebuildNeeded(const std::string& testCacheString);
	void buildCompilations();
	Path saveCompilation(TGA& tga, int& fileNumber, int currentUsedPixels);
	void loadCompilations();

	Sets m_sets;

	typedef std::map<std::string, PicturePos> FrameMap;
	FrameMap m_frameMap;

	typedef std::vector<PtrD3D<IDirect3DTexture9> > Textures;
	Textures m_textures;

	const int m_texSize;

	Config* m_config;
};