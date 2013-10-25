// ------------------------------------------------------------------------------------------------
//
// TextureAnimatorLayered
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "TextureAnimatorLayered.h"

TextureAnimatorLayered::TextureAnimatorLayered(const PtrGC<class Sprite>& sprite, TextureAnimationSet& set, int layers)
{
	assert(layers);

	m_layers.resize(layers);
	for (uint i = 0; i < m_layers.size(); ++i)
	{
		m_layers[i] = new SpriteFXAnimator(sprite, set);
	}
}

TextureAnimatorLayered::~TextureAnimatorLayered()
{
	freeSTL(m_layers);
}

void TextureAnimatorLayered::update(float delta)
{
	for (uint i = 0; i < m_layers.size(); ++i)
		m_layers[i]->update(delta);
}

void TextureAnimatorLayered::clear()
{
	for (uint i = 0; i < m_layers.size(); ++i)
		m_layers[i]->play("");
}

void TextureAnimatorLayered::stop()
{
	for (uint i = 0; i < m_layers.size(); ++i)
		m_layers[i]->stop();
}

void TextureAnimatorLayered::start()
{
	for (uint i = 0; i < m_layers.size(); ++i)
		m_layers[i]->start();
}

const TextureAnimationFrame& TextureAnimatorLayered::frame() const
{
	for (int i = (int)m_layers.size() - 1; i >= 1; --i)
	{
		if (!m_layers[i]->playingNull())
			return m_layers[i]->frame();
	}

	return m_layers[0]->frame();
}

void TextureAnimatorLayered::useSet(class AnimationSet& set)
{
	for (int i = (int)m_layers.size() - 1; i > -1; --i)
		m_layers[i]->useSet(set);
}
