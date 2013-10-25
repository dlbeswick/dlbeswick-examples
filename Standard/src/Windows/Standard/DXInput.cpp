// ---------------------------------------------------------------------------------------------------------
//
// DXInput
//
// ---------------------------------------------------------------------------------------------------------
#include "Standard/pch.h"
#include "Standard/DXInput.h"
#include "Standard/Help.h"
#include "Standard/D3DHelp.h"
#include "Standard/Log.h"
#include <algorithm>

// name data
const char* g_keyNames[256] = {
	"", "", "", "", "", "", "", "",
	"Backspace",
	"Tab", "", "",
	"Clear",
	"Return", "", "",
	"Shift",
	"Ctrl",
	"Menu",
	"Pause",
	"Caps Lock", "", "", "", "", "", "", //1A
	"Escape", "", "", "", "",
	"Space",
	"Page Up",
	"Page Down",
	"End",
	"Home",
	"Left",
	"Up",
	"Right",
	"Down",
	"Select",
	"Print",
	"Execute",
	"Print Screen",
	"Insert",
	"Delete",
	"Help",
	"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "","","","","","","",
	"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P",
	"Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
	"Left Windows Key",
	"Right Windows Key",
	"Apps", "",
	"Sleep",
	"Num 0", "Num 1", "Num 2", "Num 3", "Num 4", "Num 5", "Num 6", "Num 7", "Num 8", "Num 9",
	"*",
	"+",
	"\\",
	"-",
	".",
	"/",
	"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "F13", "F14",
	"F15", "F16", "F17", "F18", "F19", "F20", "F21", "F22", "F23", "F24", "", "", "", "", "", "", "", "",
	"Num Lock",
	"Scroll Lock", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"Left Shift",
	"Right Shift",
	"Left Ctrl",
	"Right Ctrl",
	"Left Menu",
	"Right Menu",
	"Back",
	"Forward",
	"Refresh",
	"Stop",
	"Search",
	"Favourites",
	"Home",
	"Mute",
	"Volume Down",
	"Volume Up",
	"Next Track",
	"Prev Track",
	"Stop Media",
	"Play/Pause",
	"Mail",
	"Media",
	"Launch App 1",
	"Launch App 2", "", "",
	"OEM 1",
	"OEM +",
	"OEM ,",
	"OEM -",
	"OEM .",
	"OEM 2",
	"OEM 3", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"OEM 4",
	"OEM 5",
	"OEM 6",
	"OEM 7",
	"OEM 8", "", "",
	"OEM 102", "", "",
	"IME Process", "", "", "", "", "", "", "", "", "", "",
	"Attn",
	"CrSel",
	"ExSel",
	"ErEOF",
	"Play",
	"Zoom", "",
	"PA1",
	"Clear"
};

const char* g_keyIDs[256] = {
	"", "", "", "", "", "", "", "",
	"BACKSPACE",
	"TAB", "", "",
	"CLEAR",
	"RETURN", "", "",
	"SHIFT",
	"CTRL",
	"MENU",
	"PAUSE",
	"CAPSLOCK", "", "", "", "", "", "", //1A
	"ESCAPE", "", "", "", "",
	"SPACE",
	"PAGEUP",
	"PAGEDOWN",
	"END",
	"HOME",
	"LEFT",
	"UP",
	"RIGHT",
	"DOWN",
	"SELECT",
	"PRINT",
	"EXECUTE",
	"PRINTSCREEN",
	"INSERT",
	"DELETE",
	"HELP",
	"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "","","","","","","",
	"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P",
	"Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
	"LWINDOWS",
	"RWINDOWS",
	"APPS", "",
	"SLEEP",
	"NUM0", "NUM1", "NUM2", "NUM3", "NUM4", "NUM5", "NUM6", "NUM7", "NUM8", "NUM9",
	"*",
	"+",
	"\\",
	"-",
	".",
	"/",
	"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "F13", "F14",
	"F15", "F16", "F17", "F18", "F19", "F20", "F21", "F22", "F23", "F24", "", "", "", "", "", "", "", "",
	"NUMLOCK",
	"SCROLLLOCK", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"LSHIFT",
	"RSHIFT",
	"LCTRL",
	"RCTRL",
	"LMENU",
	"RMENU",
	"BACK",
	"FORWARD",
	"REFRESH",
	"STOP",
	"SEARCH",
	"FAVOURITES",
	"HOME",
	"MUTE",
	"VOLDOWN",
	"VOLUP",
	"NEXTTRACK",
	"PREVTRACK",
	"STOPMEDIA",
	"PLAYPAUSE",
	"MAIL",
	"MEDIA",
	"LAUNCHAPP1",
	"LAUNCHAPP2", "", "",
	"OEM1",
	"OEM+",
	"OEM,",
	"OEM-",
	"OEM.",
	"OEM2",
	"OEM3", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"OEM4",
	"OEM5",
	"OEM6",
	"OEM7",
	"OEM8", "", "",
	"OEM102", "", "",
	"IMEPROCESS", "", "", "", "", "", "", "", "", "", "",
	"ATTN",
	"CRSEL",
	"EXSEL",
	"EREOF",
	"PLAY",
	"ZOOM", "",
	"PA1",
	"CLEAR",
	"", "", "", "", "", "", "", ""
};

