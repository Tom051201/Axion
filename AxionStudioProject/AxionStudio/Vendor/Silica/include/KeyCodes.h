#pragma once

#include <cstdint>

namespace Silica {

	enum class Key : uint32_t {
		Unknown = 0,

		// -- NAVIGATION --
		Left, Right, Up, Down,

		// -- CONTROL --
		Backspace, Delete, Enter, Escape, Space, Tab,

		// -- MODIFIERS --
		LeftShift, RightShift, LeftControl, RightControl, LeftAlt, RightAlt,

		// -- LETTERS --
		A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

		// -- NUMBERS --
		Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9

	};

}
