#pragma once

#include <string>
#include <functional>

#include "AxionEngine/Source/core/Core.h"

namespace Axion {

	enum class EventType {
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		FileDrop,
		ApplicationTick, ApplicationUpdate, ApplicationRender,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled,
		RenderingPrepared, RenderingFinished,
		ProjectChanged, SceneChanged
	};

	enum EventCategory {
		None = 0,
		EventCategoryApplication	= BIT(0),
		EventCategoryInput			= BIT(1),
		EventCategoryKeyboard		= BIT(2),
		EventCategoryMouse			= BIT(3),
		EventCategoryMouseButton	= BIT(4)
	};


	#define AX_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

	#define EVENT_CLASS_TYPE(type)	static EventType getStaticType() { return EventType::##type; }\
									virtual EventType getEventType() const override { return getStaticType(); }\
									virtual const char* getName() const override { return #type; }

	#define EVENT_CLASS_CATEGORY(category) virtual int getCategoryFlags() const override { return category; }

	class EventReply {
	public:

		constexpr bool isHandled() const { return m_handled; }

		static constexpr EventReply handled() { return EventReply(true); }
		static constexpr EventReply unhandled() { return EventReply(false); }

	private:

		constexpr EventReply(bool handled) : m_handled(handled) {}
		bool m_handled = false;

	};


	class Event {
	public:

		bool handled = false;
		
		virtual EventType getEventType() const = 0;
		virtual const char* getName() const = 0;
		virtual int getCategoryFlags() const = 0;
		virtual std::string toString() const { return getName(); }

		inline bool isInCategory(EventCategory category) { return getCategoryFlags() & category; }

	private:

		friend class EventDispatcher;

	};



	class EventDispatcher {
	private:

		template<typename T> using EventFn = std::function<EventReply(T&)>;
		Event& m_event;

	public:

		EventDispatcher(Event& ev) : m_event(ev) {}

		template<typename T> bool dispatch(EventFn<T> func) {
			if (m_event.getEventType() == T::getStaticType()) {
				m_event.handled |= func(static_cast<T&>(m_event)).isHandled();
				return true;
			}
			return false;
		}

	};



	inline std::ostream& operator<<(std::ostream& os, const Event& e) { return os << e.toString(); }

}