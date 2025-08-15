#pragma once
#include "axpch.h"

#include "AxionEngine/Vendor/imgui/imgui.h"
#include "AxionEngine/Source/input/Input.h"

namespace Axion {

	class ImGuiInputMapper {
	public:

		static ImGuiKey toImGuiKeyCode(KeyCode code);
		static int toImGuiMouseCode(MouseButton button);

	private:

		static const std::unordered_map<KeyCode, ImGuiKey> s_imGuiKeyCodeMap;
		static const std::unordered_map<MouseButton, int> s_imGuiMouseCodeMap;

	};

}
