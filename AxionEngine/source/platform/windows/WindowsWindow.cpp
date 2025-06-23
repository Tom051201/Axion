#include "axpch.h"
#include "WindowsWindow.h"

#include "Axion/events/ApplicationEvent.h"
#include "Axion/events/KeyEvent.h"
#include "Axion/events/MouseEvent.h"
#include "Axion/render/GraphicsContext.h"

#include "platform/windows/WindowsInputMapper.h"
#include "platform/directx/D12Context.h"

#include "imgui/backends/imgui_impl_win32.h"


extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Axion {

	Window* Window::create(const WindowProperties& wp) { return new WindowsWindow(wp); }

	WindowsWindow::WindowsWindow(const WindowProperties& wp) {
		initialize(wp);
	}

	WindowsWindow::~WindowsWindow() {
		shutdown();
	}

	void WindowsWindow::initialize(const WindowProperties& wp) {
		m_data.width = wp.width;
		m_data.height = wp.height;
		m_data.title = wp.title;

		WNDCLASSEX wcex = {};
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = staticWndProc;
		wcex.hInstance = GetModuleHandle(nullptr);
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.lpszClassName = windowClassName;
		ATOM resultCEX = RegisterClassEx(&wcex);
		if (!resultCEX) { AX_CORE_LOG_ERROR("Failed to register window class"); throw::std::runtime_error("Failed to register window class"); }

		RECT wRect = { 0, 0, (LONG)m_data.width, (LONG)m_data.height };
		DWORD style;
		// activates the custom title bar for windows
		#if AX_WIN_USING_CUSTOM_TITLE_BAR
			style = WS_POPUP | WS_THICKFRAME | WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
		#else
			style = WS_OVERLAPPEDWINDOW;
		#endif
		AdjustWindowRect(&wRect, style, FALSE);

		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);
		int posX = (screenWidth / 2)  - (m_data.width / 2);
		int posY = (screenHeight / 2)  - (m_data.height / 2);

		m_hwnd = CreateWindowEx(
			0, windowClassName,
			std::wstring(m_data.title.begin(), m_data.title.end()).c_str(),
			style,
			posX, posY,
			wRect.right - wRect.left, wRect.bottom - wRect.top,
			nullptr, nullptr, wcex.hInstance, this
		);
		if (!m_hwnd) { AX_CORE_LOG_ERROR("Failed to create window"); throw::std::runtime_error("Failed to create window"); }

		SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)this);

		ShowWindow(m_hwnd, SW_SHOW);
		AX_CORE_LOG_INFO("Created window '{0}' ({1}, {2})", wp.title, wp.width, wp.height);
	}

	void WindowsWindow::shutdown() {
		GraphicsContext::get()->shutdown();
		AX_CORE_LOG_INFO("Window shutdown");
		
	}

	void WindowsWindow::onUpdate() {
		MSG msg = {};
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	void WindowsWindow::setPosition(uint32_t x, uint32_t y) {
		// TODO: add function
	}

	LRESULT CALLBACK WindowsWindow::staticWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
		
		if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
			return true;
		}
		
		WindowsWindow* window = (WindowsWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		if (window && window->m_data.eventCallback) {

			switch (msg) {
				case WM_CLOSE: {
					WindowCloseEvent ev;
					window->m_data.eventCallback(ev);
					PostQuitMessage(0);
					break;
				}
				case WM_SIZE: {
					UINT width = LOWORD(lparam);
					UINT height = HIWORD(lparam);

					WindowResizeEvent ev(width, height);
					window->m_data.eventCallback(ev);
					break;
				}
				case WM_SETFOCUS: {
					WindowFocusEvent ev;
					window->m_data.eventCallback(ev);
					break;
				}
				case WM_KILLFOCUS: {
					WindowLostFocusEvent ev;
					window->m_data.eventCallback(ev);
					break;
				}
				case WM_MOVE: {
					float x = (float)(short)LOWORD(lparam);
					float y = (float)(short)HIWORD(lparam);

					WindowMovedEvent ev(x, y);
					window->m_data.eventCallback(ev);
					break;
				}
				case WM_KEYDOWN: case WM_SYSKEYDOWN: {
					int keycode = static_cast<int>(wparam);
					int repeatCount = (lparam & 0xFFFF);
					KeyPressedEvent ev(WindowsInputMapper::toAxionKeyCode(keycode), repeatCount);
					window->m_data.eventCallback(ev);
					break;
				}
				case WM_KEYUP: case WM_SYSKEYUP: {
					int keycode = static_cast<int>(wparam);
					KeyReleasedEvent ev(WindowsInputMapper::toAxionKeyCode(keycode));
					window->m_data.eventCallback(ev);
					break;
				}
				case WM_CHAR: {
					uint32_t character = static_cast<uint32_t>(wparam);
					KeyTypedEvent ev(character);
					window->m_data.eventCallback(ev);
					break;
				}
				case WM_MOUSEMOVE: {
					float x = static_cast<float>(GET_X_LPARAM(lparam));
					float y = static_cast<float>(GET_Y_LPARAM(lparam));
					MouseMovedEvent ev(x, y);
					window->m_data.eventCallback(ev);
					break;
				}
				case WM_LBUTTONDOWN: {
					MouseButtonPressedEvent ev(VK_LBUTTON);
					window->m_data.eventCallback(ev);
					break;
				}
				case WM_LBUTTONUP: {
					MouseButtonReleasedEvent ev(VK_LBUTTON);
					window->m_data.eventCallback(ev);
					break;
				}
				case WM_RBUTTONDOWN: {
					MouseButtonPressedEvent ev(VK_RBUTTON);
					window->m_data.eventCallback(ev);
					break;
				}
				case WM_RBUTTONUP: {
					MouseButtonReleasedEvent ev(VK_RBUTTON);
					window->m_data.eventCallback(ev);
					break;
				}
				case WM_MOUSEWHEEL: {
					float dt = GET_WHEEL_DELTA_WPARAM(wparam) / static_cast<float>(WHEEL_DELTA);
					MouseScrolledEvent ev(0.0f, dt);
					window->m_data.eventCallback(ev);
					break;
				}
				case WM_NCCALCSIZE: {
					#if AX_WIN_USING_CUSTOM_TITLE_BAR
					if (wparam == TRUE) {
						return 0;
					}
					#endif
					break;
				}
				case WM_NCHITTEST: {
					#if AX_WIN_USING_CUSTOM_TITLE_BAR
					POINT pt = { GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
					ScreenToClient(hwnd, &pt);

					RECT rect;
					GetClientRect(hwnd, &rect);

					const int border = 6;

					bool left = pt.x < border;
					bool right = pt.x >= rect.right - border;
					bool top = pt.y < border;
					bool bottom = pt.y >= rect.bottom - border;

					if (top && left) return HTTOPLEFT;
					if (top && right) return HTTOPRIGHT;
					if (bottom && left) return HTBOTTOMLEFT;
					if (bottom && right) return HTBOTTOMRIGHT;
					if (left) return HTLEFT;
					if (right) return HTRIGHT;
					if (top) return HTTOP;
					if (bottom) return HTBOTTOM;

					return HTCLIENT;
					#else
					break;
					#endif
				}
				default: { break; }
			}

		}

		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
}
