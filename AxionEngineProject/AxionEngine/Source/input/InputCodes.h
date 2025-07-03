#pragma once
#include "axpch.h"

#include "AxionEngine/Vendor/spdlog/include/spdlog/fmt/fmt.h"

namespace Axion {

	enum class KeyCode : uint16_t {
		// Letter keys
		A = 0x0001,			B,					C,
		D,					E,					F,
		G,					H,					I,
		J,					K,					L,
		M,					N,					O,
		P,					Q,					R,
		S,					T,					U,
		V,					W,					X,
		Y,					Z,

		// Number keys
		Zero,				One,				Two,
		Three,				Four,				Five,
		Six,				Seven,				Eight,
		Nine,

		// Symbol keys
		Semicolon,			Equal,				Comma,
		Minus,				Period,				Slash,
		GraveAccent,		LeftBracket,		Backslash,
		RightBracket, Apostrophe,

		// Function keys
		F1,					F2,					F3,
		F4,					F5,					F6,
		F7,					F8,					F9,
		F10,				F11,				F12,

		// Naviagtion keys
		Left,				Right,
		Up,					Down,
		PageUp,				PageDown,
		Home,				End,
		Insert,				Delete,

		// Modifiers
		LeftShift,			RightShift,
		LeftControl,		RightControl,
		LeftAlt,			RightAlt,
		CapsLock,			NumLock,
		ScrollLock,			PrintScreen,
		Pause,

		// Special keys
		Space,				Enter,				Escape,
		Tab,				Backspace,

		// Numpad keys
		Numpad0,			Numpad1,			Numpad2,
		Numpad3,			Numpad4,			Numpad5,
		Numpad6,			Numpad7,			Numpad8,
		Numpad9,
		NumpadDecimal,		NumpadDivide,		NumpadMultiply,
		NumpadSubtract,		NumpadAdd,


		Unknown = 0xFFFF
		
	};



	enum class MouseButton : uint16_t {
		Left = 0x0001,		Right,				Middle,
		X1,					X2,


		Unknown = 0xFFFF
	};

}
