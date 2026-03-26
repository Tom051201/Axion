#include "EditorTheme.h"

#include "AxionEngine/Source/core/Logging.h"

#include "AxionEngine/Vendor/imgui/imgui.h"
#include "AxionEngine/Vendor/yaml-cpp/include/yaml-cpp/yaml.h"

#include <fstream>

namespace Axion {

	void EditorTheme::setTheme(Theme theme) {

		// -- Shared structural layout --
		ImGuiStyle& style = ImGui::GetStyle();
		style.FrameRounding = 2.0f;
		style.PopupRounding = 2.0f;
		style.ScrollbarRounding = 2.0f;
		style.GrabRounding = 2.0f;
		style.FramePadding = ImVec2(3.0f, 3.0f);
		style.ItemSpacing = ImVec2(3.0f, 3.0f);

		// -- Shared base dark color --
		ImVec4* colors = style.Colors;
		colors[ImGuiCol_WindowBg] = ImVec4(0.176f, 0.176f, 0.188f, 1.0f);
		colors[ImGuiCol_Text] = ImVec4(0.91f, 0.91f, 0.91f, 1.0f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.076f, 0.076f, 0.088f, 1.0f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.076f, 0.076f, 0.088f, 1.0f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.076f, 0.076f, 0.088f, 1.0f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.076f, 0.076f, 0.088f, 1.0f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.65f);

		// -- RED BLACK RELEASE I THEME --
		if (theme == Theme::RedBlack) {

			colors[ImGuiCol_Border] = ImVec4(0.32f, 0.0f, 0.0f, 1.0f);
			colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
			colors[ImGuiCol_CheckMark] = ImVec4(0.32f, 0.0f, 0.0f, 1.0f);
			
			colors[ImGuiCol_Header] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
			colors[ImGuiCol_HeaderHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
			colors[ImGuiCol_HeaderActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.45f);
			
			colors[ImGuiCol_Button] = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
			colors[ImGuiCol_ButtonHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
			colors[ImGuiCol_ButtonActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.45f);
			
			colors[ImGuiCol_FrameBg] = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
			colors[ImGuiCol_FrameBgActive] = ImVec4(0.32f, 0.0f, 0.0f, 1.0f);
			colors[ImGuiCol_FrameBgHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.45f);
			
			colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
			
			colors[ImGuiCol_ResizeGrip] = ImVec4(0.32f, 0.0f, 0.0f, 1.0f);
			colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
			colors[ImGuiCol_ResizeGripActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.45f);
			
			colors[ImGuiCol_Tab] = ImVec4(0.32f, 0.0f, 0.0f, 1.0f);
			colors[ImGuiCol_TabActive] = ImVec4(0.32f, 0.0f, 0.0f, 1.0f);
			colors[ImGuiCol_TabHovered] = ImVec4(0.42f, 0.0f, 0.0f, 1.0f);
			colors[ImGuiCol_TabDimmed] = ImVec4(0.32f, 0.0f, 0.0f, 1.0f);
			colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.32f, 0.0f, 0.0f, 1.0f);
			colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.62f, 0.0f, 0.0f, 1.0f);
			colors[ImGuiCol_TabSelected] = ImVec4(0.52f, 0.0f, 0.0f, 1.0f);
			colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.62f, 0.0f, 0.0f, 1.0f);

		}

		// -- PURPLE ACCENTS RELEASE II THEME --
		else if (theme == Theme::Purple) {

			ImVec4 purpleAccent = ImVec4(0.45f, 0.25f, 0.68f, 1.0f);
			ImVec4 purpleHover = ImVec4(0.55f, 0.35f, 0.78f, 1.0f);
			ImVec4 purpleActive = ImVec4(0.35f, 0.15f, 0.58f, 1.0f);

			colors[ImGuiCol_Border] = purpleAccent;
			colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
			colors[ImGuiCol_CheckMark] = purpleAccent;

			colors[ImGuiCol_Header] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
			colors[ImGuiCol_HeaderHovered] = purpleHover;
			colors[ImGuiCol_HeaderActive] = purpleActive;

			colors[ImGuiCol_Button] = purpleAccent;
			colors[ImGuiCol_ButtonHovered] = purpleHover;
			colors[ImGuiCol_ButtonActive] = purpleActive;

			colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.12f, 0.14f, 1.0f);
			colors[ImGuiCol_FrameBgActive] = purpleActive;
			colors[ImGuiCol_FrameBgHovered] = purpleHover;

			colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.12f, 0.12f, 0.14f, 1.0f);

			colors[ImGuiCol_ResizeGrip] = purpleAccent;
			colors[ImGuiCol_ResizeGripHovered] = purpleHover;
			colors[ImGuiCol_ResizeGripActive] = purpleActive;

			colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
			colors[ImGuiCol_TabActive] = purpleAccent;
			colors[ImGuiCol_TabHovered] = purpleHover;
			colors[ImGuiCol_TabDimmed] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
			colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
			colors[ImGuiCol_TabDimmedSelectedOverline] = purpleAccent;
			colors[ImGuiCol_TabSelected] = purpleActive;
			colors[ImGuiCol_TabSelectedOverline] = purpleHover;

		}

	}

	EditorTheme::Theme EditorTheme::loadTheme(const char* filePath) {
		try {
			std::ifstream stream(filePath);
			if (stream.good()) {
				YAML::Node data = YAML::Load(stream);
				if (data["Theme"]) {
					return (Theme)data["Theme"].as<int>();
				}
			}
		}
		catch (...) {
			AX_CORE_LOG_WARN("Failed to load Editor theme from settings file");
		}
		return Theme::RedBlack;
	}

	void EditorTheme::saveTheme(Theme theme, const char* filePath) {
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Theme" << YAML::Value << (int)theme;
		out << YAML::EndMap;

		std::ofstream fout(filePath);
		fout << out.c_str();
	}

}
