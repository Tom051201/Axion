#include "AssetManagerPanel.h"

#include "AxionEngine/Vendor/imgui/imgui.h"

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/UUID.h"
#include "AxionEngine/Source/render/Mesh.h"
#include "AxionEngine/Source/render/Shader.h"
#include "AxionEngine/Source/render/Material.h"
#include "AxionEngine/Source/scene/Skybox.h"

namespace Axion {

	AssetManagerPanel::AssetManagerPanel(const std::string& name) : Panel(name) {}

	AssetManagerPanel::~AssetManagerPanel() {
		shutdown();
	}

	void AssetManagerPanel::setup() {}

	void AssetManagerPanel::shutdown() {}

	void AssetManagerPanel::onGuiRender() {
		ImGui::Begin("Asset Manager Inspector");

		// -- Mesh assets --
		drawAssetInfo<Mesh>("Mesh", [](Ref<Mesh> mesh) {
			ImGui::Text("Vertices: %u", mesh->getVertexBuffer()->getVertexCount());
			ImGui::Text("Indices: %u", mesh->getIndexCount());
		});


		// -- Material Assets --
		ImGui::SeparatorText("");
		drawAssetInfo<Material>("Material", [](Ref<Material> material) {
			ImGui::Text("Name: %s", material->getName().c_str());
		});


		// -- Skybox Assets --
		ImGui::SeparatorText("");
		drawAssetInfo<Skybox>("Skybox", [](Ref<Skybox> skybox) {
			ImGui::Text("Cubemap Path: %s", AssetManager::getRelativeToAssets(skybox->getTexturePath()).c_str());
		});


		// -- Shader Assets --
		ImGui::SeparatorText("");
		drawAssetInfo<Shader>("Shader", [](Ref<Shader> shader) {
			ImGui::Text("Name: %s", shader->getName().c_str());
		});


		ImGui::End();
	}

}
