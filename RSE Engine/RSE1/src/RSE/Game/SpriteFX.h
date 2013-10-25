// ------------------------------------------------------------------------------------------------
//
// SpriteFX
// SpriteFX instances are actions associated with a particular frame of sprite animation. They are
// executed when that frame appears as a result of a sprite's animation.
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include <Standard/STLHelp.h>
#include <RSE/UI/Controls/EditableProperties.h>

class Config;
class Sprite;
class SpriteFXPlayer;
class SpriteFXFactory;
class TextureAnimator;

// SpriteFX base class
class RSE_API SpriteFX : public EditableProperties
{
	USE_RTTI(SpriteFX, EditableProperties);
	USE_STREAMING;
	USE_EDITSTREAM;

public:
	SpriteFX() :
	  frame(0),
	  id(-1)
	{}

	virtual ~SpriteFX()
	{}

	int frame;
	int id;

	virtual const char* help() = 0;

	// This function should perform the class's action.
	// Called when the animation frame containing the SpriteFX is encountered. 
	virtual void start(SpriteFXPlayer& mgr) {};

	// This function should terminate any action being performed by the SpriteFX.
	virtual void stop(SpriteFXPlayer& mgr) {};

	virtual void setPos(const Point2& pos) {}
	virtual const Point2& pos() const { return Point2::MAX; }
	virtual void setRot(const Quat& rot) {}
	virtual const Quat& rot() const { return Quat::NONE; }

	// If true, then 'stop' is called when the animation containing 
	// the SpriteFX reaches its conclusion, or if the Sprite's animation is changed.
	virtual bool shouldStopOnAnimationEnd() { return false; };
};

class RSE_API SpriteFXPos : public SpriteFX
{
	USE_RTTI(SpriteFXPos, SpriteFX);
	USE_EDITSTREAM;

public:
	SpriteFXPos() :
		m_pos(0,0),
		m_rot(1,0,0,0),
		m_attachToParent(true),
		m_stopOnAnimEnd(false)
	{}

	virtual void setPos(const Point2& pos) { m_pos = pos; }
	virtual const Point2& pos() const { return m_pos; }
	virtual void setRot(const Quat& rot) { m_rot = rot; }
	virtual const Quat& rot() const { return m_rot; }
	
	virtual bool shouldStopOnAnimationEnd() { return m_stopOnAnimEnd; };

protected:
	Point2 m_pos;
	Quat m_rot;
	bool m_attachToParent;
	bool m_stopOnAnimEnd;
};

// For SpriteFX classes that operate on several fx of a given id
class RSE_API SpriteFXIterating : public SpriteFX
{
	USE_RTTI(SpriteFXIterating, SpriteFX);
private:
	virtual void start(SpriteFXPlayer& mgr);
	virtual void stop(SpriteFXPlayer& mgr);

protected:
	virtual void start(SpriteFXPlayer& mgr, SpriteFX* fx) {};
	virtual void stop(SpriteFXPlayer& mgr, SpriteFX* fx) {};
};

// SpriteFX Player
class RSE_API SpriteFXPlayer
{
public:
	typedef std::vector<SpriteFX*> FXList;
	typedef stdext::hash_map<std::string, FXList> FXMap;

	SpriteFXPlayer(const PtrGC<Sprite>& o, SpriteFXFactory& factory) : 
	  m_lastFrame(-1),
	  m_sprite(o),
	  m_factory(factory),
	  m_currentSet(0),
	  m_currentSequence(0)
	{
		m_current = m_fx.end();
	}

	~SpriteFXPlayer();

	void update(class ITextureAnimator& animator);
    void retrigger(class ITextureAnimator& animator);
	FXList* current() { if (m_current == m_fx.end()) return 0; else return &m_current->second; }
	
	const PtrGC<Sprite>& sprite() { return m_sprite; }

	void stop(int id = -1);
	
	FXList get(int id);
	const SpriteFX* findFirst(const RTTI& what, const std::string& name = "") const;

protected:
	void play(class ITextureAnimator& animator);

	FXMap m_fx;
	FXMap::iterator m_current;
	
	int m_lastFrame;
	PtrGC<Sprite> m_sprite;
	SpriteFXFactory& m_factory;
	class TextureAnimationSet* m_currentSet;
	const class TextureAnimationSequence* m_currentSequence;
};

// SpriteFX Factory
class RSE_API SpriteFXFactory
{
public:
	SpriteFXFactory(const std::string& configName);
	~SpriteFXFactory();

	void fill(std::vector<SpriteFX*>& list, const std::string& set, const std::string& sequence);
	void update(const std::vector<SpriteFX*>& list, const std::string& set, const std::string& sequence);
	void writeToConfig();

private:
	typedef std::vector<SpriteFX*> FXList;
	typedef stdext::hash_map<std::string, FXList> FXMap;
	typedef stdext::hash_map<std::string, FXMap> SetMap;
	SetMap m_fx;
	std::string m_configName;
	Config* m_config;
};

