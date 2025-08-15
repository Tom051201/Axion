#pragma once

#include "AxionEngine/Source/core/Cursor.h"

namespace Axion {

	class WindowsCursor : public Cursor {
	public:

		WindowsCursor(HWND hwnd);
		~WindowsCursor();

		void show() override;
		void hide() override;
		bool isVisible() const override;

		void setPositionOnScreen(uint32_t x, uint32_t y) const override;
		void setPositionInWindow(uint32_t x, uint32_t y) const override;

		void centerOnScreen() const override;
		void centerInWindow() const override;
		void centerInRectScreen(uint32_t x, uint32_t y, uint32_t w, uint32_t h) const override;
		void centerInRectWindow(uint32_t x, uint32_t y, uint32_t w, uint32_t h) const override;

		void setCursor(CursorType type) override;

	private:

		HWND m_hwnd;
		bool m_visible;

	};

}
