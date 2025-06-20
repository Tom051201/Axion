#pragma once

#include "Axion/core/Core.h"
#include "Axion/input/InputCodes.h"

namespace Axion {

	class Input {
	public:

		static bool isKeyPressed(KeyCode code);

		static bool isMouseButtonPressed(MouseButton button);
		static std::pair<float, float> getMousePosition();
		static float getMouseX();
		static float getMouseY();

	};

}
