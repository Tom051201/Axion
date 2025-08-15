#include "axpch.h"
#include "ImGuiInputMapper.h"

namespace Axion {


	ImGuiKey ImGuiInputMapper::toImGuiKeyCode(KeyCode code) {
		auto it = s_imGuiKeyCodeMap.find(code);
		if (it != s_imGuiKeyCodeMap.end()) {
			return it->second;
		}
		return ImGuiKey_None;
	}

	int ImGuiInputMapper::toImGuiMouseCode(MouseButton button) {
		auto it = s_imGuiMouseCodeMap.find(button);
		if (it != s_imGuiMouseCodeMap.end()) {
			return it->second;
		}
		return -1;
	}


	const std::unordered_map<KeyCode, ImGuiKey> ImGuiInputMapper::s_imGuiKeyCodeMap = {

		// Letters
		{ KeyCode::A, ImGuiKey_A },
		{ KeyCode::B, ImGuiKey_B },
		{ KeyCode::C, ImGuiKey_C },
		{ KeyCode::D, ImGuiKey_D },
		{ KeyCode::E, ImGuiKey_E },
		{ KeyCode::F, ImGuiKey_F },
		{ KeyCode::G, ImGuiKey_G },
		{ KeyCode::H, ImGuiKey_H },
		{ KeyCode::I, ImGuiKey_I },
		{ KeyCode::J, ImGuiKey_J },
		{ KeyCode::K, ImGuiKey_K },
		{ KeyCode::L, ImGuiKey_L },
		{ KeyCode::M, ImGuiKey_M },
		{ KeyCode::N, ImGuiKey_N },
		{ KeyCode::O, ImGuiKey_O },
		{ KeyCode::P, ImGuiKey_P },
		{ KeyCode::Q, ImGuiKey_Q },
		{ KeyCode::R, ImGuiKey_R },
		{ KeyCode::S, ImGuiKey_S },
		{ KeyCode::T, ImGuiKey_T },
		{ KeyCode::U, ImGuiKey_U },
		{ KeyCode::V, ImGuiKey_V },
		{ KeyCode::W, ImGuiKey_W },
		{ KeyCode::X, ImGuiKey_X },
		{ KeyCode::Y, ImGuiKey_Y },
		{ KeyCode::Z, ImGuiKey_Z },

		// Numbers
		{ KeyCode::Zero, ImGuiKey_0 },
		{ KeyCode::One, ImGuiKey_1 },
		{ KeyCode::Two, ImGuiKey_2 },
		{ KeyCode::Three, ImGuiKey_3 },
		{ KeyCode::Four, ImGuiKey_4 },
		{ KeyCode::Five, ImGuiKey_5 },
		{ KeyCode::Six, ImGuiKey_6 },
		{ KeyCode::Seven, ImGuiKey_7 },
		{ KeyCode::Eight, ImGuiKey_8 },
		{ KeyCode::Nine, ImGuiKey_9 },

		// Special keys
		{ KeyCode::Space, ImGuiKey_Space },
		{ KeyCode::Enter, ImGuiKey_Enter },
		{ KeyCode::Escape, ImGuiKey_Escape },
		{ KeyCode::Tab, ImGuiKey_Tab },
		{ KeyCode::Backspace, ImGuiKey_Backspace },

		// Modifiers
		{ KeyCode::LeftControl, ImGuiKey_LeftCtrl },
		{ KeyCode::RightControl, ImGuiKey_RightCtrl },
		{ KeyCode::LeftShift, ImGuiKey_LeftShift },
		{ KeyCode::RightShift, ImGuiKey_RightShift },
		{ KeyCode::LeftAlt, ImGuiKey_LeftAlt },
		{ KeyCode::RightAlt, ImGuiKey_RightAlt },

		// Arrows
		{ KeyCode::Left, ImGuiKey_LeftArrow },
		{ KeyCode::Right, ImGuiKey_RightArrow },
		{ KeyCode::Up, ImGuiKey_UpArrow },
		{ KeyCode::Down, ImGuiKey_DownArrow },

		// Function keys
		{ KeyCode::F1, ImGuiKey_F1 },
		{ KeyCode::F2, ImGuiKey_F2 },
		{ KeyCode::F3, ImGuiKey_F3 },
		{ KeyCode::F4, ImGuiKey_F4 },
		{ KeyCode::F5, ImGuiKey_F5 },
		{ KeyCode::F6, ImGuiKey_F6 },
		{ KeyCode::F7, ImGuiKey_F7 },
		{ KeyCode::F8, ImGuiKey_F8 },
		{ KeyCode::F9, ImGuiKey_F9 },
		{ KeyCode::F10, ImGuiKey_F10 },
		{ KeyCode::F11, ImGuiKey_F11 },
		{ KeyCode::F12, ImGuiKey_F12 },

	};

	const std::unordered_map<MouseButton, int> ImGuiInputMapper::s_imGuiMouseCodeMap = {
		{ MouseButton::Left, 0 },
		{ MouseButton::Right, 1 },
		{ MouseButton::Middle, 2 },
	};
}
