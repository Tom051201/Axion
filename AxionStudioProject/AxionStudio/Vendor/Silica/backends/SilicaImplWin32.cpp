#include "SilicaImplWin32.h"

#include <windowsx.h>

#include "../include/Renderer.h"
#include "../include/SWidget.h"

namespace Silica {

	struct BackendStateWin32 {
		HWND hwnd = nullptr;
		float clientWidth = 0.0f;
		float clientHeight = 0.0f;
	};

	static BackendStateWin32 s_state;
	static HCURSOR s_currentCursor = LoadCursor(NULL, IDC_ARROW);

	// -- Helper to map Silica Keys to Win32 Virtual Keys --
	static int getWin32VKCode(Key key) {
		switch (key) {
			case Key::Left: return VK_LEFT;
			case Key::Right: return VK_RIGHT;
			case Key::Up: return VK_UP;
			case Key::Down: return VK_DOWN;
			case Key::Backspace: return VK_BACK;
			case Key::Delete: return VK_DELETE;
			case Key::Enter: return VK_RETURN;
			case Key::Escape: return VK_ESCAPE;
			case Key::Space: return VK_SPACE;
			case Key::Tab: return VK_TAB;
			case Key::LeftShift: return VK_LSHIFT;
			case Key::RightShift: return VK_RSHIFT;
			case Key::LeftControl: return VK_LCONTROL;
			case Key::RightControl: return VK_RCONTROL;
			case Key::LeftAlt: return VK_LMENU;
			case Key::RightAlt: return VK_RMENU;
		}

		// -- Letters --
		if (key >= Key::A && key <= Key::Z) {
			return 0x41 + (static_cast<int>(key) - static_cast<int>(Key::A));
		}

		// -- Numbers --
		if (key >= Key::Num0 && key <= Key::Num9) {
			return 0x30 + (static_cast<int>(key) - static_cast<int>(Key::Num0));
		}

		return 0;
	}

	namespace Platform {
		void setMouseCapture(bool capture) {
			if (capture) {
				SetCapture(s_state.hwnd);
			}
			else {
				ReleaseCapture();
			}
		}

		void setCursor(Cursor cursor) {
			switch (cursor) {
				case Cursor::Arrow: { s_currentCursor = LoadCursor(NULL, IDC_ARROW); break; }
				case Cursor::Hand: { s_currentCursor = LoadCursor(NULL, IDC_HAND); break; }
				case Cursor::TextInput: { s_currentCursor = LoadCursor(NULL, IDC_IBEAM); break; }
				case Cursor::ResizeEW: { s_currentCursor = LoadCursor(NULL, IDC_SIZEWE); break; }
				case Cursor::ResizeNS: { s_currentCursor = LoadCursor(NULL, IDC_SIZENS); break; }
			}
			::SetCursor(s_currentCursor);
		}

		bool isKeyDown(Key key) {
			int vk = getWin32VKCode(key);
			return (GetAsyncKeyState(vk) & 0x8000) != 0;
		}

		void setClipboardText(const std::string& text) {
			if (!OpenClipboard(s_state.hwnd)) return;
			EmptyClipboard();

			HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
			if (hMem) {
				void* memPtr = GlobalLock(hMem);
				if (memPtr) {
					std::memcpy(memPtr, text.c_str(), text.size() + 1);
					GlobalUnlock(hMem);
					SetClipboardData(CF_TEXT, hMem);
				}
			}
			CloseClipboard();
		}

		std::string getClipboardText() {
			if (!IsClipboardFormatAvailable(CF_TEXT)) return "";
			if (!OpenClipboard(s_state.hwnd)) return "";

			std::string result = "";
			HANDLE hData = GetClipboardData(CF_TEXT);
			if (hData) {
				char* textPtr = static_cast<char*>(GlobalLock(hData));
				if (textPtr) {
					result = textPtr;
					GlobalUnlock(hData);
				}
			}
			CloseClipboard();
			return result;
		}
	}

	// ----- Win32 Key Mapper -----
	static Key mapWin32KeyToSilica(WPARAM wParam) {
		switch (wParam) {
			case VK_LEFT: return Key::Left;
			case VK_RIGHT: return Key::Right;
			case VK_UP: return Key::Up;
			case VK_DOWN: return Key::Down;
			case VK_BACK: return Key::Backspace;
			case VK_DELETE: return Key::Delete;
			case VK_RETURN: return Key::Enter;
			case VK_ESCAPE: return Key::Escape;
			case VK_SPACE: return Key::Space;
			case VK_TAB: return Key::Tab;
			case VK_SHIFT: return Key::LeftShift;   // Note: GetKeyState is needed for Left vs Right
			case VK_CONTROL: return Key::LeftControl;
			case VK_MENU: return Key::LeftAlt;
		}

		// A-Z
		if (wParam >= 0x41 && wParam <= 0x5A) {
			return static_cast<Key>(static_cast<uint32_t>(Key::A) + (wParam - 0x41));
		}
		// 0-9
		if (wParam >= 0x30 && wParam <= 0x39) {
			return static_cast<Key>(static_cast<uint32_t>(Key::Num0) + (wParam - 0x30));
		}

		return Key::Unknown;
	}

	bool ImplWin32_init(HWND hwnd) {
		s_state.hwnd = hwnd;

		RECT rect;
		GetClientRect(hwnd, &rect);
		s_state.clientWidth = static_cast<float>(rect.right - rect.left);
		s_state.clientHeight = static_cast<float>(rect.bottom - rect.top);

		return true;
	}

