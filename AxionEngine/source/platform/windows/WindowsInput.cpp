#include "axpch.h"

#include "Axion/core/Application.h"
#include "Axion/input/Input.h"

#include "platform/windows/WindowsInputMapper.h"

namespace Axion {

	bool Input::isKeyPressed(KeyCode code) {
		int winCode = WindowsInputMapper::toWin32KeyCode(code);
		return (GetAsyncKeyState(winCode) & 0x8000) != 0;
	}

	bool Input::isMouseButtonPressed(MouseButton button) {
		int winButton = WindowsInputMapper::toWin32MouseButton(button);
		return (GetAsyncKeyState(winButton) & 0x8000) != 0;
	}

	float Input::getMouseX() {
		auto [x, y] = getMousePosition();
		return x;
	}

	float Input::getMouseY() {
		auto[x, y] = getMousePosition();
		return y;
	}

	std::pair<float, float> Input::getMousePosition() {
		POINT cursorPos;
		if (GetCursorPos(&cursorPos)) {
			ScreenToClient(static_cast<HWND>(Application::get().getWindow().getNativeHandle()), &cursorPos);
		}
		return { cursorPos.x, cursorPos.y };
	}

}
