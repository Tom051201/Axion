#pragma once
#include "axpch.h"

#include "AxionEngine/Vendor/imgui/imgui.h"
#include "AxionEngine/Source/input/Input.h"

namespace Axion {

	class ImGuiInputMapper {
	public:

		static ImGuiKey toImGuiKeyCode(KeyCode code);

	private:

		static const std::unordered_map<KeyCode, ImGuiKey> s_imGuiKeyCodeMap;

	};

}
