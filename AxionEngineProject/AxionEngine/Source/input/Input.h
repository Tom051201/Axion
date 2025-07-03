#pragma once

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/input/InputCodes.h"

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
