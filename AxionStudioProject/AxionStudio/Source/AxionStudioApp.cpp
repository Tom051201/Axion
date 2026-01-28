#include <Axion.h>
#include <AxionEngine/Source/core/EntryPoint.h>

#include "AxionEngine/Vendor/imgui/backends/imgui_impl_win32.h"
#include "AxionEngine/Vendor/imgui/backends/imgui_impl_dx12.h"
#include "AxionEngine/Vendor/imgui/backends/imgui_impl_opengl3.h"

#include "EditorLayer.h"

namespace Axion {

	class AxionStudio : public Application {
	public:

		AxionStudio(const ApplicationSpecification& spec) : Application(spec) {

			pushLayer(new EditorLayer());
		}

		~AxionStudio() override {}

	};


	Application* createApplication() {
		WindowProperties windowProperties;
		windowProperties.title = "Axion Studio";
		windowProperties.dragAcceptFiles = true;
		windowProperties.iconFilePath = "AxionStudio/Resources/logo.ico";

		ApplicationSpecification spec;
		spec.windowProperties = windowProperties;
		spec.guiLayoutFilePath = "AxionStudio/Config/Layout.ini";
		spec.guiSyleSetter = []() {
			ImGuiStyle& style = ImGui::GetStyle();

			// sets layout
			style.FrameRounding = 2.0f;
			style.PopupRounding = 2.0f;
			style.ScrollbarRounding = 2.0f;
			style.GrabRounding = 2.0f;

			style.FramePadding = ImVec2(3.0f, 3.0f);
			style.ItemSpacing = ImVec2(3.0f, 3.0f);

			// sets colors
			ImVec4* colors = style.Colors;
			colors[ImGuiCol_WindowBg] = ImVec4(0.176f, 0.176f, 0.188f, 1.0f);
			colors[ImGuiCol_Text] = ImVec4(0.91f, 0.91f, 0.91f, 1.0f);

			colors[ImGuiCol_Header] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
			colors[ImGuiCol_HeaderHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
			colors[ImGuiCol_HeaderActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.45f);

			colors[ImGuiCol_Button] = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
			colors[ImGuiCol_ButtonHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
			colors[ImGuiCol_ButtonActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.45f);

			colors[ImGuiCol_MenuBarBg] = ImVec4(0.076f, 0.076f, 0.088f, 1.0f);

			colors[ImGuiCol_Border] = ImVec4(0.32f, 0.0f, 0.0f, 1.0f);
			colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);

			colors[ImGuiCol_TitleBg] = ImVec4(0.076f, 0.076f, 0.088f, 1.0f);
			colors[ImGuiCol_TitleBgActive] = ImVec4(0.076f, 0.076f, 0.088f, 1.0f);
			colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);

			colors[ImGuiCol_CheckMark] = ImVec4(0.32f, 0.0f, 0.0f, 1.0f);

			colors[ImGuiCol_FrameBg] = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
			colors[ImGuiCol_FrameBgActive] = ImVec4(0.32f, 0.0f, 0.0f, 1.0f);
			colors[ImGuiCol_FrameBgHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.45f);

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

			// sets fonts
			ImGuiIO& io = ImGui::GetIO();
			io.Fonts->Clear();

			io.Fonts->AddFontFromFileTTF("AxionStudio/Resources/fonts/openSans/OpenSans-Bold.ttf", 18.0f);
			ImFontConfig icons_config;
			icons_config.MergeMode = true;
			icons_config.PixelSnapH = true;
			static const ImWchar icon_ranges[] = { 0xE000, 0xF8FF, 0 };
			io.Fonts->AddFontFromFileTTF("AxionStudio/Resources/fonts/icons/MaterialSymbolsOutlined-Regular.ttf", 20.0f, &icons_config, icon_ranges);

			io.FontDefault = io.Fonts->Fonts[0];

			switch (Renderer::getAPI()) {
			case Axion::RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet"); return; }

			case Axion::RendererAPI::DirectX12: {
				ImGui_ImplDX12_InvalidateDeviceObjects();
				ImGui_ImplDX12_CreateDeviceObjects();
				break;
			}

			case Axion::RendererAPI::OpenGL3: {
				//ImGui_ImplOpenGL3_DestroyDeviceObjects();
				//ImGui_ImplOpenGL3_CreateDeviceObjects();	
				break;
			}
			}
		};

		return new AxionStudio(spec);
	}

}
