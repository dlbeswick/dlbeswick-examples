// ------------------------------------------------------------------------------------------------
//
// TextureAnimationManager
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "TextureAnimationManager.h"
#include "PathResource.h"
#include "Standard/Config.h"
#include "Standard/FileHelp.h"
#include "Standard/FileMgr.h"
#include "Standard/Filesystem.h"
#include "Standard/Log.h"
#include "Standard/STLHelp.h"
#include "Standard/TGA.h"
#include <Standard/Exception/Filesystem.h>
#include "Render/SDeviceD3D.h"
#include "AppBase.h"

obinstream& operator << (obinstream& s, const TextureAnimationManager::PicturePos& obj)
{
	s.s().write((char*)&obj, sizeof(TextureAnimationManager::PicturePos)); 
	return s;
}

ibinstream& operator >> (ibinstream& s, TextureAnimationManager::PicturePos& obj)
{
	s.s().read((char*)&obj, sizeof(TextureAnimationManager::PicturePos)); 
	return s;
}


TextureAnimationManager::TextureAnimationManager(const std::string& fname, int texSize) :
	m_texSize(texSize),
	m_config(new Config(PathResource(fname)))
{
	Config& config = *m_config;

	// make null set
	m_sets[""] = new TextureAnimationSet;

	// load frames of animation from config
	const Config::Categories& cats = config.categories();
	for (Config::Categories::const_iterator i = cats.begin(); i != cats.end(); ++i)
	{
		TextureAnimationSet*& newSet = m_sets[i->category->name];
		if (!newSet)
			newSet = new TextureAnimationSet;

		newSet->name = i->category->name;

		for (Config::Vars::const_iterator j = i->vars.begin(); j != i->vars.end(); ++j)
		{
			const std::string& sequenceName = (*j)->name;
			std::string sequenceText;
			config.get(i->category->name, sequenceName, sequenceText);

			// resolve any relative paths in the sequenceText
			// tbd: too much conversion back and forth here
			bool modified = false;
			std::vector<std::string> frameVec;
			split(sequenceText, frameVec, ",");
			
			bool skip = false;

			// hack, skip "0"
			if (frameVec.size() == 1 && frameVec[0] == "0")
				skip = true;

			if (!skip)
			{
				for (uint idx = 0; idx < frameVec.size(); ++idx)
				{
					std::string& s = frameVec[idx];
					// hack -- skip 'len'
					if (s.find("len:") != std::string::npos)
						continue;

					if (s.find('\\') == std::string::npos)
					{
						s = i->category->name + "\\" + s;
						modified = true;
					}
				}

				sequenceText = join(frameVec, ",");
				if (modified)
					config.set(i->category->name, sequenceName, sequenceText);
			}

			TextureAnimationSequence* newSeq = new TextureAnimationSequence;
			newSet->sequences[sequenceName] = newSeq;

			if (!sequenceText.empty())
			{
				newSeq->name = sequenceName;

				if (sequenceText == "0")
				{
					newSeq->makeNull();
				}
				else
				{
					itextstream stream(sequenceText);
					stream >> *newSeq;
				}
			}
		}
	}

	// load or build framemap
	std::string testCacheString;

	try
	{
		ibinstream s((AppBase().pathLocalSettings() + Path("compiled images/frame map.dat")).open(std::ios::binary | std::ios::in));

		s >> testCacheString;

		if (rebuildNeeded(testCacheString))
			buildCompilations();
	}
	catch (ExceptionFilesystem&)
	{
		buildCompilations();
	}

	try
	{
		ibinstream s((AppBase().pathLocalSettings() + Path("compiled images/frame map.dat")).open(std::ios::binary | std::ios::in));

		s >> testCacheString;
		s >> m_frameMap;
	}
	catch (ExceptionFilesystem&)
	{
		throwf("Error while compiling animation frames - couldn't open 'frame map.dat'");
	}

	try
	{
		loadCompilations();
	}
	catch (const Exception&)
	{
		buildCompilations();

		try
		{
			loadCompilations();
		}
		catch (const Exception& e)
		{
			throwf("'frame map.dat' was corrupt and could not be rebuilt: " + e);
		}
	}
}

