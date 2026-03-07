#include <Axion.h>
#include <AxionEngine/Source/core/EntryPoint.h>

#include "AxionEngine/Vendor/imgui/backends/imgui_impl_win32.h"
#include "AxionEngine/Vendor/imgui/backends/imgui_impl_dx12.h"
#include "AxionEngine/Vendor/imgui/backends/imgui_impl_opengl3.h"

#include "EditorLayer.h"
#include "AxionStudio/Source/core/EditorTheme.h"

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
			// -- Sets fonts and icons --
			ImGuiIO& io = ImGui::GetIO();
			io.Fonts->Clear();
			io.Fonts->AddFontFromFileTTF("AxionStudio/Resources/fonts/openSans/OpenSans-Bold.ttf", 18.0f);
			ImFontConfig icons_config;
			icons_config.MergeMode = true;
			icons_config.PixelSnapH = true;
			static const ImWchar icon_ranges[] = { 0xE000, 0xF8FF, 0 };
			io.Fonts->AddFontFromFileTTF("AxionStudio/Resources/fonts/icons/MaterialSymbolsOutlined-Regular.ttf", 20.0f, &icons_config, icon_ranges);
			io.FontDefault = io.Fonts->Fonts[0];

			// -- Loads editor theme from settings file --
			EditorTheme::Theme savedTheme = EditorTheme::loadTheme("AxionStudio/Config/Settings.yaml");
			EditorTheme::setTheme(savedTheme);

			// -- Init graphics API --
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