// constructor
DXInput::DXInput() :
	m_pDI(0),
	m_pMouse(0),
	m_extMin(0, 0),
	m_extMax(1, 0.75f),
	m_osMouse(false)
{
	m_mouseData.m_size = 0;
	memset(m_keys, 0, sizeof(m_keys));
}


// destructor
DXInput::~DXInput()
{
	destroy();
}

void DXInput::destroy()
{
	if (m_pMouse)
	{
		m_pMouse->Unacquire();
		m_pMouse->Release();
		m_pMouse = 0;
	}

	freeDX(m_pDI);
}

// create
bool DXInput::create(HINSTANCE hInst)
{
	if (DXFAIL(DirectInput8Create(hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void **)&m_pDI, 0)))
		return false;

	return true;
}

// createMouse
bool DXInput::createMouse(HWND hWnd)
{
	if (!m_pMouse)
	{
		if (DXFAIL(m_pDI->CreateDevice(GUID_SysMouse, &m_pMouse, 0)))
			return false;

		if (DXFAIL(m_pMouse->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE)))
			return false;

		if (DXFAIL(m_pMouse->SetDataFormat(&c_dfDIMouse)))
			return false;

		DIPROPDWORD dipdw;
		dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		dipdw.diph.dwObj        = 0;
		dipdw.diph.dwHow        = DIPH_DEVICE;
		dipdw.dwData            = MOUSE_BUFFER_SIZE;

		if (DXFAIL(m_pMouse->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph)))
			return false;
	}
	else
	{
		m_pMouse->Unacquire();
		m_pMouse->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
	}

	return true;
}


// createKeyboard
/*bool DXInput::createKeyboard(HWND hWnd)
{
	if (DXFAIL(m_pDI->CreateDevice(GUID_SysKeyboard, &m_pKeys, 0)))
		return false;

	if (DXFAIL(m_pKeys->SetDataFormat(&c_dfDIKeyboard)))
		return false;

	if (DXFAIL(m_pKeys->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND)))
		return false;

	DIPROPDWORD dipdw;
    dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
    dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dipdw.diph.dwObj        = 0;
    dipdw.diph.dwHow        = DIPH_DEVICE;
    dipdw.dwData            = KEY_BUFFER_SIZE;

	if (DXFAIL(m_pKeys->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph)))
		return false;

	return true;
}*/


// update
void DXInput::update()
{
	updateMouse();
	updateKeys();
	GetKeyboardState(m_keys);
}


// clear
void DXInput::clear()
{
	m_mouseData.m_size = 0;
	m_keyDown.clear();
	m_keyUp.clear();
	m_keyChar.clear();
}


