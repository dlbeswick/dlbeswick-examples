// ---------------------------------------------------------------------------------------------------------
//
// SAppBase
//
// ---------------------------------------------------------------------------------------------------------

#include "pch.h"
#include "AppBase.h"
#include "Terrain/TerrainDB.h"
#include "Standard/Alert.h"
#include "Standard/Config.h"
#include "Standard/CriticalSection.h"
#include "Standard/CriticalSectionBlock.h"
#include "Standard/DXInput.h"
#include "Standard/FileMgr.h"
#include "Standard/Profiler.h"
#include "Standard/DebugInfo.h"
#include "Standard/FileMgr.h"
#include "Standard/ThreadStepper.h"
#include <Standard/Exception/Filesystem.h>

#include "Exception/DialogMgr.h"
#include "Game/Database2D.h"
#include "Game/Level.h"
#include "Game/ObjectMgr.h"
#include "Game/SpriteFX.h"
#include "Render/BillBoarder.h"
#include "Render/Camera.h"
#include "Render/D3DPainter.h"
#include <Render/FontElement.h>
#include "Render/SDeviceD3D.h"
#include "Render/SFont.h"
#include "Render/MeshObject.h"
#include "Render/ParticleFactory.h"
#include "Render/Scene.h"
#include "Render/SkelMesh.h"
#include "Render/TextureAnimationManager.h"
#include "Scripting/Ruby/Ruby.h"
#include "Sound/ISoundProvider.h"
#include "Sound/MusicManager.h"
#include "UI/DialogMgr.h"
#include "UI/UIElement.h"
#include "UI/Dialogs/DlgFatal.h"

RSE_API void trans_func( unsigned int u, EXCEPTION_POINTERS* pExp )
{
	throw SE_Exception(DebugInfo::callstack(pExp->ContextRecord));
}

RSE_API void HandleException(SAppBase* pAppBase, const std::string& message, const std::vector<std::string>& callstack)
{
#ifdef USE_MEMCHECK
	MemCheck().clear();
#endif

	std::string lastFile;
	try
	{
		lastFile = *pAppBase->filesystem().lastOpen();

		if (!callstack.empty())
		{
			dlog << join(callstack, " <- ") << dlog.endl;
			derr << "FATAL EXCEPTION: \n\n" << join(callstack, " <- ") << dlog.endl;
		}
	}
	catch(...)
	{
	}

	try
	{
		if (!pAppBase || !pAppBase->validForDisplay())
			throw message;

		// try fancy exception catch
		pAppBase->onException(callstack, message + "\n\nLast file opened: " + lastFile);
		pAppBase->run();
		delete pAppBase;
	}
	catch (...)
	{
		// last-ditch exception catch (exception during fancy exception)
		try
		{
			alertf("%s\r%s\rLast file opened: %s", join(callstack, " <- ").c_str(), message.c_str(), lastFile.c_str());
			if (pAppBase)
				DestroyWindow(pAppBase->hWnd());
		}
		catch (...)
		{
			MessageBox(0, message.c_str(), "RSE Fatal Error", 0);
		}
	}
}

////

// construct
SAppBase::SAppBase(const std::string& appName, const std::string& defaultDlg, const std::string& cmdLine) :
	_filesystem(0),
	m_appName(appName),
	m_cmdLine(cmdLine),
	m_defaultDlg(defaultDlg),
	m_bRun(false),
	m_bInit(false),
	m_bExit(false),
	m_lastUpdateTime(0),
	m_hWnd(0),
	m_bException(false),
	m_criticalMainLoop(new CriticalSection),
	m_mainLoopStep(0)
{
	m_pCurrent = this;
}

SAppBase*	SAppBase::m_pCurrent;

// StaticMsgProc
LRESULT WINAPI SAppBase::StaticMsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	SAppBase* pSAppBase = (SAppBase*)(DWORD_PTR)GetWindowLongPtr(hWnd, GWL_USERDATA);

	if (pSAppBase && pSAppBase->isRunning())
		return pSAppBase->MsgProc(hWnd, msg, wParam, lParam);
	else
		return DefWindowProc(hWnd, msg, wParam, lParam);
}

