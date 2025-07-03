#pragma once
#include "axpch.h"

#include "AxionEngine/Source/input/InputCodes.h"

namespace Axion {

	class WindowsInputMapper {
	public:

		static KeyCode toAxionKeyCode(uint16_t win32KeyCode);
		static MouseButton toAxionMouseButton(uint16_t win32MouseButton);

		static int toWin32KeyCode(KeyCode code);
		static int toWin32MouseButton(MouseButton button);

	private:

		static const std::unordered_map<uint16_t, KeyCode> s_win32KeyCodeMap;
		static const std::unordered_map<uint16_t, MouseButton> s_win32MouseButtonMap;

		static const std::unordered_map<KeyCode, uint16_t> s_axionToWin32KeyCodeMap;
		static const std::unordered_map<MouseButton, uint16_t> s_axionToWin32MouseButtonMap;

	};


}