	void ImplWin32_shutdown() {
		s_state.hwnd = nullptr;
		s_state.clientHeight = 0.0f;
		s_state.clientHeight = 0.0f;
	}

	bool ImplWin32_wndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, WidgetPtr rootWidget) {
		if (!rootWidget) return false;

		switch (msg) {
			case WM_SETCURSOR: {
				if (LOWORD(lParam) == HTCLIENT) {
					::SetCursor(s_currentCursor);
					return true;
				}
				return false;
			}
			case WM_SIZE: {
				s_state.clientWidth = static_cast<float>(LOWORD(lParam));
				s_state.clientHeight = static_cast<float>(HIWORD(lParam));
				return false;
			}
			case WM_MOUSEMOVE: {
				float x = static_cast<float>(GET_X_LPARAM(lParam));
				float y = static_cast<float>(GET_Y_LPARAM(lParam));

				Platform::setCursor(Platform::Cursor::Arrow);

				Renderer::processMouseMove(rootWidget, s_state.clientWidth, s_state.clientHeight, x, y);
				return false;
			}
			case WM_LBUTTONDOWN: {
				float mx = static_cast<float>(GET_X_LPARAM(lParam));
				float my = static_cast<float>(GET_Y_LPARAM(lParam));

				Renderer::processMouseDown(rootWidget, s_state.clientWidth, s_state.clientHeight, mx, my, MouseButton::Left);
				return false;
			}
			case WM_LBUTTONUP: {
				float x = static_cast<float>(GET_X_LPARAM(lParam));
				float y = static_cast<float>(GET_Y_LPARAM(lParam));

				Renderer::processMouseUp(rootWidget, s_state.clientWidth, s_state.clientHeight, x, y, MouseButton::Left);
				return false;
			}
			case WM_RBUTTONDOWN: {
				float mx = static_cast<float>(GET_X_LPARAM(lParam));
				float my = static_cast<float>(GET_Y_LPARAM(lParam));
				Renderer::processMouseDown(rootWidget, s_state.clientWidth, s_state.clientHeight, mx, my, MouseButton::Right);
				return false;
			}
			case WM_RBUTTONUP: {
				float x = static_cast<float>(GET_X_LPARAM(lParam));
				float y = static_cast<float>(GET_Y_LPARAM(lParam));

				Renderer::processMouseUp(rootWidget, s_state.clientWidth, s_state.clientHeight, x, y, MouseButton::Right);
				return false;
			}
			case WM_MBUTTONDOWN: {
				float mx = static_cast<float>(GET_X_LPARAM(lParam));
				float my = static_cast<float>(GET_Y_LPARAM(lParam));
				Renderer::processMouseDown(rootWidget, s_state.clientWidth, s_state.clientHeight, mx, my, MouseButton::Middle);
				return false;
			}
			case WM_MBUTTONUP: {
				float x = static_cast<float>(GET_X_LPARAM(lParam));
				float y = static_cast<float>(GET_Y_LPARAM(lParam));

				Renderer::processMouseUp(rootWidget, s_state.clientWidth, s_state.clientHeight, x, y, MouseButton::Middle);
				return false;
			}
			case WM_XBUTTONDOWN: {
				float mx = static_cast<float>(GET_X_LPARAM(lParam));
				float my = static_cast<float>(GET_Y_LPARAM(lParam));

				UINT xButton = GET_XBUTTON_WPARAM(wParam);
				MouseButton button = (xButton == XBUTTON1) ? MouseButton::Side1 : MouseButton::Side2;

				Renderer::processMouseDown(rootWidget, s_state.clientWidth, s_state.clientHeight, mx, my, button);
				return false;
			}
			case WM_XBUTTONUP: {
				float x = static_cast<float>(GET_X_LPARAM(lParam));
				float y = static_cast<float>(GET_Y_LPARAM(lParam));

				UINT xButton = GET_XBUTTON_WPARAM(wParam);
				MouseButton button = (xButton == XBUTTON1) ? MouseButton::Side1 : MouseButton::Side2;

				Renderer::processMouseUp(rootWidget, s_state.clientWidth, s_state.clientHeight, x, y, button);
				return false;
			}
			case WM_CHAR: {
				if (SWidget::getFocusedWidget()) {
					SWidget::getFocusedWidget()->onChar((char)wParam);
				}
				return false;
			}
			case WM_KEYDOWN: {
				if (SWidget::getFocusedWidget()) {
					Key mappedKey = mapWin32KeyToSilica(wParam);
					SWidget::getFocusedWidget()->onKeyDown(mappedKey);
				}
				return false;
			}
			case WM_KEYUP: {
				if (SWidget::getFocusedWidget()) {
					Key mappedKey = mapWin32KeyToSilica(wParam);
					SWidget::getFocusedWidget()->onKeyUp(mappedKey);
				}
				return 0;
			}
			case WM_MOUSEWHEEL: {
				float delta = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / WHEEL_DELTA;
				POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

				ScreenToClient(hwnd, &pt);
				Renderer::processMouseWheel(rootWidget, s_state.clientWidth, s_state.clientHeight, (float)pt.x, (float)pt.y, delta);
				return false;
			}
		}

		return false;
	}

}