void SAppBase::init()
{
	// setup logging
	dlog.add(new dlogprinterfile(Path::localSettings(Path("log.txt"))));
	derr.add(new dlogprinterfile(Path::localSettings(Path("errors.txt"))));

	loadConfig("options.ini", true);
	m_options = &config("options.ini");

	loadConfig("ui.ini", true);
	m_ui = &config("ui.ini");
	
	loadConfig("materials.ini");
	m_materials = &config("materials.ini");

	std::string uistylesConfigName = m_options->get("UI", "StyleFile", std::string("uistyles.ini"));
	loadConfig(uistylesConfigName);
	m_uiStyles = &config(uistylesConfigName);

	// Heap debugging
	if (hasCmdLineOption("heapdebug"))
	{
#if IS_MSVC
		int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
		tmpFlag |= _CRTDBG_CHECK_ALWAYS_DF;
		_CrtSetDbgFlag( tmpFlag );
#else
		throw("Heap debugging is only enabled in MSVC builds.");
#endif
	}

	// create log
	dlog << m_appName << " - " << getCPUVendorID() << " " << getCPUTicksPerSecond() / 1000000.0f << " Mhz" << dlog.endl;

	// get default window icon, if available
	// tbd:mingw: fix
	//HICON groupIcon = LoadIcon(GetModuleHandle(0), "APP");

    // Register the window class.
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, StaticMsgProc, 0L, sizeof(LONG), 
                      GetModuleHandle(NULL), /*groupIcon*/0, LoadCursor(0, IDC_ARROW), NULL, NULL,
                      m_appName.c_str(), NULL };
    RegisterClassEx( &wc );

	//FreeResource(groupIcon);

    // Create the application's window.
    m_hWnd = CreateWindow( m_appName.c_str(), m_appName.c_str(), 
                              WS_OVERLAPPEDWINDOW, 0, 0, 640, 480,
                              GetDesktopWindow(), NULL, wc.hInstance, NULL );

	if (!m_hWnd)
		throwf("Couldn't create a window.");

	// setup device
	bool windowed = hasCmdLineOption("w");
	Point2i fullscreenSize = options().get("Render", "FullscreenSize", Point2i(1024, 768));
	Point2i windowedSize = options().get("Render", "WindowedSize", Point2i(640, 480));
	D3D().newDevice(
		windowed,
		fullscreenSize.x,
		fullscreenSize.y,
		windowedSize.x,
		windowedSize.y
		);

	createFonts();
		
	// create dialog mgr
	m_pDlgMgrSingleton = new SDialogMgr;

	if (!Input().create(GetModuleHandle(NULL)))
		throwf("Couldn't create input");

	if (!Input().createMouse(m_hWnd))
		throwf("Couldn't create mouse");

	// setup ui
	DialogMgr().create();

	m_textureAnimation = new TextureAnimationManager("animations.ini");
	m_particles = new ParticleFactory("particles.ini", m_textureAnimation->set("particles"));
	m_spriteFX = new SpriteFXFactory("spritefx.ini");

	timer().restart();

	m_updateLoopFreq = options().get("Game", "UpdateLoopFreq", 0.0f);

	// create sound
	std::string soundProvider = options().get("Sound", "Provider", std::string("FMODProvider"));
	dlog << "AppBase: Initialising sound provider '" << soundProvider << "'" << dlog.endl;
	m_sound = (ISoundProvider*)Base::newObject(soundProvider);
	if (!m_sound)
		throwf("Couldn't initialize sound provider '" + soundProvider + "'");

	// music manager
	loadConfig("music.ini");
	loadConfig("musicmanager.ini");

	m_music = new MusicManager;

	// Add sounds path
	filesystem().addFileMethod(new FileMethodDiskRelative(pathResources() + m_options->get("Sound", "SoundsPath", std::string("media\\sounds"))));

	// make main loop update thread
#if USE_FIBERS
	m_mainLoopStep = new ThreadStepper(ThreadStepper::Delegate(this, (ThreadStepper::Delegate::FuncType)&SAppBase::doMainLoopUpdate));
	add(*m_mainLoopStep);
#endif
}

// destruct
SAppBase::~SAppBase()
{
	DestroyWindow(m_hWnd);
	delete m_mainLoopStep;
	delete m_pDlgMgrSingleton;
	delete m_textureAnimation;
	delete m_particles;
	//delete m_ruby;
	// collect final garbage before app shutdown
	ptrGCManager.garbage->collectAll();

	for (SavedConfigs::iterator configIt = m_savedConfigs.begin(); configIt != m_savedConfigs.end(); ++configIt)
	{
		Config* config = configIt->second.first;

		if (m_bInit && config->dirty())
		{
			const Path& path = configIt->second.second;
			try
			{
				otextstream streamConfigOut(path.open(std::ios::out));
				config->services()[0]->save(streamConfigOut);
			}
			catch (ExceptionFilesystem& e)
			{
				dlog << "Couldn't open config file for write: " << e.what() << dlog.endl;
			};
		}

		delete config;
	}

	for (Configs::iterator configIt = m_configs.begin(); configIt != m_configs.end(); ++configIt)
	{
		delete configIt->second;
	}
	
	delete m_sound;

	// must destroy directx objects here, or ExitProcess in shutdown sequence will term for us and leak memory
	D3D().destroy();
	Input().destroy();
}

