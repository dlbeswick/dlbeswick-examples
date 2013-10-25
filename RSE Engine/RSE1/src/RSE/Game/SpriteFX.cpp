// ------------------------------------------------------------------------------------------------
//
// SpriteFX
// System to automatically trigger events on sprite animations.
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "SpriteFX.h"
#include "AppBase.h"
#include "PathResource.h"
#include "Render/TextureAnimator.h"
#include "Render/TextureAnimationManager.h"
#include "Render/TextureAnimationTypes.h"
#include "UI/Controls/UIPropertyEditor.h"
#include <Standard/Config.h>
#include "Standard/Exception/TextStreamParseException.h"

#define METADATA SpriteFX
IMPLEMENT_RTTI(SpriteFX);

STREAM
	STREAMVAR(frame);
}

EDITSTREAM
	EDITSTREAMNAME("ID", id, "Optional identifier for the effect. Several effects can share the same identifier (multiple effects can be stopped at once using this method, for example).");
}
#undef METADATA

#define METADATA SpriteFXPos
IMPLEMENT_RTTI(SpriteFXPos);

EDITSTREAM
	EDITSTREAMNAME("Pos", m_pos, "Position used for effect (click on viewing window)");
	EDITSTREAMNAME("Rot", m_rot, "Rotation used for effect (click on viewing window)");
	EDITSTREAMNAME("Attach To Parent?", m_attachToParent, "If true, the effect is attached to the sprite. If false, the effect floats in the world.");
	EDITSTREAMNAME("Stop On End?", m_stopOnAnimEnd, "If true, the effect is stopped when the animation completes.");
}
#undef METADATA

IMPLEMENT_RTTI(SpriteFXIterating);

// Player
SpriteFXPlayer::~SpriteFXPlayer()
{
	for (FXMap::iterator i = m_fx.begin(); i != m_fx.end(); ++i)
	{
		for (uint j = 0; j < i->second.size(); ++j)
		{
			SpriteFX* fx = i->second[j];
			if (fx->shouldStopOnAnimationEnd())
				fx->stop(*this);
			delete fx;
		}
	}
}

void SpriteFXPlayer::update(ITextureAnimator& animator)
{
	play(animator);

	FXList& fxList = m_current->second;
	if (fxList.empty())
		return;

	int frame = (int)animator.frameIdx();
	if (frame != m_lastFrame)
	{
		for (uint i = 0; i < fxList.size(); ++i)
		{
			SpriteFX& fx = *fxList[i];
			if ((m_lastFrame < fx.frame && frame >= fx.frame) ||
				(m_lastFrame > fx.frame && frame <= fx.frame))
			{
				fx.start(*this);
			}
		}

		m_lastFrame = frame;
	}
}

void SpriteFXPlayer::play(ITextureAnimator& animator)
{
	if (m_currentSet != &animator.set() || m_currentSequence != &animator.sequence())
	{
		m_lastFrame = -1;

		if (m_current != m_fx.end())
		{
			FXList& fx = m_current->second;
			for (uint i = 0; i < fx.size(); ++i)
			{
				SpriteFX& curFX = *fx[i];
				if (curFX.shouldStopOnAnimationEnd())
					curFX.stop(*this);
			}
		}

		// must clear fx cache when changing sets
		if (m_currentSet != &animator.set())
			m_fx.clear();

		FXMap::iterator it = m_fx.find(animator.sequence().name);
		if (it == m_fx.end())
		{
			it = m_fx.insert(FXMap::value_type(animator.sequence().name, FXList())).first;
			m_factory.fill(it->second, animator.set().name, animator.sequence().name);
		}

		m_current = it;

		m_currentSet = &animator.set();
		m_currentSequence = &animator.sequence();
	}
}

void SpriteFXPlayer::retrigger(ITextureAnimator& animator)
{
	FXList& fxList = m_current->second;
	for (uint i = 0; i < fxList.size(); ++i)
	{
		fxList[i]->stop(*this);
		if (fxList[i]->frame == (int)animator.frameIdx())
			fxList[i]->start(*this);
	}
}

void SpriteFXPlayer::stop(int id)
{
	FXList& fxList = m_current->second;
	for (uint i = 0; i < fxList.size(); ++i)
	{
		if (fxList[i]->id == id || id == -1)
			fxList[i]->stop(*this);
	}
}

SpriteFXPlayer::FXList SpriteFXPlayer::get(int id)
{
	FXList idList;

	FXList& fxList = m_current->second;
	for (uint i = 0; i < fxList.size(); ++i)
	{
		if (fxList[i]->id == id)
			idList.push_back(fxList[i]);
	}

	return idList;
}

const SpriteFX* SpriteFXPlayer::findFirst(const RTTI& what, const std::string& name) const
{
	FXMap::const_iterator obj;
	if (name.empty())
		obj = m_current;
	else
		obj = m_fx.find(name);

	if (obj == m_fx.end())
		return 0;

	const FXList& fxList = obj->second;
	for (uint i = 0; i < fxList.size(); ++i)
	{
		const SpriteFX& o = *fxList[i];
		if (o.rtti().isA(what))
			return &o;
	}

	return 0;
}

// Factory
SpriteFXFactory::SpriteFXFactory(const std::string& configName) :
	m_configName(configName),
	m_config(new Config)
{
	m_config->add(*new ConfigService(PathResource(configName), false));
	m_config->load();

	try
	{
		m_config->linkImports(AppBase().textureAnimation().config());
		m_config->load(m_fx);
	}
	catch(TextStreamParseException& s)
	{
		dlog << "SpriteFXFactory could not load config file " << configName << " (" << s.what() << ")" << dlog.endl;
		// too bad!
		m_fx.clear();
	}
}

SpriteFXFactory::~SpriteFXFactory()
{
	for (SetMap::iterator i = m_fx.begin(); i != m_fx.end(); ++i)
	{
		for (FXMap::iterator j = i->second.begin(); j != i->second.end(); ++j)
			freeSTL(j->second);
	}
}

void SpriteFXFactory::writeToConfig()
{
	otextstream outSpriteFXIni(AppBase().pathConfig("spritefx.ini").open(std::ios::out));
	m_config->services()[0]->save(outSpriteFXIni);
}

void SpriteFXFactory::fill(std::vector<SpriteFX*>& list, const std::string& set, const std::string& sequence)
{
	SetMap::iterator i = m_fx.find(set);
	if (i != m_fx.end())
	{
		FXMap::iterator j = i->second.find(sequence);
		if (j != i->second.end())
		{
			list.resize(0);

			FXList& l = j->second;
			for (uint i = 0; i < l.size(); ++i)
			{
				list.push_back(l[i]->clone<SpriteFX>());
			}
		}
	}
}

void SpriteFXFactory::update(const std::vector<SpriteFX*>& list, const std::string& set, const std::string& sequence)
{
	FXList& l = m_fx[set][sequence];
	l.resize(0);
	for (uint i = 0; i < list.size(); ++i)
	{
		l.push_back(list[i]->clone<SpriteFX>());
	}

	m_config->set(set, sequence, l);
}

///////////////////////////////

void SpriteFXIterating::start(SpriteFXPlayer& mgr)
{
	SpriteFXPlayer::FXList list = mgr.get(id);
	for (uint i = 0; i < list.size(); ++i)
		start(mgr, list[i]);
};

void SpriteFXIterating::stop(SpriteFXPlayer& mgr)
{
	SpriteFXPlayer::FXList list = mgr.get(id);
	for (uint i = 0; i < list.size(); ++i)
		stop(mgr, list[i]);
};
