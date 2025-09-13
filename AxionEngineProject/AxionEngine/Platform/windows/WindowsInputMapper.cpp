#include "axpch.h"
#include "WindowsInputMapper.h"

namespace Axion {

	// ----- Keys: Win32 -> Axion -----
	KeyCode WindowsInputMapper::toAxionKeyCode(uint16_t win32KeyCode) {
		auto it = s_win32KeyCodeMap.find(win32KeyCode);
		if (it != s_win32KeyCodeMap.end()) {
			return it->second;
		}
		return KeyCode::Unknown;
	}


	// ----- Mouse: Win32 -> Axion -----
	MouseButton WindowsInputMapper::toAxionMouseButton(uint16_t win32MouseButton) {
		auto it = s_win32MouseButtonMap.find(win32MouseButton);
		if (it != s_win32MouseButtonMap.end()) {
			return it->second;
		}
		return MouseButton::Unknown;
	}


	// ----- Keys: Axion -> Win32 -----
	int WindowsInputMapper::toWin32KeyCode(KeyCode code) {
		auto it = s_axionToWin32KeyCodeMap.find(code);
		if (it != s_axionToWin32KeyCodeMap.end()) {
			return static_cast<int>(it->second);
		}
		return -1;
	}


	// ----- Mouse: Axion -> Win32 -----
	int WindowsInputMapper::toWin32MouseButton(MouseButton button) {
		auto it = s_axionToWin32MouseButtonMap.find(button);
		if (it != s_axionToWin32MouseButtonMap.end()) {
			return static_cast<int>(it->second);
		}
		return -1;
	}