void SAppBase::createFonts()
{
	// font
	if (!Font().createSystemFont("Arial", "arial", 24))
		throwf("Couldn't create SAppBase font");
	Font().set("arial");
	Font().activeFont()->setScale(0.75f);

	if (!Font().copyFont("arial", "smallarial", 0.5f))
		throwf("Couldn't create SAppBase font");

	if (!Font().copyFont("arial", "worldarial", 500.0f))
		throwf("Couldn't create SAppBase font");

	if (!Font().createSystemFont("Arial", "bigarial", 72, 512))
		throwf("Couldn't create bigarial");
	Font().get("bigarial")->setScale(2.0f);

	Font().makeTextureFonts();
}

// run
void SAppBase::run()
{
	if (!m_bInit)
	{
		// always do last
		SetWindowLongPtr(m_hWnd, GWL_USERDATA, (LONG)(DWORD_PTR)this);
		m_bInit = true;
	}

	if (!m_bRun)
	{
		m_bRun = true;
		ShowWindow(m_hWnd, SW_SHOW);
	}

	// setup startup dialog if not running because of exception
	if (!m_bException)
	{
		std::string cmdLineDlg = getCmdLineOption("d");
		try
		{
			DialogMgr().add(cmdLineDlg);
		}
		catch (ExceptionDialogMgr&)
		{
			if (!m_defaultDlg.empty())
				DialogMgr().add(m_defaultDlg);
		}

		DialogMgr().update(0.0001f);
	}

	// init timer
	timer().endFrame();

	bool bNeedsUpdate = false;

	MSG msg; 
	do
	{
		Critical(*m_criticalMainLoop);

		// windows message handling
		while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			if (msg.message == WM_QUIT)
				m_bExit = true;

			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}

		if (m_bExit)
			break;

		// game update
		float delta = (float)(timer().realTime() - m_lastUpdateTime);
		bNeedsUpdate = m_updateLoopFreq == 0 || delta >= (1.0f / m_updateLoopFreq);
		if (bNeedsUpdate)
		{
			timer().endFrame();
			double startFrameTime = timer().realTime();
			m_lastUpdateTime = startFrameTime;

			delta = (float)timer().frame();

			{
				Profile("Dialog Update");

				Input().update();
				DialogMgr().update(delta);
			}

			{
				Profile("Update");
				update(delta);
				SteppableThreadHost::update();
			}

			Input().clear();

			{
				Profile("Render");

				render();
			}
		
			{
				Profile("Garbage");
				ptrGCManager.garbage->collect();
			}

			m_totalFrameTime = timer().realTime() - startFrameTime;
			Profiler().end();
			Profiler().finalise();
			Profiler().start("Total");
		}
	}
	while (!m_bExit);

	m_bRun = false;
}

void SAppBase::update(float delta)
{
	m_music->update(delta);
}

void SAppBase::doMainLoopUpdate()
{
	STEPMARK(mainLoopStep(), "Level Update");

	m_levels.flush();
	for (LevelList::iterator i = m_levels.begin(); i != m_levels.end(); ++i)
	{
		(*i)->update();
	}
}

// render
// guaranteed to be called only after init
void SAppBase::render()
{
	if (D3DD().Present( NULL, NULL, NULL, NULL ) == D3DERR_DEVICELOST)
		D3D().onDeviceLost();

	if (D3D().beginScene())
	{
		DialogMgr().draw();
		
		if (options().get<bool>("Debug", "ShowFPS"))
		{
			char buf[128];
			sprintf(buf, "fps: %f (locked %d fps)", 1.0f / (float)(m_totalFrameTime), (int)(m_updateLoopFreq));
			Font().get("arial")->write(D3DPaint(), buf, 0, 0);
			D3DPaint().draw();
		}

		D3D().endScene();
	}
}

