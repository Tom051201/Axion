#pragma once

#include "Axion/core/Core.h"
#include "Axion/input/InputCodes.h"

namespace Axion {

	class Input {
	public:

		inline static bool isKeyPressed(KeyCode code) { return s_instance->isKeyPressedImpl(code); }

		inline static bool isMouseButtonPressed(MouseButton button) { return s_instance->isMouseButtonPressedImpl(button); }
		inline static std::pair<float, float> getMousePosition() { return s_instance->getMousePositionImpl(); }
		inline static float getMouseX() { return s_instance->getMouseXImpl(); }
		inline static float getMouseY() { return s_instance->getMouseYImpl(); }

	protected:

		virtual bool isKeyPressedImpl(KeyCode code) = 0;

		virtual bool isMouseButtonPressedImpl(MouseButton button) = 0;
		virtual std::pair<float, float> getMousePositionImpl() = 0;
		virtual float getMouseXImpl() = 0;
		virtual float getMouseYImpl() = 0;

	private:

		static Input* s_instance;

	};

}