TextureAnimationManager::~TextureAnimationManager()
{
	delete m_config;
	freeHash(m_sets);
}

// make textures from the compilation images
void TextureAnimationManager::loadCompilations()
{
	uint i = 0;
	do
	{
		Path fName(AppBase().pathLocalSettings() + Path(std::string("compiled images\\compiled") + i + ".tga"));

		++i;

		if (!fName.exists())
			break;

		m_textures.push_back(D3D().loadTexture(fName, false));
	}
	while (true);

	// resolve textures to frame names
	for (Sets::iterator i = m_sets.begin(); i != m_sets.end(); ++i)
	{
		TextureAnimationSet& animSet = *i->second;
		for (TextureAnimationSet::Sequences::iterator j = animSet.sequences.begin(); j != animSet.sequences.end(); ++j)
		{
			TextureAnimationSequence& seq = (TextureAnimationSequence&)*j->second;
			for (uint frameNum = 0; frameNum < seq.frameNames.size(); ++frameNum)
			{
				PicturePos& pos = m_frameMap[seq.frameNames[frameNum]];
				TextureAnimationFrame& frame = (TextureAnimationFrame&)**seq.frames.insert(seq.frames.end(), new TextureAnimationFrame);
				int textureNum = pos.compilationNum;
				if (textureNum != -1)
				{
					if (textureNum < 0 || textureNum > (int)m_textures.size())
					{
						throwf(std::string("Frame called '") + seq.frameNames[frameNum] + "' has an invalid texture compilation index '" + textureNum + "'.");
					}

					frame.texture = m_textures[textureNum];
					frame.uvMin = Point2((float)pos.pos.x / m_texSize, (float)pos.pos.y / m_texSize);
					frame.uvMax = Point2((float)(pos.pos.x + pos.size.x) / m_texSize, (float)(pos.pos.y + pos.size.y) / m_texSize);
				}
			}

			// framenames no longer needed
			seq.frameNames.empty();
		}
	}
}

Path TextureAnimationManager::saveCompilation(TGA& tga, int& fileNumber, int currentUsedPixels)
{
	Path path;

	// save current compilation if available
	if (tga.size().x != 0)
	{
		// apply colour key
		tga.colourKey(RGBA(1, 0, 1), RGBA(0, 0, 0, 0));

		/*Point2i pos;
		// blend pixels around zero alpha areasp
		for (pos.y = 0; pos.y < tga.size().y; ++pos.y)
		{
			for (pos.x = 0; pos.x < tga.size().x; ++pos.x)
			{
				uchar* c = tga.data(pos);
				if (c[3] == 0)
				{
					int newVal[4] = {0, 0, 0, 0};

					for (uint i = 0; i < 3; ++i)
					{
						int div = 0;

						if (pos.x > 0)
						{
							if (tga.data(pos.x - 1, pos.y)[3] != 0)
								newVal[i] += tga.data(pos.x - 1, pos.y)[i];
							++div;
						}
						if (pos.x < tga.size().x)
						{
							if (tga.data(pos.x + 1, pos.y)[3] != 0)
								newVal[i] += tga.data(pos.x + 1, pos.y)[i];
							++div;
						}
						if (pos.y > 0)
						{
							if (tga.data(pos.x - 1, pos.y + 1)[3] != 0)
								newVal[i] += tga.data(pos.x, pos.y - 1)[i];
							++div;
						}
						if (pos.y < tga.size().y)
						{
							if (tga.data(pos.x, pos.y + 1)[3] != 0)
								newVal[i] += tga.data(pos.x, pos.y + 1)[i];
							++div;
						}
						if (pos.x > 0 && pos.y > 0)
						{
							if (tga.data(pos.x - 1, pos.y - 1)[3] != 0)
								newVal[i] += tga.data(pos.x - 1, pos.y - 1)[i];
							++div;
						}
						if (pos.x < tga.size().x && pos.y > 0)
						{
							if (tga.data(pos.x + 1, pos.y - 1)[3] != 0)
								newVal[i] += tga.data(pos.x + 1, pos.y - 1)[i];
							++div;
						}
						if (pos.x < tga.size().x && pos.y < tga.size().y)
						{
							if (tga.data(pos.x + 1, pos.y + 1)[3] != 0)
								newVal[i] += tga.data(pos.x + 1, pos.y + 1)[i];
							++div;
						}
						if (pos.x > 0 && pos.y < tga.size().y)
						{
							if (tga.data(pos.x - 1, pos.y + 1)[3] != 0)
								newVal[i] += tga.data(pos.x - 1, pos.y + 1)[i];
							++div;
						}

						newVal[i] /= div;
						c[i] = newVal[i];
					}
				}
			}
		}*/

		// save file
		path = AppBase().pathLocalSettings() + Path("compiled images" + std::string("\\compiled") + fileNumber++ + std::string(".tga"));
		path.create();

		obinstream out(path.open(std::ios::out | std::ios::binary));
		out << tga;

		if (!path.empty())
		{
			int currentPixels = tga.size().x * tga.size().y;
			float waste = 100.0f - ((float)currentUsedPixels / currentPixels) * 100.0f;
			dlog << "Writing " << *path << " (" << waste << "% wasted)" << dlog.endl;
		}
	}

	return path;
}