// MsgProc
LRESULT WINAPI SAppBase::MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch( msg )
    {
		case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

		// this is nice and all, but for efficiency reasons we really need to ensure update then draw, in that order.
/*        case WM_PAINT:
			if (validForDisplay())
				render();

            ValidateRect( hWnd, NULL );
            return 0;*/

		case WM_SYSCHAR:
			// resize
			if (wParam == 13)
				D3D().toggleFullscreen();
			return 0;

		case WM_SHOWWINDOW:
			/*Input().onWindowActivate(hWnd);
			DialogMgr().setActiveOSWindow(0);*/
			return 0;
		case WM_ACTIVATE:
			if (LOWORD(wParam) != WA_INACTIVE)
			{
				DialogMgr().setActiveOSWindow(0);
			}
			return 0;
    }

	if (m_bInit)
	{
		HRESULT ret = Input().keyMsg(msg, wParam, lParam);
		if (ret != -1)
			return ret;
	}

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

// onException
void SAppBase::onException(const std::vector<std::string>& callstack, const std::string& message)
{
	m_bException = true;

	if (D3D().isDeviceLost())
		throw(message);

	DialogMgr().removeAll();
	DialogMgr().activeBranch()->addChild(new DlgFatal(callstack, message));
}

// getCmdLineOption
std::string SAppBase::getCmdLineOption(const std::string& option)
{
	// tbd: remove/replace this
	boost::regex regex("-" + option + "\\s?(\\\"([\\w\\s]+)\\\"|(\\w+))");

	boost::match_results<std::string::const_iterator> results;
	boost::regex_search(m_cmdLine, results, regex);

	if (results[2].matched)
		return results[2].str();
	else if (results[1].matched)
		return results[1].str();
	else
		return "";
}

// hasCmdLineOption
bool SAppBase::hasCmdLineOption(const std::string& option)
{
	char* start = strstr(m_cmdLine.c_str(), ("-" + option).c_str());
	
	return start != 0;
}

// validForDisplay
bool SAppBase::validForDisplay()
{
	if (D3D().isDeviceLost())
		return false;

	return /*m_bRun && */m_bInit;
}

void SAppBase::addLevel(Level& l)
{
	assert(!m_levels.contains(&l));
	m_levels.add(&l);
}

void SAppBase::removeLevel(Level& l)
{
	m_levels.remove(&l);
}

Path SAppBase::pathConfig(const std::string& configFileName) const
{
	return pathResources() + Path("cfg") + Path(configFileName);
}

void SAppBase::loadConfig(const std::string& name, bool saveOnExit)
{
	Configs::iterator i = m_configs.find(name);
	if (i != m_configs.end())
		throwf("Config with name '" + name + "' has already been loaded.");

	SavedConfigs::iterator j = m_savedConfigs.find(name);
	if (j != m_savedConfigs.end())
		throwf("Config with name '" + name + "' has already been loaded.");

	Config* config = new Config();
	
	Path configPath = pathConfig(name);
	Path configLocalPath = pathLocalSettings() + name;

	if (saveOnExit || configLocalPath.exists())
		config->add(*new ConfigService(configLocalPath, !saveOnExit));

	config->add(*new ConfigService(configPath, true));

	if (!saveOnExit)
		m_configs[name] = config;
	else
		m_savedConfigs[name] = std::pair<Config*, Path>(config, configLocalPath);
	
	config->load();
}

Config& SAppBase::config(const std::string& name)
{
	Config* config = 0;

	Configs::iterator i = m_configs.find(name);
	if (i == m_configs.end())
	{
		SavedConfigs::iterator j = m_savedConfigs.find(name);
		if (j != m_savedConfigs.end())
		{
			config = j->second.first;
		}
	}
	else
	{
		config = i->second;
	}

	if (!config)
		throwf("No such config '" + name + "' -- has 'loadConfig' been called?");

	return *config;
}

Path SAppBase::pathLocalSettings() const
{
	return Path::localSettings();
}

Path SAppBase::pathResources() const
{
	return (Path::exePath().dirPath() + Path("..")).absolute();
}

FilesystemCompound& SAppBase::filesystem() const 
{ 
	if (!_filesystem)
		_filesystem = constructFilesystem();

	return *_filesystem; 
}

FilesystemCompound* SAppBase::constructFilesystem() const
{
	// File Manager
	static FileMethod* fileMethods[] = {
		new FileMethodDiskRelative(Path(".")),
		0
	};

	return new FilesystemCompound(fileMethods);
}
