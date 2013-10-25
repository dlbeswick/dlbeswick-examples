// ------------------------------------------------------------------------------------------------
//
// TextureAnimator
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/UI/Controls/UIShuttle.h" // fix: own file

class AnimationFrame;
class AnimationSet;
class AnimationSequence;
class TextureAnimationFrame;
class TextureAnimationSet;
class TextureAnimationSequence;

// TBD: Tidy, move into own header files
class RSE_API IAnimator
{
public:
	virtual void update(float delta) = 0;
	virtual float frameIdx() const = 0;
	virtual int frames() const = 0;

	virtual void useSet(AnimationSet& animSet) = 0;
	virtual bool has(const std::string& sequenceName) = 0;
	virtual void play(const std::string& sequenceName, bool loop = false, bool force = false) = 0;
	virtual void stop() = 0;
	virtual void start() = 0;
	virtual void setScale(float scale) = 0;
	virtual void setFrame(int idx) = 0;

	virtual float scale() const = 0;
	virtual float duration() const = 0;
	virtual bool finished() const = 0;
	virtual bool playing() const = 0;
	virtual bool playingNull() = 0;
};

class RSE_API ITextureAnimator : virtual public IAnimator
{
public:
	virtual const TextureAnimationFrame& frame() const = 0;
	virtual const TextureAnimationFrame& frame(const std::string& sequence, uint idx) = 0;
	virtual const TextureAnimationFrame& frame(uint idx) = 0;
	virtual const TextureAnimationFrame& frameAt(float time, float animScale = 1.0f) const = 0;
	virtual class TextureAnimationSet& set() const = 0;
	virtual const class TextureAnimationSequence& sequence() const = 0;
	virtual const class TextureAnimationSequence& sequence(const std::string& sequence) const = 0;

	virtual const std::string& curSequenceName() const = 0;
};

class RSE_API Animator : virtual public IAnimator, virtual public IShuttleNotify, public PtrGCHost
{
public:
	Animator(AnimationSet& animSet);

	void update(float delta);
	const AnimationFrame& frame() const;
	const AnimationFrame& frame(const std::string& sequence, uint idx);
	const AnimationFrame& frame(uint idx);
	const AnimationFrame& frameAt(float time, float animScale = 1.0f) const;
	float frameIdx() const { return m_frame; }
	int frames() const;

	void useSet(AnimationSet& animSet);
	bool has(const std::string& sequenceName);
	void play(const std::string& sequenceName, bool bLoop = false, bool bForce = false);
	void stop() { m_bPlay = false; }
	void start() { m_bPlay = true; }
	void setScale(float scale) { m_scale = scale; }
	void setFrame(int idx);

	float scale() const { return m_scale; }
	float duration() const;
	bool finished() const;
	bool playing() const { return m_bPlay; }
	bool playingNull();
	
	class AnimationSet& set() const { return *m_set; }
	const class AnimationSequence& sequence() const { return *m_sequence; }
	const class AnimationSequence& sequence(const std::string& sequence) const;

	const std::string& curSequenceName() const { return m_curSequenceName; }

	// IShuttleNotify
	virtual bool shuttleIsPlaying() { return playing(); }

	TTPMultiDelegate<class Object, const AnimationFrame&> onFrame;

protected:
	AnimationSequence& nullSequence() const;

	class AnimationSet* m_set;
	const AnimationSequence* m_sequence;
	std::string m_curSequenceName;
	float m_frame;
	float m_scale;
	bool m_bLoop;
	bool m_bFinished;
	bool m_bPlay;

private:
	virtual const class EmbeddedPtrHost* host() const { return this; }
	virtual class EmbeddedPtrHost* host() { return this; }
};

class RSE_API TextureAnimator : public ITextureAnimator, public Animator
{
public:
	TextureAnimator(TextureAnimationSet& animSet) :
	  Animator((AnimationSet&)animSet)
	{};

	virtual const TextureAnimationFrame& frame() const { return (const TextureAnimationFrame&)Animator::frame(); }

	const TextureAnimationFrame& frame(const std::string& sequence, uint idx) { return (const TextureAnimationFrame&)Animator::frame(sequence, idx); }
	const TextureAnimationFrame& frame(uint idx) { return (const TextureAnimationFrame&)Animator::frame(idx); }
	const TextureAnimationFrame& frameAt(float time, float animScale = 1.0f) const { return (const TextureAnimationFrame&)Animator::frameAt(time, animScale); }

	TextureAnimationSet& set() const { return (TextureAnimationSet&)Animator::set(); }
	const TextureAnimationSequence& sequence() const { return (TextureAnimationSequence&)Animator::sequence(); }
	const TextureAnimationSequence& sequence(const std::string& sequence) const { return (TextureAnimationSequence&)Animator::sequence(sequence); }

	const std::string& curSequenceName() const { return Animator::curSequenceName(); }
};

class RSE_API SpriteFXAnimator : public TextureAnimator
{
public:
	SpriteFXAnimator(const PtrGC<class Sprite>& sprite, TextureAnimationSet& animSet);
	~SpriteFXAnimator();

	virtual void update(float delta);

	class SpriteFXPlayer& spriteFX() const { return *m_spriteFX; }

protected:
	class SpriteFXPlayer* m_spriteFX;
};