// updateMouse
void DXInput::updateMouse()
{
	m_mouseData.m_size = 0;

	m_mouseDeltaX = 0;
	m_mouseDeltaY = 0;

	if (!m_pMouse || m_osMouse)
		return;

	if (FAILED(m_pMouse->Acquire()))
		return;

	m_mouseData.m_size = MOUSE_BUFFER_SIZE;

	if (FAILED(m_pMouse->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), m_mouseData.m_data, (unsigned long *)&m_mouseData.m_size, 0)))
	{
		m_mouseData.m_size = 0;
		return;
	}

	for (uint i = 0; i < m_mouseData.m_size; i++)
	{
		m_mouseData.m_bActive[i] = true;

		if (m_mouseData.m_data[i].dwOfs == DIMOFS_X)
		{
			m_mouseDeltaX += m_mouseData.m_data[i].dwData;
		}
		else if (m_mouseData.m_data[i].dwOfs == DIMOFS_Y)
		{
				m_mouseDeltaY += m_mouseData.m_data[i].dwData;
		}

		for (uint k = 0; k < DXInput::MOUSE_BUTTONS; k++)
		{
			if (m_mouseData.m_data[i].dwOfs == DIMOFS_BUTTON0 + k)
			{
				if (m_mouseData.m_data[i].dwData & 0x80)
				{
					m_mousePressed[k] = true;
				}
				else
				{
					m_mousePressed[k] = false;
				}
			}
		}
	}
}


// updateKeys
void DXInput::updateKeys()
{
}


// keyMsg
HRESULT DXInput::keyMsg(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
		{
			for (uint i = 0; i < (uint)(lParam & 0xFFFF); i++)
				m_keyDown.push_back((int)wParam);

			return 0;
		}
	case WM_KEYUP:
		{
			for (uint i = 0; i < (uint)(lParam & 0xFFFF); i++)
				m_keyUp.push_back((int)wParam);

			return 0;
		}
		break;
	case WM_CHAR:
		{
			for (uint i = 0; i < (uint)(lParam & 0xFFFF); i++)
				m_keyChar.push_back((int)wParam);

			return 0;
		}
		break;
	}

/*	if (m_osMouse)
	{
		int& idx = m_mouseData.m_used;
		if (idx < m_mouseData.CAPACITY)
		{
			switch (msg)
			{
			case WM_LBUTTONDOWN:
				m_mouseData.m_bActive[idx] = true;
				m_mouseData.m_data[idx].dwOfs = DIMOFS_BUTTON0;
				m_mouseData.m_data[idx].dwData = 0x80;
				++idx;
				return 0;
			case WM_MBUTTONDOWN:
				m_mouseData.m_bActive[idx] = true;
				m_mouseData.m_data[idx].dwOfs = DIMOFS_BUTTON1;
				m_mouseData.m_data[idx].dwData = 0x80;
				++idx;
				return 0;
			case WM_RBUTTONDOWN:
				m_mouseData.m_bActive[idx] = true;
				m_mouseData.m_data[idx].dwOfs = DIMOFS_BUTTON2;
				m_mouseData.m_data[idx].dwData = 0x80;
				++idx;
				return 0;
			case WM_LBUTTONUP:
				m_mouseData.m_bActive[idx] = true;
				m_mouseData.m_data[idx].dwOfs = DIMOFS_BUTTON0;
				m_mouseData.m_data[idx].dwData = 0;
				++idx;
				return 0;
			case WM_MBUTTONUP:
				m_mouseData.m_bActive[idx] = true;
				m_mouseData.m_data[idx].dwOfs = DIMOFS_BUTTON1;
				m_mouseData.m_data[idx].dwData = 0;
				++idx;
				return 0;
			case WM_RBUTTONUP:
				m_mouseData.m_bActive[idx] = true;
				m_mouseData.m_data[idx].dwOfs = DIMOFS_BUTTON2;
				m_mouseData.m_data[idx].dwData = 0;
				++idx;
				return 0;
			}
		}
	}*/

	return -1;
}

int DXInput::keyFromId(const std::string& id)
{
	for (uint i = 0; i < 256; ++i)
		if (g_keyIDs[i] == id)
			return i;

	return 0;
}

const char* DXInput::idFromKey(int key)
{
	if (key < 0 || key > 255)
		return "";

	return g_keyIDs[key];
}

int DXInput::keyFromName(const std::string& name)
{
	for (uint i = 0; i < 256; ++i)
		if (g_keyNames[i] == name)
			return i;

	return 0;
}

const char* DXInput::nameFromKey(int key)
{
	if (key < 0 || key > 255)
		return "";

	return g_keyNames[key];
}

void DXInput::setOSMouse(bool b)
{
	m_osMouse = b;

	if (m_pMouse && m_osMouse)
	{
		m_pMouse->Unacquire();
		ShowCursor(true);
	}
}

void DXInput::reset()
{
	zero(m_mousePressed);
	zero(m_mouseData);
}
