#include "AssetManagerPanel.h"

#include "AxionEngine/Vendor/imgui/imgui.h"

#include "AxionEngine/Source/core/AssetManager.h"
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
		const auto& meshes = AssetManager::getMap<Mesh>();
		std::string meshesLabel = "Meshes (" + std::to_string(meshes.size()) + ")";
		if (ImGui::CollapsingHeader(meshesLabel.c_str())) {
			if (meshes.empty()) {
				ImGui::TextDisabled("No meshes loaded");
			}
			else {
				for (const auto& [handle, mesh] : meshes) {
					ImGui::PushID((int)std::hash<UUID>()(handle.uuid));

					bool open = ImGui::TreeNodeEx("Mesh", ImGuiTreeNodeFlags_SpanAvailWidth, "Mesh [%s]", handle.uuid.toString().c_str());

					if (open) {
						ImGui::Text("Asset File: %s", AssetManager::getRelativeToAssets(AssetManager::getAssetFilePath<Mesh>(handle)).c_str());

						if (mesh) {
							ImGui::Text("Vertices: %u", mesh->getVertexBuffer()->getVertexCount());
							ImGui::Text("Indices: %u", mesh->getIndexCount());
						}
						else {
							ImGui::TextDisabled("Mesh data not loaded");
						}
						ImGui::TreePop();
					}
					ImGui::PopID();
				}
			}
		}

		// -- Skybox Assets --
		const auto& skyboxes = AssetManager::getMap<Skybox>();
		std::string skyboxesLabel = "Skyboxes (" + std::to_string(skyboxes.size()) + ")";
		if (ImGui::CollapsingHeader(skyboxesLabel.c_str())) {
			if (skyboxes.empty()) {
				ImGui::TextDisabled("No skyboxes loaded");
			}
			else {
				for (const auto& [handle, skybox] : skyboxes) {
					ImGui::PushID((int)std::hash<UUID>()(handle.uuid));

					bool open = ImGui::TreeNodeEx("Skybox", ImGuiTreeNodeFlags_SpanAvailWidth, "Skybox [%s]", handle.uuid.toString().c_str());

					if (open) {
						ImGui::Text("Asset File: %s", AssetManager::getRelativeToAssets(AssetManager::getAssetFilePath<Skybox>(handle)).c_str());

						if (skybox) {
							ImGui::Text("Cubemap Path: %s", AssetManager::getRelativeToAssets(skybox->getTexturePath()).c_str());
						}
						else {
							ImGui::TextDisabled("Skybox is queued for load.");
						}
						ImGui::TreePop();
					}

					ImGui::PopID();
				}
			}
		}

		// -- Shader Assets --
		const auto& shaders = AssetManager::getMap<Shader>();
		std::string shadersLabel = "Shaders (" + std::to_string(shaders.size()) + ")";
		if (ImGui::CollapsingHeader(shadersLabel.c_str())) {
			if (shaders.empty()) {
				ImGui::TextDisabled("No shaders loaded");
			}
			else {
				for (const auto& [handle, shader] : shaders) {
					ImGui::PushID((int)std::hash<UUID>()(handle.uuid));

					bool open = ImGui::TreeNodeEx("Shader", ImGuiTreeNodeFlags_SpanAvailWidth, "Shader [%s]", handle.uuid.toString().c_str());

					if (open) {
						ImGui::Text("Asset File: %s", AssetManager::getRelativeToAssets(AssetManager::getAssetFilePath<Shader>(handle)).c_str());

						if (shader) {
							ImGui::Text("Name: %s", shader->getName().c_str());
						}
						else {
							ImGui::TextDisabled("Shader is queued for load.");
						}
						ImGui::TreePop();
					}

					ImGui::PopID();
				}
			}
		}

		// -- Material Assets --
		const auto& materials = AssetManager::getMap<Material>();
		std::string materialsLabel = "Materials (" + std::to_string(materials.size()) + ")";
		if (ImGui::CollapsingHeader(materialsLabel.c_str())) {
			if (materials.empty()) {
				ImGui::TextDisabled("No materials loaded");
			}
			else {
				for (const auto& [handle, material] : materials) {
					ImGui::PushID((int)std::hash<UUID>()(handle.uuid));

					bool open = ImGui::TreeNodeEx("Material", ImGuiTreeNodeFlags_SpanAvailWidth, "Material [%s]", handle.uuid.toString().c_str());

					if (open) {
						ImGui::Text("Asset File: %s", AssetManager::getRelativeToAssets(AssetManager::getAssetFilePath<Material>(handle)).c_str());

						if (material) {
							ImGui::Text("Name: %s", material->getName().c_str());
						}
						else {
							ImGui::TextDisabled("Material is queued for load.");
						}
						ImGui::TreePop();
					}

					ImGui::PopID();
				}
			}
		}

		ImGui::End();
	}

}
