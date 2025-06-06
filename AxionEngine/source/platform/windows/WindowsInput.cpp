#include "axpch.h"
#include "WindowsInput.h"

#include "Axion/Application.h"

#include "WindowsInputMapper.h"

namespace Axion {

	Input* Input::s_instance = new WindowsInput();

	bool WindowsInput::isKeyPressedImpl(KeyCode code) {
		int winCode = WindowsInputMapper::toWin32KeyCode(code);
		return (GetAsyncKeyState(winCode) & 0x8000) != 0;
	}

	bool WindowsInput::isMouseButtonPressedImpl(MouseButton button) {
		int winButton = WindowsInputMapper::toWin32MouseButton(button);
		return (GetAsyncKeyState(winButton) & 0x8000) != 0;
	}

	float WindowsInput::getMouseXImpl() {
		auto [x, y] = getMousePositionImpl();
		return x;
	}

	float WindowsInput::getMouseYImpl() {
		auto[x, y] = getMousePositionImpl();
		return y;
	}

	std::pair<float, float> WindowsInput::getMousePositionImpl() {
		POINT cursorPos;
		if (GetCursorPos(&cursorPos)) {
			ScreenToClient(static_cast<HWND>(Application::get().getWindow().getNativeHandle()), &cursorPos);
		}
		return { cursorPos.x, cursorPos.y };
	}

}
