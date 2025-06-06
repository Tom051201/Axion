#pragma once

#include "Axion/input/Input.h"

namespace Axion {

	class WindowsInput : public Input {
	protected:

		bool isKeyPressedImpl(KeyCode code) override;

		bool isMouseButtonPressedImpl(MouseButton button) override;
		std::pair<float, float> getMousePositionImpl() override;
		float getMouseXImpl() override;
		float getMouseYImpl() override;

	};

}
