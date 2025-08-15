#pragma once
#include "axpch.h"

#include "AxionEngine/Source/core/Window.h"

namespace Axion {

	enum class CursorType {
		Arrow,
		IBeam,
		Hand,
		Wait,
		Cross,
		UpArrow,
		SizeAll,
		SizeNWSE,
		SizeNESW,
		SizeWE,
		SizeNS,
		Help,
		No,
		AppStarting
	};

	class Cursor {
	public:

		virtual ~Cursor() = default;

		virtual void show() = 0;
		virtual void hide() = 0;
		virtual bool isVisible() const = 0;

		virtual void setPositionOnScreen(uint32_t x, uint32_t y) const = 0;
		virtual void setPositionInWindow(uint32_t x, uint32_t y) const = 0;

		virtual void centerOnScreen() const = 0;
		virtual void centerInWindow() const = 0;
		virtual void centerInRectScreen(uint32_t x, uint32_t y, uint32_t w, uint32_t h) const = 0;
		virtual void centerInRectWindow(uint32_t x, uint32_t y, uint32_t w, uint32_t h) const = 0;

		virtual void setCursor(CursorType type) = 0;


		static Cursor* create(Window* window);
	};

}
