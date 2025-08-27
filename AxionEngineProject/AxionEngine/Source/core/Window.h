#pragma once
#include "axpch.h"

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/events/Event.h"

namespace Axion {

	class WindowProperties {
	public:

		std::string title;
		uint32_t width;
		uint32_t height;

		WindowProperties(const std::string& title = "Axion Engine", uint32_t width = 1280, uint32_t height = 720)
			: title(title), width(width), height(height) {}

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
		virtual void setIcon(const std::string& path) const = 0;

		virtual void minimize() const = 0;
		virtual void maximizeOrRestore() const = 0;

		virtual void setEventCallback(const EventCallbackFn& callback) = 0;
		virtual void setVSync(bool enabled) = 0;
		virtual bool isVSync() const = 0;

		virtual void* getNativeHandle() const = 0;


		static Window* create(const WindowProperties& wp = WindowProperties());
	};

}
