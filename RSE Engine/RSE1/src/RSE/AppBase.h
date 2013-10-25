// ---------------------------------------------------------------------------------------------------------
//
// AppBase
//
// ---------------------------------------------------------------------------------------------------------
#ifndef RSE_APPBASE_H
#define RSE_APPBASE_H


#include <Standard/Timer.h>
#include <Standard/Path.h>
#include <Standard/PendingList.h>
#include <Standard/PtrGC.h>
#include <Standard/SteppableThreadHost.h>

#if IS_GNUC
#include <excpt.h>
#endif

class Config;
class FilesystemCompound;
class ISoundProvider;
class MusicManager;
class Level;
class ParticleFactory;
class SpriteFXFactory;
class TextureAnimationManager;
class ThreadStepper;


class RSE_API SAppBase : public SteppableThreadHost
{
public:
	SAppBase(const std::string& appName, const std::string& defaultDlg, const std::string& cmdLine);
	virtual ~SAppBase();

	FilesystemCompound& filesystem() const;

	virtual void run();
	virtual bool validForDisplay();
	virtual bool isRunning() const { return m_bRun; }
	virtual void render();
	virtual void exit() { m_bExit = true; }
	
	virtual void onException(const std::vector<std::string>& callstack, const std::string& message);

	const std::string& name() { return m_appName; }
	float freq() { return (float)m_updateLoopFreq; }
	HWND hWnd() { return m_hWnd; }

	Timer&						timer()						{ return m_timer; }
	TextureAnimationManager&	textureAnimation() const	{ return *m_textureAnimation; }
	ParticleFactory&			particles() const			{ return *m_particles; }
	ISoundProvider&				sound() const				{ return *m_sound; }
	MusicManager&				music() const				{ return *m_music; }
	SpriteFXFactory&			spriteFX() const			{ return *m_spriteFX; }
	ThreadStepper&				mainLoopStep() const		{ return *m_mainLoopStep; }

	// commandline
	std::string getCmdLineOption(const std::string& option);
	bool hasCmdLineOption(const std::string& option);

	/// Returns a config with the given filename, previously created with loadConfig. 
	Config& config(const std::string& name);

	/// Creates and loads a config from the resources folder with the given name.
	/// Modified settings are saved in the 'local' settings directory, but the original config is never changed.
	/// Services are defined with paths pointing to config files in the master and local config directories.
	/// Configs are saved on exit if mutable.
	void loadConfig(const std::string& name, bool saveOnExit = false);

	Config& options()			{ return *m_options; }
	Config& ui()				{ return *m_ui; }
	Config& uiStyles()			{ return *m_uiStyles; }
	Config& materials()			{ return *m_materials; }

	// levels
	void addLevel(Level& l);
	void removeLevel(Level& l);

	static SAppBase& current() { return *m_pCurrent; }

	class CriticalSection& criticalMainLoop() const { return *m_criticalMainLoop; }

	virtual Path pathConfig(const std::string& configFilename) const;
	virtual Path pathLocalSettings() const;
	virtual Path pathResources() const;

protected:
	template <class T> friend void RunApp(const std::string& cmdLine);

	virtual void init();
	virtual void createFonts();
	virtual FilesystemCompound* constructFilesystem() const;

	// windows messaging
	virtual LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT WINAPI StaticMsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

	virtual void update(float delta);
	void doMainLoopUpdate();
	
	void showAxis();

	mutable FilesystemCompound* _filesystem;

	bool				m_bException;
	bool				m_bRun;
	bool				m_bInit;
	bool				m_bExit;
	std::string			m_cmdLine;
	std::string			m_appName;
	std::string			m_defaultDlg;
	HWND				m_hWnd;

	Config*				m_options;
	Config*				m_ui;
	Config*				m_uiStyles;
	Config*				m_materials;

	Timer				m_timer;
	TextureAnimationManager* m_textureAnimation;
	SpriteFXFactory*	m_spriteFX;
	ParticleFactory*	m_particles;

	ISoundProvider*		m_sound;
	MusicManager*		m_music;

	ThreadStepper*		m_mainLoopStep;

	double				m_updateLoopFreq;
	double				m_lastUpdateTime;
	double				m_totalFrameTime;

	static SAppBase* m_pCurrent;
	
	class SDialogMgr* m_pDlgMgrSingleton;
	class CriticalSection* m_criticalMainLoop;

	class Ruby*			m_ruby;

	typedef PendingListNullRemover<PtrGC<class Level> > LevelList;
	LevelList m_levels;

private:
	typedef stdext::hash_map<std::string, Config*> Configs;
	typedef stdext::hash_map<std::string, std::pair<Config*, Path> > SavedConfigs;
	Configs m_configs;
	SavedConfigs m_savedConfigs;
};

inline static SAppBase& AppBase() { return SAppBase::current(); }

RSE_API void HandleException(SAppBase* pAppBase, const std::string& message, const std::vector<std::string>& callstack);

class SE_Exception
{
public:
    std::vector<std::string> m_callstack;

	SE_Exception( const std::vector<std::string>& callstack ) : m_callstack( callstack ) {}
    ~SE_Exception() {}
};

RSE_API void trans_func( unsigned int u, EXCEPTION_POINTERS* pExp );

// RunApp
// call this to create your App's object and start the message loop
template <class T> void RunApp(const std::string& cmdLine)
{
	SAppBase* pAppBase = 0;

#if IS_MSVC
	_set_se_translator(trans_func);
#else
	//__try1(trans_func);
#endif

	try
	{
		// make AppBase
		pAppBase = new T(cmdLine);
		pAppBase->init();

		pAppBase->run();

		delete pAppBase;
	}
	catch (const std::exception& s)
	{
		HandleException(pAppBase, s.what(), std::vector<std::string>());
	}
	catch (SE_Exception e)
	{
		HandleException(pAppBase, "", e.m_callstack);
	}
};

#endif
