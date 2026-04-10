#include "PhysicsMaterialImportModal.h"

#include "AxionEngine/Vendor/imgui/imgui.h"
#include "AxionEngine/Vendor/imgui/misc/cpp/imgui_stdlib.h"

#include "AxionEngine/Source/EngineConfig.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/AssetVersions.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionAssetPipeline/Source/AxPhysicsMaterial.h"

namespace Axion {

	constexpr float inputFieldWidth = 200.0f;

	void PhysicsMaterialImportModal::renderContent() {
		ImGui::SeparatorText("Import Physics Material Asset");
		ImGui::Spacing();

		if (ImGui::BeginTable("##ImportPhysicsMaterialTable", 2, ImGuiTableFlags_BordersInnerV)) {
			ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 120.0f);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

			// -- Name --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Name");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##PhyMatName_input", &m_name);


			// -- Static Friction --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Static Friction");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::DragFloat("##staticFric_drag", &m_staticFriction, 0.05f, 0.0f, 10.0f);


			// -- Dynamic Friction --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Dynamic Friction");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::DragFloat("##dynamicFric_drag", &m_dynamicFriction, 0.05f, 0.0f, 10.0f);


			// -- Restitution --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Restitution");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::DragFloat("##Restitution_drag", &m_restitution, 0.05f, 0.0f, 1.0f);


			// -- Output path --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Separator();
			ImGui::Text("Output Location");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##PhyMatOutputPath_input", &m_outputPath);
			ImGui::SameLine();
			if (ImGui::Button("Browse...##PhyMatOutputDir_button")) {
				std::filesystem::path phymatDir = ProjectManager::getProject()->getAssetsPath() / "physics";
				std::filesystem::path absPath;
				if (std::filesystem::exists(phymatDir)) {
					absPath = FileDialogs::openFolder(phymatDir);
				}
				else {
					absPath = FileDialogs::openFolder(ProjectManager::getProject()->getAssetsPath());
				}
				if (!absPath.empty()) m_outputPath = absPath.string();
			}

			ImGui::EndTable();

			// -- Validate input --
			std::string finalName = m_name + ".axpmat";
			std::filesystem::path finalPath = std::filesystem::path(m_outputPath) / finalName;

			bool outputExists = std::filesystem::exists(m_outputPath);
			bool outputIsDirectory = std::filesystem::is_directory(m_outputPath);
			bool invalidOutFileName = std::filesystem::exists(finalPath);
			bool nameTooLong = m_name.length() > Config::MaxBinaryStringLength;

			bool disabled = (
				m_name.empty() ||
				m_outputPath.empty() ||
				!outputExists ||
				!outputIsDirectory ||
				invalidOutFileName ||
				nameTooLong
			);

			if (disabled) {
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 50, 50, 255));
				if (m_name.empty())ImGui::Text("Name needs to be set.");
				else if (m_outputPath.empty()) ImGui::Text("No output directory is set.");
				else if (!outputExists) ImGui::Text("Output directory does not exist.");
				else if (!outputIsDirectory) ImGui::Text("Output is not a directory.");
				else if (invalidOutFileName) ImGui::Text("Asset with this name already exists.");
				else if (nameTooLong) ImGui::Text("Name exceeds max limit (%d characters).", Config::MaxBinaryStringLength);
				ImGui::PopStyleColor();
			}
			else {
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(50, 255, 50, 255));
				ImGui::Text("Ready to create asset.");
				ImGui::PopStyleColor();
			}

			ImGui::Separator();
			ImGui::BeginDisabled(disabled);
			if (ImGui::Button("Create")) {
				UUID newAssetUUID = UUID::generate();

				AAP::PhysicsMaterialAssetData data;
				data.uuid = newAssetUUID;
				data.name = m_name;
				data.staticFriction = m_staticFriction;
				data.dynamicFriction = m_dynamicFriction;
				data.restitution = m_restitution;

				AAP::PhysicsMaterialParser::createTextFile(data, finalPath);

				AssetMetadata metadata;
				metadata.handle = newAssetUUID;
				metadata.type = AssetType::PhysicsMaterial;
				metadata.filePath = AssetManager::getRelativeToAssets(finalPath);

				auto registry = ProjectManager::getProject()->getAssetRegistry();
				registry->add(metadata);
				registry->serialize(ProjectManager::getProject()->getProjectPath() / "AssetRegistry.yaml");

				close();
			}
			ImGui::EndDisabled();
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) {
				close();
			}

			// -- Version --
			std::string versionText = "v" + std::to_string(ASSET_VERSION_PHYSICS_MATERIAL);
			float textWidth = ImGui::CalcTextSize(versionText.c_str()).x;
			float windowWidth = ImGui::GetWindowWidth();
			ImGui::SameLine(windowWidth - textWidth - ImGui::GetStyle().WindowPadding.x);
			ImGui::TextDisabled("%s", versionText.c_str());

		}
	}

	void PhysicsMaterialImportModal::resetInputs() {
		m_name.clear();
		m_outputPath.clear();
		m_staticFriction = 0.5f;
		m_dynamicFriction = 0.5f;
		m_restitution = 0.05f;
	}

}
