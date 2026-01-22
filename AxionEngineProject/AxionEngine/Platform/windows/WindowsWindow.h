#pragma once

#include "AxionEngine/Source/core/Window.h"

namespace Axion {

	constexpr wchar_t* windowClassName = L"Win32WindowClass";

	class WindowsWindow : public Window {
	public:

		WindowsWindow(const WindowProperties& wp);
		virtual ~WindowsWindow();

		void shutdown() override;

		void onUpdate() override;

		void setPosition(uint32_t x, uint32_t y) override;
		void setTitle(const std::string& title) override;
		uint32_t getWidth() const override { return m_data.width; }
		uint32_t getHeight() const override { return m_data.height; }
		void setIcon(const std::string& path) const override;

		void minimize() const override;
		void maximizeOrRestore() const override;

		void setEventCallback(const EventCallbackFn& callback) override { m_data.eventCallback = callback; }
		void setVSync(bool enabled) override { m_data.vsync = enabled; }
		bool isVSync() const override { return m_data.vsync; }

		void* getNativeHandle() const override { return m_hwnd; }

		std::function<bool(int x, int y)> isDragZone = nullptr;

	private:

		struct WindowData {
			std::string title;
			uint32_t width, height;
			bool vsync;
			EventCallbackFn eventCallback;
		};

		WindowData m_data;
		HWND m_hwnd = nullptr;

		void initialize(const WindowProperties& wp);

		static LRESULT CALLBACK staticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	};

}
