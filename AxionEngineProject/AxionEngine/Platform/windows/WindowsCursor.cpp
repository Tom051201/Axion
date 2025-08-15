#include "axpch.h"
#include "WindowsCursor.h"

namespace Axion {

	Cursor* Cursor::create(Window* window) {
		HWND hwnd = static_cast<HWND>(window->getNativeHandle());
		return new WindowsCursor(hwnd);
	}

	WindowsCursor::WindowsCursor(HWND hwnd) {
		m_hwnd = hwnd;
		m_visible = true;
		show();
	}

	WindowsCursor::~WindowsCursor() {}

	void WindowsCursor::show() {
		while (ShowCursor(TRUE) < 0) {}
		m_visible = true;
	}

	void WindowsCursor::hide() {
		while (ShowCursor(FALSE) >= 0) {}
		m_visible = false;
	}

	bool WindowsCursor::isVisible() const {
		return m_visible;
	}

	void WindowsCursor::setPositionOnScreen(uint32_t x, uint32_t y) const {
		SetCursorPos((int)x, (int)y);
	}

	void WindowsCursor::setPositionInWindow(uint32_t x, uint32_t y) const {
		POINT pt{ (LONG)x, (LONG)y };
		ClientToScreen(m_hwnd, &pt);
		SetCursorPos(pt.x, pt.y);
	}

	void WindowsCursor::centerOnScreen() const {
		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);
		SetCursorPos(screenWidth / 2, screenHeight / 2);
	}

	void WindowsCursor::centerInWindow() const {
		RECT rect;
		GetClientRect(m_hwnd, &rect);

		POINT center{
			rect.right / 2,
			rect.bottom / 2
		};

		ClientToScreen(m_hwnd, &center);
		SetCursorPos(center.x, center.y);
	}

	void WindowsCursor::centerInRectScreen(uint32_t x, uint32_t y, uint32_t w, uint32_t h) const {
		POINT center{
			(LONG)(x + w / 2),
			(LONG)(y + h / 2)
		};

		SetCursorPos(center.x, center.y);
	}

	void WindowsCursor::centerInRectWindow(uint32_t x, uint32_t y, uint32_t w, uint32_t h) const {
		POINT center{
			(LONG)(x + w / 2),
			(LONG)(y + h / 2)
		};

		ClientToScreen(m_hwnd, &center);
		SetCursorPos(center.x, center.y);
	}

	void WindowsCursor::setCursor(CursorType type) {
		LPCWSTR id = IDC_ARROW;
		switch (type) {
			case Axion::CursorType::Arrow: { id = IDC_ARROW; break; }
			case Axion::CursorType::IBeam: { id = IDC_IBEAM; break; }
			case Axion::CursorType::Hand: { id = IDC_HAND; break; }
			case Axion::CursorType::Wait: { id = IDC_WAIT; break; }
			case Axion::CursorType::Cross: { id = IDC_CROSS; break; }
			case Axion::CursorType::UpArrow: { id = IDC_UPARROW; break; }
			case Axion::CursorType::SizeAll: { id = IDC_SIZEALL; break; }
			case Axion::CursorType::SizeNWSE: { id = IDC_SIZENWSE; break; }
			case Axion::CursorType::SizeNESW: { id = IDC_SIZENESW; break; }
			case Axion::CursorType::SizeWE: { id = IDC_SIZEWE; break; }
			case Axion::CursorType::SizeNS: { id = IDC_SIZENS; break; }
			case Axion::CursorType::Help: { id = IDC_HELP; break; }
			case Axion::CursorType::No: { id = IDC_NO; break; }
			case Axion::CursorType::AppStarting: { id = IDC_APPSTARTING; break; }
			default: { AX_CORE_ASSERT(false, "Undefined CursorType"); id = IDC_ARROW; break; }
		}

		HCURSOR hcursor = LoadCursor(NULL, id);
		SetCursor(hcursor);
	}

}
