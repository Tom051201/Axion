#pragma once

#include "AxionEngine/Source/events/Event.h"

namespace Axion {

	class MouseMovedEvent : public Event {
	public:

		MouseMovedEvent(float x, float y) : m_mouseX(x), m_mouseY(y) {}

		inline float getX() const { return m_mouseX; }
		inline float getY() const { return m_mouseY; }

		std::string toString() const override {
			std::stringstream ss;
			ss << "MouseMovedEvent: " << m_mouseX << ", " << m_mouseY;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:

		float m_mouseX, m_mouseY;

	};



	class MouseScrolledEvent : public Event {
	public:

		MouseScrolledEvent(float xOff, float yOff) : m_xOffset(xOff), m_yOffset(yOff) {}

		inline float getXOffset() const { return m_xOffset; }
		inline float getYOffset() const { return m_yOffset; }

		std::string toString() const override {
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << m_xOffset << ", " << m_yOffset;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseScrolled)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:

		float m_xOffset, m_yOffset;

	};



	class MouseButtonEvent : public Event {
	public:

		inline int getMouseButton() const { return m_button; }
		
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	protected:

		MouseButtonEvent(int button) : m_button(button) {}

		int m_button;

	};



	class MouseButtonPressedEvent : public MouseButtonEvent {
	public:

		MouseButtonPressedEvent(int button) : MouseButtonEvent(button) {}

		std::string toString() const override {
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << m_button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)

	};
	
	
	
	class MouseButtonReleasedEvent : public MouseButtonEvent {
	public:

		MouseButtonReleasedEvent(int button) : MouseButtonEvent(button) {}

		std::string toString() const override {
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent: " << m_button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)

	};

}
