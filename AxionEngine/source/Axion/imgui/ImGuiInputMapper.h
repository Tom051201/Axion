#pragma once

#include "axpch.h"
#include "imgui.h"
#include "Axion/input/Input.h"

namespace Axion {

	class ImGuiInputMapper {
	public:

		static ImGuiKey toImGuiKeyCode(KeyCode code);

	private:

		static const std::unordered_map<KeyCode, ImGuiKey> s_imGuiKeyCodeMap;

	};

}