bool TextureAnimationManager::rebuildNeeded(const std::string& testCacheString)
{
	std::string cache;
	cacheString(cache);

	return cache != testCacheString;
}

void TextureAnimationManager::cacheString(std::string& str, Path path)
{
	WIN32_FIND_DATA data;

	str += __TIMESTAMP__;

	if (path.empty())
		path = AppBase().pathResources() + Path("media/textures");

	HANDLE h = FindFirstFile((*(path + Path("/*.*"))).c_str(), &data);
	if (h == INVALID_HANDLE_VALUE)
		return;

	do
	{
		// don't recurse back or to current directory
		if (std::string(data.cFileName) != "." && std::string(data.cFileName) != "..")
		{
			str += std::string(data.cFileName) + (uint)data.ftLastWriteTime.dwLowDateTime + std::string("+") + (uint)data.ftLastWriteTime.dwHighDateTime;
			if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				cacheString(str, path + Path(data.cFileName));
			}
		}

		if (!FindNextFile(h, &data))
			break;
	}
	while (1);

	FindClose(h);
}

void TextureAnimationManager::buildCompilations()
{
	dlog << "Assembling compiled images for texture animation..." << dlog.endl;

	// create "compiled images" local path
	(AppBase().pathLocalSettings() + Path("compiled images")).create();

	m_frameMap.empty();

	// form a set of unique filenames
	// due to alphabetical sorting, files will be grouped by directory
	for (Sets::iterator i = m_sets.begin(); i != m_sets.end(); ++i)
	{
		for (TextureAnimationSet::Sequences::iterator j = i->second->sequences.begin(); j != i->second->sequences.end(); ++j)
		{
			for (uint idx = 0; idx < j->second->frameNames.size(); ++idx)
			{
				m_frameMap.insert(FrameMap::value_type(j->second->frameNames[idx], PicturePos()));
			}
		}
	}

	// organise picture data into large textures
	std::string currentCategory;
	TGA currentTGA;
	Point2i currentPos(1, 1);
	int maxSizeY = 0;
	int imageNumber = 0;
	int totalPixels = 0;
	int totalUsedPixels = 0;
	int currentUsedPixels = 0;

	dlog.tabIn();

	FrameMap::iterator currentPic = m_frameMap.begin();
	while (currentPic != m_frameMap.end())
	{
		const std::string& frameName = currentPic->first;
		if (frameName.empty())
		{
			currentPic->second.compilationNum = -1;
			currentPic++;
			continue;
		}

		PicturePos& picturePos = currentPic->second;

		PathResource fullPath("media/textures/" + frameName + ".tga");
		
		TGA tga;

		// read tga
		try
		{
			ibinstream stream(fullPath.open(std::ios::binary | std::ios::in));
			stream >> tga;
		}
		catch(ExceptionFilesystem&)
		{
			throwf("Animation frame " + *fullPath + " specified in animations.ini was not found");
		}
		
		//if (tga.size().y > m_texSize)
		if (tga.size().x > m_texSize && tga.size().y > m_texSize)
		{
			//throwf("Animation frame " + *fullPath + " is too large with size " + tga.size().x + std::string("x") + tga.size().y + std::string(", the height must be smaller than ") + m_texSize);
			throwf("Animation frame " + *fullPath + " is too large with size " + tga.size().x + std::string("x") + tga.size().y + std::string(", dimensions must be smaller than ") + m_texSize);
		}

		Point2i srcPos(0, 0);
		Point2i srcSize(tga.size());

		// copy picture to compilation image.
		// loop copying in segments if the source image is wider than the compilation image (not yet implemented)
		while (srcPos.x < srcSize.x)
		{
			// move write pos to next line if necessary
			if (currentPos.x + srcSize.x >= m_texSize)
			{
				currentPos.x = 1;
				currentPos.y += maxSizeY + 1;
				maxSizeY = 0;
			}

			// get the category of the picture
			uint lastSlash = (*fullPath).rfind('\\');
			if (lastSlash == std::string::npos)
				throwf("Bad data for animation frame " + *fullPath + ": no slash in path");
			
			std::string picCategory = (*fullPath).substr(0, lastSlash);
			if (picCategory != currentCategory || currentPos.y + tga.size().y > m_texSize)
			{
				int currentPixels = currentTGA.size().x * currentTGA.size().y;
				totalPixels += currentPixels;
				totalUsedPixels += currentUsedPixels;
				
				// save current compilation if available
				saveCompilation(currentTGA, imageNumber, currentUsedPixels);

				// create a new picture after switching categories
				currentTGA.create(m_texSize, m_texSize, Image::ARGB32);
				memset(currentTGA.data(), 0, currentTGA.bytesPerPixel() * m_texSize * m_texSize);
				currentCategory = picCategory;
				currentPos = Point2i(1, 1);
				currentUsedPixels = 0;
			}
			
			Point2i copySize(std::min(m_texSize, srcSize.x), std::min(m_texSize, srcSize.y));

			// transfer picture to compilation tga
			tga.copy(currentTGA, currentPos, srcPos, srcSize);
			//picturePos.pos.push_back(currentPos);
			//picturePos.size.push_back(copySize);
			picturePos.pos = currentPos;
			picturePos.size = copySize;
			
			currentUsedPixels += copySize.x * copySize.y;

			// move copy pos
			maxSizeY = std::max(maxSizeY, std::min(m_texSize, tga.size().y));
			currentPos.x += srcSize.x;
			srcSize.x -= copySize.x;
			srcSize.y -= copySize.y;
		}
		
		// leave a one-pixel gap between source images, to prevent bleedover when using linear interpolation magfilter
		currentPos.x += 1;

		picturePos.compilationNum = imageNumber;

		currentPic++;
	}

	saveCompilation(currentTGA, imageNumber, currentUsedPixels);

	// save record of frame mappings and cache string
	obinstream s((AppBase().pathLocalSettings() + Path("compiled images/frame map.dat")).open(std::ios::out | std::ios::binary));

	std::string cache;
	cacheString(cache);

	s << cache;
	s << m_frameMap;

	dlog.tabOut();

	dlog << "Compilation complete, " << 100.0f - (((float)totalUsedPixels / totalPixels) * 100.0f) << "% of space was wasted." << dlog.endl;
}
