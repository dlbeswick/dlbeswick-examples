// ------------------------------------------------------------------------------------------------
//
// TextureAnimationTypes
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/RSE.h"
#include "Standard/PtrD3D.h"
#include "Standard/Streamable.h"
#include "Standard/textstream.h"

// TBD: Tidy ////////////////////////////////

class RSE_API AnimationFrame
{
	virtual bool isNullFrame() const = 0;
};

class RSE_API AnimationSequence : public Streamable
{
public:
	AnimationSequence() :
	  duration(0)
	{
	}

	virtual ~AnimationSequence()
	{
		for (uint i = 0; i < frames.size(); ++i)
			delete frames[i];
	}

	virtual AnimationFrame& nullFrame() const = 0;

	AnimationFrame& frame(int idx) { return (AnimationFrame&)*frames[idx]; }

	virtual void makeNull()
	{
		frames.push_back(&nullFrame());
		frameNames.push_back("");
		duration = 0;
	}

	virtual void read(itextstream& s)
	{
		if (s.has("len:"))
		{
			s >> "len:" >> duration >> ",";
		}
		else
		{
			duration = 1;
		}

		s << itextstream::whitespace("\n");
		s << itextstream::str_delimiter("\n,");
		while (s)
		{
			frameNames.push_back("");
			s >> frameNames.back();
			if (!s.has(","))
				break;

			s >> ",";
		}
	}

	std::vector<AnimationFrame*> frames; 
	std::vector<std::string> frameNames; 
	float duration;
	std::string name;
};

class RSE_API AnimationSet
{
public:
	virtual ~AnimationSet()
	{
		for (Sequences::iterator i = sequences.begin(); i != sequences.end(); ++i)
			delete i->second;
	}

	virtual AnimationSequence* nullSequence() = 0;

	AnimationSequence& sequence(const std::string& name) { return (AnimationSequence&)*sequences[name]; }

	typedef stdext::hash_map<std::string, AnimationSequence*> Sequences;
	Sequences sequences;
	std::string name;
};

//////////////////////////////////

class RSE_API TextureAnimationFrame : public AnimationFrame
{
public:
	enum
	{
		// TBD: get this from compiled image data or texture animation manager.
		COMPILED_IMAGE_SIZE = 512
	};

	Point2 uvMin;
	Point2 uvMax;
	PtrD3D<IDirect3DTexture9> texture;

	virtual bool isNullFrame() const;

	/// Return the size of the frame in uv space.
	Point2 size() const { return uvMax - uvMin; }

	/// Return the size in pixels of the frame.
	Point2 pixels() const { return (uvMax - uvMin) * (float)COMPILED_IMAGE_SIZE; }
};

class RSE_API TextureAnimationSequence : public AnimationSequence
{
public:
	TextureAnimationFrame& frameAt(int idx) { return (TextureAnimationFrame&)*frames[idx]; }

	virtual AnimationFrame& nullFrame() const;
};

class RSE_API TextureAnimationSet : public AnimationSet
{
public:
	TextureAnimationSequence& sequence(const std::string& name) { return (TextureAnimationSequence&)*sequences[name]; }

	virtual AnimationSequence* nullSequence();
};