	// ----- Keys: Win32 to Axion mapping -----
	const std::unordered_map<uint16_t, KeyCode> WindowsInputMapper::s_win32KeyCodeMap = {
		// Letter keys
		{ 'A', KeyCode::A },		 { 'B', KeyCode::B },		 { 'C', KeyCode::C },
		{ 'D', KeyCode::D },		 { 'E', KeyCode::E },		 { 'F', KeyCode::F },
		{ 'G', KeyCode::G },		 { 'H', KeyCode::H },		 { 'I', KeyCode::I },
		{ 'J', KeyCode::J },		 { 'K', KeyCode::K },		 { 'L', KeyCode::L },
		{ 'M', KeyCode::M },		 { 'N', KeyCode::N },		 { 'O', KeyCode::O },
		{ 'P', KeyCode::P },		 { 'Q', KeyCode::Q },		 { 'R', KeyCode::R },
		{ 'S', KeyCode::S },		 { 'T', KeyCode::T },		 { 'U', KeyCode::U },
		{ 'V', KeyCode::V },		 { 'W', KeyCode::W },		 { 'X', KeyCode::X },
		{ 'Y', KeyCode::Y },		 { 'Z', KeyCode::Z },		



		// Number keys
		{ '0', KeyCode::Zero },		{ '1', KeyCode::One },		{ '2', KeyCode::Two },
		{ '3', KeyCode::Three },	{ '4', KeyCode::Four },		{ '5', KeyCode::Five },
		{ '6', KeyCode::Six },		{ '7', KeyCode::Seven },	{ '8', KeyCode::Eight },
		{ '9', KeyCode::Nine },



		// Symbol keys
		{ VK_OEM_1, KeyCode::Semicolon },			{ VK_OEM_PLUS, KeyCode::Equal },			{ VK_OEM_COMMA, KeyCode::Comma },
		{ VK_OEM_MINUS, KeyCode::Minus },			{ VK_OEM_PERIOD, KeyCode::Period },			{ VK_OEM_2, KeyCode::Slash },
		{ VK_OEM_3, KeyCode::GraveAccent },			{ VK_OEM_4, KeyCode::LeftBracket },			{ VK_OEM_5, KeyCode::Backslash },
		{ VK_OEM_6, KeyCode::RightBracket },		{ VK_OEM_7, KeyCode::Apostrophe  },



		// Function Keys
		{ VK_F1, KeyCode::F1 },						{ VK_F2, KeyCode::F2 },						{ VK_F3, KeyCode::F3 },
		{ VK_F4, KeyCode::F4 },						{ VK_F5, KeyCode::F5 },						{ VK_F6, KeyCode::F6 },
		{ VK_F7, KeyCode::F7 },						{ VK_F8, KeyCode::F8 },						{ VK_F9, KeyCode::F9 },
		{ VK_F10, KeyCode::F10 },					{ VK_F11, KeyCode::F11 },					{ VK_F12, KeyCode::F12 },



		// Navigation keys
		{ VK_LEFT, KeyCode::Left },					{ VK_RIGHT, KeyCode::Right },
		{ VK_UP, KeyCode::Up },						{ VK_DOWN, KeyCode::Down },
		{ VK_PRIOR, KeyCode::PageUp },				{ VK_NEXT, KeyCode::PageDown },
		{ VK_HOME, KeyCode::Home },					{ VK_END, KeyCode::End },
		{ VK_INSERT, KeyCode::Insert },				{ VK_DELETE, KeyCode::Delete },



		// Modifiers
		{ VK_LSHIFT, KeyCode::LeftShift },			{ VK_RSHIFT, KeyCode::RightShift },
		{ VK_LCONTROL, KeyCode::LeftControl },		{ VK_RCONTROL, KeyCode::RightControl },
		{ VK_LMENU, KeyCode::LeftAlt },				{ VK_RMENU, KeyCode::RightAlt },
		{ VK_CAPITAL, KeyCode::CapsLock },			{ VK_NUMLOCK, KeyCode::NumLock },
		{ VK_SCROLL, KeyCode::ScrollLock },			{ VK_SNAPSHOT, KeyCode::PrintScreen },
		{ VK_PAUSE, KeyCode::Pause },



		// Special keys
		{ VK_SPACE, KeyCode::Space },				{ VK_RETURN, KeyCode::Enter },				{ VK_ESCAPE, KeyCode::Escape },
		{ VK_TAB, KeyCode::Tab },					{ VK_BACK, KeyCode::Backspace },



		// Numpad keys
		{ VK_NUMPAD0, KeyCode::Numpad0 },			{ VK_NUMPAD1, KeyCode::Numpad1 },			{ VK_NUMPAD2, KeyCode::Numpad2 },
		{ VK_NUMPAD3, KeyCode::Numpad3 },			{ VK_NUMPAD4, KeyCode::Numpad4 },			{ VK_NUMPAD5, KeyCode::Numpad5 },
		{ VK_NUMPAD6, KeyCode::Numpad6 },			{ VK_NUMPAD7, KeyCode::Numpad7 },			{ VK_NUMPAD8, KeyCode::Numpad8 },
		{ VK_NUMPAD9, KeyCode::Numpad9 },
		{ VK_DECIMAL, KeyCode::NumpadDecimal },		{ VK_DIVIDE, KeyCode::NumpadDivide },		{ VK_MULTIPLY, KeyCode::NumpadMultiply },
		{ VK_SUBTRACT, KeyCode::NumpadSubtract },	{ VK_ADD, KeyCode::NumpadAdd }

	};


	// ----- Mouse: Win32 to Axion mapping -----
	const std::unordered_map<uint16_t, MouseButton> WindowsInputMapper::s_win32MouseButtonMap = {

		{ MK_LBUTTON, MouseButton::Left },			{ VK_RBUTTON, MouseButton::Right },			{ VK_MBUTTON, MouseButton::Middle }, 
		{ VK_XBUTTON1, MouseButton::X1 },			{ VK_XBUTTON2, MouseButton::X2 }

	};


	// ----- Keys: Axion to Win32 mapping -----
	const std::unordered_map<KeyCode, uint16_t> WindowsInputMapper::s_axionToWin32KeyCodeMap = []() {
		std::unordered_map<KeyCode, uint16_t> map;
		for (const auto& [win32Code, axionCode] : WindowsInputMapper::s_win32KeyCodeMap) {
			map[axionCode] = win32Code;
		}
		return map;
	}();


	// ----- Mouse: Axion to Win32 mapping -----
	const std::unordered_map<MouseButton, uint16_t> WindowsInputMapper::s_axionToWin32MouseButtonMap = []() {
		std::unordered_map<MouseButton, uint16_t> map;
		for (const auto& [win32Code, axionCode] : WindowsInputMapper::s_win32MouseButtonMap) {
			map[axionCode] = win32Code;
		}
		return map;
	}();

}
