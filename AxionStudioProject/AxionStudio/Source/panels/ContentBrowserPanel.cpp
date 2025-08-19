#include "ContentBrowserPanel.h"

#include "AxionEngine/Vendor/imgui/imgui.h"

#include "AxionEngine/Source/core/AssetManager.h"

namespace Axion {

	ContentBrowserPanel::ContentBrowserPanel() {

	}

	ContentBrowserPanel::~ContentBrowserPanel() {
		shutdown();
	}

	void ContentBrowserPanel::setup() {
	
	}

	void ContentBrowserPanel::shutdown() {

	}

	void ContentBrowserPanel::onGuiRender() {
		if (ImGui::Begin("Content Browser")) {
			for (auto& [handle, mesh] : AssetManager::getMeshMap()) {
				if (ImGui::Selectable(handle.path.c_str())) {
					// Do something
				}
			}
		}
		ImGui::End();
	}

}
