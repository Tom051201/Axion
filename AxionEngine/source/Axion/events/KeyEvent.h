#pragma once

#include "axpch.h"

#include "Event.h"

#include "Axion/input/InputCodes.h"

namespace Axion {

	class KeyEvent : public Event {
	public:

		inline KeyCode getKeyCode() const { return m_keyCode; }

		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

	protected:

		KeyEvent(KeyCode keycode) : m_keyCode(keycode) {}

		KeyCode m_keyCode;

	};

	

	class KeyPressedEvent : public KeyEvent {
	public:

		KeyPressedEvent(KeyCode keycode, int repeatCount) : KeyEvent(keycode), m_repeatCount(repeatCount) {}

		inline int getRepeatCount() const { return m_repeatCount; }

		std::string toString() const override {
			std::stringstream ss;
			//ss << "KeyPressedEvent: " << m_keyCode << " (" << m_repeatCount << " repeats)";
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyPressed)

	private:

		int m_repeatCount;

	};



	class KeyReleasedEvent : public KeyEvent {
	public:

		KeyReleasedEvent(KeyCode keycode) : KeyEvent(keycode) {}

		std::string toString() const override {
			std::stringstream ss;
			//ss << "KeyReleasedEvent: " << m_keyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyReleased)

	};



	class KeyTypedEvent : public KeyEvent {
	public:

		KeyTypedEvent(uint32_t character) : KeyEvent(KeyCode::Unknown), m_char(character) {}

		inline uint32_t getChar() const { return m_char; }

		std::string toString() const override {
			std::stringstream ss;
			//ss << "KeyPressedEvent: " << m_keyCode << " (" << m_repeatCount << " repeats)";
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyTyped)

	private:

		uint32_t m_char;

	};

}
