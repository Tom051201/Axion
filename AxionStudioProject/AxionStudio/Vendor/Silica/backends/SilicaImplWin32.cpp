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
				case Cursor::TextInput: { s_currentCursor = LoadCursor(NULL, IDC_IBEAM); break; }
				case Cursor::ResizeEW: { s_currentCursor = LoadCursor(NULL, IDC_SIZEWE); break; }
				case Cursor::ResizeNS: { s_currentCursor = LoadCursor(NULL, IDC_SIZENS); break; }
			}
			::SetCursor(s_currentCursor);
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
				return true;
			}
			case WM_LBUTTONDOWN: {
				float mx = static_cast<float>(GET_X_LPARAM(lParam));
				float my = static_cast<float>(GET_Y_LPARAM(lParam));

				Renderer::processMouseDown(rootWidget, s_state.clientWidth, s_state.clientHeight, mx, my, MouseButton::Left);
				return true;
			}
			case WM_LBUTTONUP: {
				float x = static_cast<float>(GET_X_LPARAM(lParam));
				float y = static_cast<float>(GET_Y_LPARAM(lParam));

				Renderer::processMouseUp(rootWidget, s_state.clientWidth, s_state.clientHeight, x, y, MouseButton::Left);
				return true;
			}
			case WM_RBUTTONDOWN: {
				float mx = static_cast<float>(GET_X_LPARAM(lParam));
				float my = static_cast<float>(GET_Y_LPARAM(lParam));
				Renderer::processMouseDown(rootWidget, s_state.clientWidth, s_state.clientHeight, mx, my, MouseButton::Right);
				return true;
			}
			case WM_RBUTTONUP: {
				float x = static_cast<float>(GET_X_LPARAM(lParam));
				float y = static_cast<float>(GET_Y_LPARAM(lParam));

				Renderer::processMouseUp(rootWidget, s_state.clientWidth, s_state.clientHeight, x, y, MouseButton::Right);
				return true;
			}
			case WM_MBUTTONDOWN: {
				float mx = static_cast<float>(GET_X_LPARAM(lParam));
				float my = static_cast<float>(GET_Y_LPARAM(lParam));
				Renderer::processMouseDown(rootWidget, s_state.clientWidth, s_state.clientHeight, mx, my, MouseButton::Middle);
				return true;
			}
			case WM_MBUTTONUP: {
				float x = static_cast<float>(GET_X_LPARAM(lParam));
				float y = static_cast<float>(GET_Y_LPARAM(lParam));

				Renderer::processMouseUp(rootWidget, s_state.clientWidth, s_state.clientHeight, x, y, MouseButton::Middle);
				return true;
			}
			case WM_CHAR: {
				if (SWidget::getFocusedWidget()) {
					SWidget::getFocusedWidget()->onChar((char)wParam);
				}
				return 0;
			}
			case WM_KEYDOWN: {
				if (SWidget::getFocusedWidget()) {
					Key mappedKey = mapWin32KeyToSilica(wParam);
					SWidget::getFocusedWidget()->onKeyDown(mappedKey);
				}
				return 0;
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
				return 0;
			}
		}

		return false;
	}

}
