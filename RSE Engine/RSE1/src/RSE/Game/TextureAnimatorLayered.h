// ------------------------------------------------------------------------------------------------
//
// TextureAnimatorLayered
// Simple animation "blending" (more like priority system) for sprite character animation.
// Individual animation layers can be accessed or the main interface used for layer 0.
// All layers use the same set but can use different sequences.
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/Render/TextureAnimator.h"
#include "RSE/Render/TextureAnimationTypes.h"


class RSE_API TextureAnimatorLayered : public ITextureAnimator
{
public:
	TextureAnimatorLayered(const PtrGC<class Sprite>& sprite, TextureAnimationSet& set, int layers);
	~TextureAnimatorLayered();

	SpriteFXAnimator& operator [] (int idx) { return *m_layers[idx]; }

	void update(float delta);

	bool has(const std::string& sequenceName) { return m_layers[0]->has(sequenceName); }
	void play(const std::string& sequenceName, bool loop = false, bool force = false) { m_layers[0]->play(sequenceName, loop, force); }
	void clear();
	void stop();
	void start();
	void setScale(float scale) { m_layers[0]->setScale(scale); }
	
	void setFrame(int idx) { m_layers[0]->setFrame(idx); }
	float scale() const { return m_layers[0]->scale(); }
	float duration() const { return m_layers[0]->duration(); }
	bool finished() const { return m_layers[0]->finished(); }
	bool playing() const { return m_layers[0]->playing(); }

	float frameIdx() const { return m_layers[0]->frameIdx(); }
	int frames() const { return m_layers[0]->frames(); }
	const TextureAnimationFrame& frame() const;
	const TextureAnimationFrame& frame(const std::string& sequence, uint idx) { return m_layers[0]->frame(sequence, idx); }
	const TextureAnimationFrame& frame(uint idx) { return m_layers[0]->frame(idx); }
	const TextureAnimationFrame& frameAt(float time, float animScale = 1.0f) const { return m_layers[0]->frameAt(time, animScale); }
	class TextureAnimationSet& set() const { return m_layers[0]->set(); }
	const class TextureAnimationSequence& sequence() const  { return m_layers[0]->sequence(); }
	const TextureAnimationSequence& sequence(const std::string& sequence) const { return m_layers[0]->sequence(sequence); }
	const std::string& curSequenceName() const { return m_layers[0]->curSequenceName(); }
	bool playingNull() { return m_layers[0]->playingNull(); }

	void useSet(class AnimationSet& set);

protected:
	bool inRange(int layer) { return layer >= 0 && layer < (int)m_layers.size(); }

	typedef std::vector<class SpriteFXAnimator*> Layers;
	Layers m_layers;
};