// ------------------------------------------------------------------------------------------------
//
// DlgParticles
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "UI/Dialogs/DlgStandardLevel.h"
#include <Standard/Config.h>


class DlgParticles : public DlgStandardLevel
{
	USE_RTTI(DlgParticles, DlgStandardLevel);
public:
	DlgParticles();

	~DlgParticles();

protected:
	virtual void navigation() { mouseNavigation(); }

	virtual void onDrawScene();
	virtual void onActivate();
	virtual void update(float delta);

	void setup(class ParticleEmitter& emitter);
	void updateEmitters();
	void updateSystems();
	void layoutProps();

	void onAnim(const std::string& s);
	void onName(const std::string& s);
	void onAddEmitter();
	void onDelEmitter();
	void onSelectEmitter(const PtrGC<UIElement>&);
	void onSelectSystem(const PtrGC<UIElement>& item);
	void onSaveSystem();
	void onSaveSystemOk(std::string s);
	void onCopySystem();
	void onCopySystemOk(std::string s);
	void onDelSystem();
	void onNewSystem();
	void onCommit();
	void onSystemProperties();

	Config m_config;

	class DlgObjProperties* m_properties;
	PtrGC<class DlgObjProperties> m_systemProperties;

	class UICombo* m_emitters;
	class UICombo* m_systems;

	PtrGC<class ParticleSystem> m_system;
	PtrGC<class ParticleEmitter> m_emitter;
	class TextureAnimationManager* m_anim;

	std::string m_systemName;
	std::string m_lastAnim;
};