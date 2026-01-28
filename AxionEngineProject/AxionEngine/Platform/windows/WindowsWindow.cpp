#include "axpch.h"
#include "WindowsWindow.h"

#include "AxionEngine/Source/events/ApplicationEvent.h"
#include "AxionEngine/Source/events/KeyEvent.h"
#include "AxionEngine/Source/events/MouseEvent.h"
#include "AxionEngine/Source/render/GraphicsContext.h"

#include "AxionEngine/Platform/windows/WindowsInputMapper.h"
#include "AxionEngine/Platform/windows/WindowsHelper.h"
#include "AxionEngine/Platform/directx/D12Context.h"

#include "AxionEngine/Vendor/imgui/backends/imgui_impl_win32.h"

#include <shellapi.h>

// ----- Used for custom title bar -----
#define WM_APP_MINIMIZE	(WM_APP + 1)
#define WM_APP_MAXIMIZE	(WM_APP + 2)
#define WM_APP_RESTORE	(WM_APP + 3)

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
		// ----- Set properties -----
		m_data.width = wp.width;
		m_data.height = wp.height;
		m_data.title = wp.title;


		// ----- Window class -----
		WNDCLASSEX wcex = {};
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = staticWndProc;
		wcex.hInstance = GetModuleHandle(nullptr);
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.lpszClassName = windowClassName;
		ATOM resultCEX = RegisterClassEx(&wcex);
		if (!resultCEX) { AX_CORE_LOG_ERROR("Failed to register window class"); throw::std::runtime_error("Failed to register window class"); }


		DWORD style;
		// ----- Activates the custom title bar -----
		#if AX_WIN_USING_CUSTOM_TITLE_BAR
			style = WS_POPUP | WS_THICKFRAME | WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
		#else
			style = WS_OVERLAPPEDWINDOW;
		#endif

		RECT wRect = { 0, 0, (LONG)m_data.width, (LONG)m_data.height };
		AdjustWindowRect(&wRect, style, FALSE);


		// ----- Create window -----
		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);
		int posX = (screenWidth / 2) - (m_data.width / 2);
		int posY = (screenHeight / 2) - (m_data.height / 2);

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

		// ----- Enable File dropping -----
		if (wp.dragAcceptFiles) {
			DragAcceptFiles(m_hwnd, TRUE);
			AX_CORE_LOG_TRACE("Drag accept files enabled");
		}

		// ----- Load icon -----
		if (!wp.iconFilePath.empty()) {
			setIcon(wp.iconFilePath);
		}

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
		RECT rect = { 0, 0, (LONG)m_data.width, (LONG)m_data.height };

		DWORD style =
			#if AX_WIN_USING_CUSTOM_TITLE_BAR
				WS_POPUP | WS_THICKFRAME | WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
			#else
				WS_OVERLAPPEDWINDOW;
			#endif

		AdjustWindowRect(&rect, style, FALSE);

		int totalWidth = rect.right - rect.left;
		int totalHeight = rect.bottom - rect.top;

		SetWindowPos(m_hwnd, nullptr, x, y, totalWidth, totalHeight, SWP_NOZORDER | SWP_NOACTIVATE);
	}

	void WindowsWindow::setTitle(const std::string& title) {
		m_data.title = title;
		SetWindowTextA(m_hwnd, title.c_str());
	}

	void WindowsWindow::setIcon(const std::string& path) const {
		HICON hIcon = (HICON)LoadImage(
			NULL,
			WindowsHelper::StringToWString(path).c_str(),
			IMAGE_ICON,
			0, 0,
			LR_LOADFROMFILE | LR_DEFAULTSIZE
		);

		if (hIcon) {
			SendMessage(m_hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
			SendMessage(m_hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
		}
		else {
			AX_CORE_LOG_ERROR("Failed to load icon for window");
		}
	}

	void WindowsWindow::minimize() const {
		// -- Minimize --
		PostMessage(m_hwnd, WM_APP_MINIMIZE, 0, 0);
	}

	void WindowsWindow::maximizeOrRestore() const {
		if (IsZoomed(m_hwnd)) {
			// -- Restore --
			PostMessage(m_hwnd, WM_APP_RESTORE, 0, 0);
		} else {
			// -- Maximize --
			PostMessage(m_hwnd, WM_APP_MAXIMIZE, 0, 0);
		}
	}

	LRESULT CALLBACK WindowsWindow::staticWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

		// ----- ImGui win32 wndProc -----
		if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
			return true;
		}
		
		WindowsWindow* window = (WindowsWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		if (window && window->m_data.eventCallback) {

			switch (msg) {
				// ----- Close -----
				case WM_CLOSE: {
					WindowCloseEvent ev;
					window->m_data.eventCallback(ev);
					PostQuitMessage(0);
					break;
				}
				// ----- Resizing -----
				case WM_SIZE: {
					UINT width = LOWORD(lparam);
					UINT height = HIWORD(lparam);

					WindowResizeEvent ev(width, height);
					window->m_data.eventCallback(ev);
					window->m_data.width = width;
					window->m_data.height = height;
					break;
				}
				// ----- Focused -----
				case WM_SETFOCUS: {
					WindowFocusEvent ev;
					window->m_data.eventCallback(ev);
					break;
				}
				// ----- Lost focus -----
				case WM_KILLFOCUS: {
					WindowLostFocusEvent ev;
					window->m_data.eventCallback(ev);
					break;
				}
				// ----- Window moved -----
				case WM_MOVE: {
					float x = (float)(short)LOWORD(lparam);
					float y = (float)(short)HIWORD(lparam);

					WindowMovedEvent ev(x, y);
					window->m_data.eventCallback(ev);
					break;
				}
				// ----- Key pressed -----
				case WM_KEYDOWN: case WM_SYSKEYDOWN: {
					int keycode = static_cast<int>(wparam);
					bool wasPreviouslyDown = (lparam & (1 << 30)) != 0;
					bool isRepeat = wasPreviouslyDown;
					KeyPressedEvent ev(WindowsInputMapper::toAxionKeyCode(keycode), isRepeat ? 1 : 0);
					window->m_data.eventCallback(ev);
					break;
				}
				// ----- Key released -----
				case WM_KEYUP: case WM_SYSKEYUP: {
					int keycode = static_cast<int>(wparam);
					KeyReleasedEvent ev(WindowsInputMapper::toAxionKeyCode(keycode));
					window->m_data.eventCallback(ev);
					break;
				}
				// ----- Key typed -----
				case WM_CHAR: {
					uint32_t character = static_cast<uint32_t>(wparam);
					KeyTypedEvent ev(character);
					window->m_data.eventCallback(ev);
					break;
				}
				// ----- Mouse moved -----
				case WM_MOUSEMOVE: {
					float x = static_cast<float>(GET_X_LPARAM(lparam));
					float y = static_cast<float>(GET_Y_LPARAM(lparam));
					MouseMovedEvent ev(x, y);
					window->m_data.eventCallback(ev);
					break;
				}
				// ----- Left mouse button pressed -----
				case WM_LBUTTONDOWN: {
					MouseButtonPressedEvent ev(WindowsInputMapper::toAxionMouseButton(VK_LBUTTON));
					window->m_data.eventCallback(ev);
					break;
				}
				// ----- Left mouse button released -----
				case WM_LBUTTONUP: {
					MouseButtonReleasedEvent ev(WindowsInputMapper::toAxionMouseButton(VK_LBUTTON));
					window->m_data.eventCallback(ev);
					break;
				}
				// ----- Right mouse button pressed -----
				case WM_RBUTTONDOWN: {
					MouseButtonPressedEvent ev(WindowsInputMapper::toAxionMouseButton(VK_RBUTTON));
					window->m_data.eventCallback(ev);
					break;
				}
				// ----- Right mouse button released -----
				case WM_RBUTTONUP: {
					MouseButtonReleasedEvent ev(WindowsInputMapper::toAxionMouseButton(VK_RBUTTON));
					window->m_data.eventCallback(ev);
					break;
				}
				// ----- Mouse wheel scrolled -----
				case WM_MOUSEWHEEL: {
					float dt = GET_WHEEL_DELTA_WPARAM(wparam) / static_cast<float>(WHEEL_DELTA);
					MouseScrolledEvent ev(0.0f, dt);
					window->m_data.eventCallback(ev);
					break;
				}
				// ----- Deactivate title bar -----
				case WM_NCCALCSIZE: {
					#if AX_WIN_USING_CUSTOM_TITLE_BAR
					if (wparam == TRUE) {
						return 0;
					}
					#endif
					break;
				}
				// ----- Hit test for custom title bar -----
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

					if (window->isDragZone && window->isDragZone(pt.x, pt.y)) return HTCAPTION;
					
					if (left) return HTLEFT;
					if (right) return HTRIGHT;
					if (top) return HTTOP;
					if (bottom) return HTBOTTOM;

					return HTCLIENT;

					#else
					break;
					#endif
				}
				// ----- Calculating size for minimizing / maximizing -----
				case WM_GETMINMAXINFO: {
					#if AX_WIN_USING_CUSTOM_TITLE_BAR
					LPMINMAXINFO mmi = (LPMINMAXINFO)lparam;

					HMONITOR hmonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
					MONITORINFO mi = {};
					mi.cbSize = sizeof(MONITORINFO);
					GetMonitorInfo(hmonitor, &mi);

					RECT rcWork = mi.rcWork; // work area excluding taskbar
					RECT rcMonitor = mi.rcMonitor;

					const int border = GetSystemMetrics(SM_CXSIZEFRAME); // horizontal border
					const int caption = GetSystemMetrics(SM_CYCAPTION); // title bar height

					mmi->ptMaxSize.x = rcWork.right - rcWork.left;
					mmi->ptMaxSize.y = rcWork.bottom - rcWork.top;
					mmi->ptMaxPosition.x = rcWork.left - rcMonitor.left;
					mmi->ptMaxPosition.y = rcWork.top - rcMonitor.top;

					return 0;
					#endif
					break;
				}
				// ----- Custom event for custom minimizing -----
				case WM_APP_MINIMIZE: {
					ShowWindow(hwnd, SW_MINIMIZE);
					break;
				}
				// ----- Custom event for custom maximizing -----
				case WM_APP_MAXIMIZE: {
					ShowWindow(hwnd, SW_MAXIMIZE);
					break;
				}
				// ----- Custom event for custom restoring -----
				case WM_APP_RESTORE: {
					ShowWindow(hwnd, SW_RESTORE);
					break;
				}
				// ----- Accept file drops -----
				case WM_DROPFILES: {
					HDROP hDrop = (HDROP)wparam;

					UINT fileCount = DragQueryFileW(hDrop, 0xFFFFFFFF, nullptr, 0);
					std::vector<std::filesystem::path> files;

					for (UINT i = 0; i < fileCount; i++) {
						wchar_t path[MAX_PATH];
						DragQueryFileW(hDrop, i, path, MAX_PATH);
						files.emplace_back(path);
					}

					DragFinish(hDrop);

					window->m_data.eventCallback(FileDropEvent(std::move(files)));

					break;
				}
				default: { break; }
			}

		}

		return DefWindowProc(hwnd, msg, wparam, lparam);
	}

}
