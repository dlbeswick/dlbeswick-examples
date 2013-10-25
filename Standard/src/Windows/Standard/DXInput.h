// ---------------------------------------------------------------------------------------------------------
//
// DXInput
//
// ---------------------------------------------------------------------------------------------------------
#ifndef STANDARD_DXINPUT_H
#define STANDARD_DXINPUT_H

#define DIRECTINPUT_VERSION 0x0800

#include "Standard/Math.h"
#include "Standard/Singleton.h"
#include "Standard/Help.h"
#include <dinput.h> // tbd: remove dinput structure from mouse data so this can be moved to implementation
#include <vector>


class STANDARD_API DXInput : public Singleton<DXInput>
{
public:
	DXInput();
	~DXInput();

	void destroy();

	bool create(HINSTANCE hInst);
	bool createMouse(HWND hWnd);
	//bool createKeyboard(HWND hWnd);
	void update();
	void clear();
	HRESULT keyMsg(UINT msg, WPARAM wParam, LPARAM lParam);

	// sDxData struct
	template<uint S> struct sDXData
	{
		const static int CAPACITY = S;
		uint m_size;
		DIDEVICEOBJECTDATA m_data[S];
		bool m_bActive[S];
	};

	void reset();

	// mouse functions
	enum { MOUSE_BUFFER_SIZE = 128 };
	enum { MOUSE_BUTTONS = 8 };

	typedef sDXData<MOUSE_BUFFER_SIZE> MouseData;

	int mouseDeltaX() const { return m_mouseDeltaX; }
	int mouseDeltaY() const { return m_mouseDeltaY; }
	bool mousePressed(int button) { return m_mousePressed[button]; }
	MouseData &mouseData() { return m_mouseData; }

	void setOSMouse(bool b);

	// keyboard functions
	typedef std::vector<int> KeyList;

	KeyList &keyDown() { return m_keyDown; }
	KeyList &keyUp() { return m_keyUp; }
	KeyList &keyChar() { return m_keyChar; }
	bool key(int keyCode) { if (keyCode < 0 || keyCode > 255) return false; return (m_keys[keyCode] & 0xF0) != 0; }

	int keyFromId(const std::string& id);
	const char* idFromKey(int key);
	int keyFromName(const std::string& name);
	const char* nameFromKey(int key);

private:
	void updateMouse();
	void updateKeys();

	int m_mouseDeltaX;
	int m_mouseDeltaY;
	DMath::Point2 m_extMin;
	DMath::Point2 m_extMax;

	uchar m_keys[256];

	IDirectInput8* m_pDI;

	IDirectInputDevice8* m_pMouse;
	MouseData m_mouseData;
	bool m_mousePressed[MOUSE_BUTTONS];
	bool m_osMouse;

	KeyList m_keyDown;
	KeyList m_keyUp;
	KeyList	m_keyChar;
};

inline DXInput &Input() { return DXInput::instance(); }

#endif
