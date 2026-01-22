#pragma once

#include "AxionEngine/Source/events/Event.h"

#include <filesystem>

namespace Axion {

	class WindowResizeEvent : public Event {
	public:

		WindowResizeEvent(uint32_t width, uint32_t height) : m_width(width), m_height(height) {}

		inline uint32_t getWidth() const { return m_width; }
		inline uint32_t getHeight() const { return m_height; }

		std::string toString() const override {
			std::stringstream ss;
			ss << "WindowResizeEvent: " << m_width << ", " << m_height;
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowResize)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	private:

		uint32_t m_width, m_height;

	};



	class WindowCloseEvent : public Event {
	public:

		WindowCloseEvent() {}

		EVENT_CLASS_TYPE(WindowClose)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	};



	class WindowFocusEvent : public Event {
	public:

		WindowFocusEvent() {}

		EVENT_CLASS_TYPE(WindowFocus)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	};



	class WindowLostFocusEvent : public Event {
	public:

		WindowLostFocusEvent() {}

		EVENT_CLASS_TYPE(WindowLostFocus)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	};



	class WindowMovedEvent : public Event {
	public:

		WindowMovedEvent(float x, float y) : m_winX(x), m_winY(y) {}

		inline float getWidth() const { return m_winX; }
		inline float getHeight() const { return m_winY; }

		std::string toString() const override {
			std::stringstream ss;
			ss << "WindowMovedEvent: " << m_winX << ", " << m_winY;
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowMoved)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	private:

		float m_winX, m_winY;

	};



	class FileDropEvent : public Event {
	public:

		explicit FileDropEvent(std::vector<std::filesystem::path>&& paths)
			: m_paths(std::move(paths)) {}

		inline const std::vector<std::filesystem::path>& getPaths() const { return m_paths; }

		EVENT_CLASS_TYPE(FileDrop)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	private:

		std::vector<std::filesystem::path> m_paths;

	};



	class AppTickEvent : public Event {
	public:

		AppTickEvent() {}

		EVENT_CLASS_TYPE(ApplicationTick)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	};



	class AppUpdateEvent : public Event {
	public:

		AppUpdateEvent() {}

		EVENT_CLASS_TYPE(ApplicationUpdate)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	};



	class  AppRenderEvent : public Event {
	public:

		AppRenderEvent() {}

		EVENT_CLASS_TYPE(ApplicationTick)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	};



	class ProjectChangedEvent : public Event {
	public:

		ProjectChangedEvent() {}

		EVENT_CLASS_TYPE(ProjectChanged)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	};



	class SceneChangedEvent : public Event {
	public:

		SceneChangedEvent() {}

		EVENT_CLASS_TYPE(SceneChanged);
		EVENT_CLASS_CATEGORY(EventCategoryApplication);

	};

}

