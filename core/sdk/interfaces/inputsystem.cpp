#include "inputsystem.h"
#include "..\..\utils\utils.h"
#include "../../utils/hook/hooks.h"
#include "../../utils/render/ui/menu.h"
// used: get_x_lparam, get_y_lparam
#include <windowsx.h>


typedef void (*SetRelativeMouseModeFn)(void*, bool);
typedef bool (*GetRelativeMouseModeFn)(void*);
typedef void (*SDLWindowsGrab)(void*, bool);

void IInputSystem::SetRelativeMouseMode(bool enable) {
	SetRelativeMouseModeFn func = reinterpret_cast<SetRelativeMouseModeFn>((*reinterpret_cast<void***>(this))[76]);
	func(this, enable);
} 
void IInputSystem::SetWindowsGrab(bool enable) {
	SDLWindowsGrab func = reinterpret_cast<SDLWindowsGrab>((*reinterpret_cast<void***>(this))[73]);
	func(this, enable);
}
bool Input::OnWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// prevent process when e.g. binding something in-menu
	if (wParam != VK_INSERT && wParam != VK_END && menu::m_opened)
		return false;

	// current active key
	int nKey = 0;
	// current active key state
	KeyState_t state = KEY_STATE_NONE;

	switch (uMsg)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if (wParam == VK_SHIFT || wParam == VK_LSHIFT || wParam == VK_RSHIFT ||
			wParam == VK_CONTROL || wParam == VK_LCONTROL || wParam == VK_RCONTROL ||
			wParam == VK_MENU || wParam == VK_LMENU || wParam == VK_RMENU)
		{
			nKey = static_cast<int>(wParam);
			state = KEY_STATE_DOWN;
		}
		else if (wParam < 256U)
		{
			nKey = static_cast<int>(wParam);
			state = KEY_STATE_DOWN;
		}
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		if (wParam == VK_SHIFT || wParam == VK_LSHIFT || wParam == VK_RSHIFT ||
			wParam == VK_CONTROL || wParam == VK_LCONTROL || wParam == VK_RCONTROL ||
			wParam == VK_MENU || wParam == VK_LMENU || wParam == VK_RMENU)
		{
			nKey = static_cast<int>(wParam);
			state = KEY_STATE_UP;
		}
		else if (wParam < 256U)
		{
			nKey = static_cast<int>(wParam);
			state = KEY_STATE_UP;
		}
		break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
		nKey = VK_LBUTTON;
		state = uMsg == WM_LBUTTONUP ? KEY_STATE_UP : KEY_STATE_DOWN;
		break;
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_RBUTTONDBLCLK:
		nKey = VK_RBUTTON;
		state = uMsg == WM_RBUTTONUP ? KEY_STATE_UP : KEY_STATE_DOWN;
		break;
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MBUTTONDBLCLK:
		nKey = VK_MBUTTON;
		state = uMsg == WM_MBUTTONUP ? KEY_STATE_UP : KEY_STATE_DOWN;
		break;
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_XBUTTONDBLCLK:
		nKey = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? VK_XBUTTON1 : VK_XBUTTON2);
		state = uMsg == WM_XBUTTONUP ? KEY_STATE_UP : KEY_STATE_DOWN;
		break;
	default:
		return false;
	}
	// Check the state of special keys using GetAsyncKeyState
	if (nKey == VK_SHIFT || nKey == VK_LSHIFT || nKey == VK_RSHIFT ||
		nKey == VK_CONTROL || nKey == VK_LCONTROL || nKey == VK_RCONTROL ||
		nKey == VK_MENU || nKey == VK_LMENU || nKey == VK_RMENU)
	{
		SHORT asyncState = _import(GetAsyncKeyState).safe()(nKey);
		if (asyncState & 0x8000) // If the high-order bit is set, the key is down
		{
			state = KEY_STATE_DOWN;
		}
		else
		{
			state = KEY_STATE_UP;
		}
	}

	// save key states
	if (state == KEY_STATE_UP && arrKeyState[nKey] == KEY_STATE_DOWN) // if swap states it will be pressed state
		arrKeyState[nKey] = KEY_STATE_RELEASED;
	else
		arrKeyState[nKey] = state;

	return true;
}

bool Input::GetBindState(KeyBind_t& keyBind)
{
	if (keyBind.uKey == 0U)
		return false;

	auto nKey = keyBind.uKey;

	if (nKey == VK_SHIFT || nKey == VK_LSHIFT || nKey == VK_RSHIFT ||
		nKey == VK_CONTROL || nKey == VK_LCONTROL || nKey == VK_RCONTROL ||
		nKey == VK_MENU || nKey == VK_LMENU || nKey == VK_RMENU)
	{
		SHORT asyncState = _import(GetAsyncKeyState).safe()(nKey);
		if (keyBind.nMode == 0) // Hold mode
		{
			keyBind.bEnable = (asyncState & 0x8000) != 0;
		}
		else if (keyBind.nMode == 1) // Toggle mode
		{
			if ((asyncState & 0x8001) == 0x8001) // Key is down and was previously up
			{
				keyBind.bEnable = !keyBind.bEnable;
			}
		}
	}
	else {
		SHORT asyncState = _import(GetAsyncKeyState).safe()(nKey);

		switch (keyBind.nMode)
		{
		case 0:
			keyBind.bEnable = IsKeyDown(keyBind.uKey);
			break;
		case 1:

			if ((asyncState & 0x8001) == 0x8001) // Key is down and was previously up
			{
				keyBind.bEnable = !keyBind.bEnable;
			}
			break;
		}

	}
	return keyBind.bEnable;
}
