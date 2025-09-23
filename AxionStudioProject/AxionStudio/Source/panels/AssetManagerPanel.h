#pragma once

#include "AxionStudio/Source/core/Panel.h"

namespace Axion {

	class AssetManagerPanel : public Panel {
	public:

		AssetManagerPanel(const std::string& name);
		~AssetManagerPanel() override;

		void setup() override;
		void shutdown() override;
		void onGuiRender() override;

	private:

		template<typename T>
		void drawAssetInfo(const char* name, std::function<void(Ref<T>)> elementFunc) {
			const auto& map = AssetManager::getMap<T>();
			std::string label = std::string(name) + " (" + std::to_string(map.size()) + ")";
			if (ImGui::CollapsingHeader(label.c_str())) {
				// -- No assets in manager --
				if (map.empty()) {
					ImGui::TextDisabled("No %s loaded", name);
					return;
				}

				// -- Draw asset info --
				for (const auto& [handle, asset] : map) {
					ImGui::PushID((int)std::hash<UUID>()(handle.uuid));
					bool open = ImGui::TreeNodeEx(name, ImGuiTreeNodeFlags_SpanAvailWidth, "%s [%s]", name, handle.uuid.toString().c_str());

					if (open) {
						ImGui::Text("Asset File: %s", AssetManager::getRelativeToAssets(AssetManager::getAssetFilePath<T>(handle)).c_str());

						if (asset) { elementFunc(asset); }
						else { ImGui::TextDisabled("%s data not loaded", name); }

						ImGui::Separator();
						ImGui::TreePop();
					}
					ImGui::PopID();
				}
			}
		}

	};

}
