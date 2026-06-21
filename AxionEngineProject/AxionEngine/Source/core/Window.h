#pragma once
#include "axpch.h"

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/events/Event.h"

namespace Axion {

	#ifdef AX_PLATFORM_WINDOWS
	using Win32_WndProcCallback = std::function<bool(void* hwnd, unsigned int msg, unsigned long long wparam, long long lparam)>;
	#endif

	class WindowProperties {
	public:

		std::string title;
		uint32_t width;
		uint32_t height;
		bool dragAcceptFiles;
		std::filesystem::path iconFilePath;

		// -- Windows WndProc Callback for Client --
		#ifdef AX_PLATFORM_WINDOWS
		Win32_WndProcCallback win32_wndProcCallback = nullptr;
		#endif

		WindowProperties(const std::string& title = "Axion Engine", uint32_t width = 1280, uint32_t height = 720, bool dragAcceptFiles = false, const std::string& iconFilePath = "")
			: title(title), width(width), height(height), dragAcceptFiles(dragAcceptFiles), iconFilePath(iconFilePath) {}

	};

	class Window {
	public:

		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() {}

		virtual void shutdown() = 0;

		virtual void onUpdate() = 0;

		virtual void setPosition(uint32_t x, uint32_t y) = 0;
		virtual void setTitle(const std::string& title) = 0;
		virtual uint32_t getWidth() const = 0;
		virtual uint32_t getHeight() const = 0;
		virtual void setIcon(const std::filesystem::path& path) const = 0;

		virtual void minimize() const = 0;
		virtual void maximizeOrRestore() const = 0;

		virtual void setEventCallback(const EventCallbackFn& callback) = 0;
		virtual void setVSync(bool enabled) = 0;
		virtual bool isVSync() const = 0;

		virtual void* getNativeHandle() const = 0;

		#ifdef AX_PLATFORM_WINDOWS
		virtual void setWndProcCallback(const Win32_WndProcCallback& callback) = 0;
		#endif


		static Window* create(const WindowProperties& wp = WindowProperties());
	};

}
