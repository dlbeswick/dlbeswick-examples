// ------------------------------------------------------------------------------------------------
//
// TextureAnimator
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "TextureAnimator.h"

#include "AppBase.h"
#include "Game/Sprite.h"
#include "Game/SpriteFX.h"
#include "TextureAnimationManager.h"
#include "TextureAnimationTypes.h"

Animator::Animator(AnimationSet& animSet) :
	m_sequence(0)
{
	useSet(animSet);
	m_frame = 0;
	m_scale = 1;
	m_bFinished = true;
	m_bPlay = true;
}

AnimationSequence& Animator::nullSequence() const
{
	AnimationSequence* s = m_set->sequences[""];
	if (!s)
	{
		s = m_set->nullSequence();
		s->frames.push_back(&s->nullFrame());
		m_set->sequences[""] = s;
	}

	return *s;
}

bool Animator::playingNull()
{
	return m_sequence == &nullSequence();
}

void Animator::useSet(AnimationSet& animSet)
{
	m_set = &animSet;
	play("");
}

void Animator::play(const std::string& sequenceName, bool bLoop, bool bForce)
{
	m_bFinished = false;

	const AnimationSequence* oldSeq = m_sequence;
	m_sequence = &sequence(sequenceName);

	m_curSequenceName = sequenceName;

	if (m_sequence->duration == 0)
	{
		m_bFinished = true;
		m_bPlay = false;
		m_frame = 0;
	}
	else if (m_sequence != oldSeq || bForce)
	{
		m_frame = 0;
		m_scale = 1;
		m_bPlay = true;
		onFrame(frame());
	}

	m_bLoop = bLoop;
}

void Animator::update(float delta)
{
	if (m_sequence->duration == 0 || !m_bPlay)
		return;

	uint numFrames = m_sequence->frames.size();

	if (numFrames == 0)
		return;

	int oldFrame = (int)m_frame;

	m_frame += delta * ((float)numFrames / m_sequence->duration) * m_scale;
	if (m_frame >= numFrames)
	{
		if (m_bLoop)
		{
			assert(fimod(m_frame, numFrames) >= 0 && fimod(m_frame, numFrames) < numFrames);
			m_frame = fimod(m_frame, numFrames);
		}
		else
		{
			m_bFinished = true;
			m_bPlay = false;
			m_frame = (float)numFrames - 1;
		}
	}

	if ((int)m_frame != oldFrame)
		onFrame(frame());

	assert(m_frame >= 0 && m_frame < numFrames);
}

const AnimationFrame& Animator::frame() const
{
	if (m_sequence->frames.empty())
		return nullSequence().nullFrame();

	assert(m_frame >= 0 && m_frame < m_sequence->frames.size());

	return *m_sequence->frames[(uint)m_frame];
}

const AnimationFrame& Animator::frame(uint idx)
{
	assert(idx >= 0 && idx < m_sequence->frames.size());
	if (idx >= m_sequence->frames.size())
		return nullSequence().nullFrame();

	return *m_sequence->frames[idx];
}

const AnimationFrame& Animator::frame(const std::string& sequence, uint idx)
{
	return *m_set->sequences[sequence]->frames[idx];
}

float Animator::duration() const
{
	if (!m_sequence)
		return 0;

	return m_sequence->duration * m_scale;
}

bool Animator::finished() const
{
	return m_bFinished;
}

void Animator::setFrame(int idx)
{
	clamp(idx, 0, (int)m_sequence->frames.size() - 1);
	m_frame = (float)idx;
}

int Animator::frames() const
{
	if (!m_sequence)
		return 0;
	
	return (int)m_sequence->frames.size();
}

const AnimationFrame& Animator::frameAt(float time, float animScale) const
{
	if (m_sequence->frames.empty())
		return nullSequence().nullFrame();

	uint numFrames = m_sequence->frames.size();

	if (m_bLoop)
		return *m_sequence->frames[(uint)(time * ((float)numFrames / m_sequence->duration) / animScale) & (numFrames - 1)];
	else
		return *m_sequence->frames[clamp<uint>((uint)(time * ((float)numFrames / m_sequence->duration) / animScale), 0, numFrames - 1)];
}

bool Animator::has(const std::string& sequenceName)
{
	return m_set->sequences.find(sequenceName) != m_set->sequences.end();
}

const AnimationSequence& Animator::sequence(const std::string& sequence) const
{
	AnimationSequence* seq = 0;

	const AnimationSet::Sequences::iterator i = m_set->sequences.find(sequence);
	if (i != m_set->sequences.end())
	{
		seq = i->second;
	}
	else
	{
		seq = &nullSequence();
	}

	return *seq;
}

/////////////////////////////////////////////////////////////

SpriteFXAnimator::SpriteFXAnimator(const PtrGC<Sprite>& sprite, TextureAnimationSet& animSet) :
	  TextureAnimator(animSet),
	  m_spriteFX(new SpriteFXPlayer(sprite, AppBase().spriteFX()))
{
};

SpriteFXAnimator::~SpriteFXAnimator()
{
	delete m_spriteFX;
}

void SpriteFXAnimator::update(float delta)
{
	TextureAnimator::update(delta);
	m_spriteFX->update(*this);
}
