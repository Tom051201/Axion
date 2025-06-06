#pragma once

#include "Axion/Window.h"

namespace Axion {

	constexpr wchar_t* windowClassName = L"Win32WindowClass";

	class WindowsWindow : public Window {
	public:

		WindowsWindow(const WindowProperties& wp);
		virtual ~WindowsWindow();

		void onUpdate() override;

		inline uint32_t getWidth() const override { return m_data.width; }
		inline uint32_t getHeight() const override { return m_data.height; }

		inline void setEventCallback(const EventCallbackFn& callback) override { m_data.eventCallback = callback; }
		inline void setVSync(bool enabled) override { m_data.vsync = enabled; }
		inline bool isVSync() const override { return m_data.vsync; }
		inline void* getNativeHandle() const override { return m_hwnd; }

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
		void shutdown();

		static LRESULT CALLBACK staticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	};

}
