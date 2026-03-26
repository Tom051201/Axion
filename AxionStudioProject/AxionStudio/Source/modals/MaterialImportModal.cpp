#include "MaterialImportModal.h"

#include "AxionEngine/Vendor/imgui/imgui.h"
#include "AxionEngine/Vendor/imgui/misc/cpp/imgui_stdlib.h"

#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/AssetVersions.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionAssetPipeline/Source/AxMaterial.h"

namespace Axion {

	constexpr float inputFieldWidth = 200.0f;

	void MaterialImportModal::renderContent() {

		ImGui::SeparatorText("Import Material Asset");
		ImGui::Spacing();

		if (ImGui::BeginTable("##ImportMaterialTable", 2, ImGuiTableFlags_BordersInnerV)) {
			ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 120.0f);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

			// -- Name --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Name");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##MatName_input", &m_name);


			// -- AlbedoColor --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Albedo Color");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::ColorEdit4("##AlbedoColor_edit", m_albedoColor.data());


			// -- Metalness --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Metalness");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::DragFloat("##Metalness_drag", &m_metalness, 0.05f, 0.0f, 1.0f);


			// -- Roughness --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Roughness");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::DragFloat("##Roughness_drag", &m_roughness, 0.05f, 0.0f, 1.0f);


			// -- Emission --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Emission");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::DragFloat("##Emission_drag", &m_emission, 0.05f, 0.0f, 1.0f);


			// -- Tiling --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Tiling");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::DragFloat("##tiling_drag", &m_tiling, 0.05f, 0.0f, 1.0f);


			// -- Albedo Map --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Albedo Map");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##AlbedoMapPath_input", &m_albedoMapPath);
			ImGui::SameLine();
			if (ImGui::Button("Browse...##AlbedoMapFile_button")) {
				std::filesystem::path dir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "textures";
				std::string absPath;
				if (std::filesystem::exists(dir)) {
					absPath = FileDialogs::openFile({ {"Axion Texture Asset", "*.axtex"} }, dir.string());
				}
				else {
					absPath = FileDialogs::openFile({ {"Axion Texture Asset", "*.axtex"} }, ProjectManager::getProject()->getAssetsPath());
				}
				if (!absPath.empty()) m_albedoMapPath = absPath;
			}

			// -- Normal Map --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Normal Map");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##NormalMapPath_input", &m_normalMapPath);
			ImGui::SameLine();
			if (ImGui::Button("Browse...##NormalMapFile_button")) {
				std::filesystem::path dir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "textures";
				std::string absPath;
				if (std::filesystem::exists(dir)) {
					absPath = FileDialogs::openFile({ {"Axion Texture Asset", "*.axtex"} }, dir.string());
				}
				else {
					absPath = FileDialogs::openFile({ {"Axion Texture Asset", "*.axtex"} }, ProjectManager::getProject()->getAssetsPath());
				}
				if (!absPath.empty()) m_normalMapPath = absPath;
			}

			// -- Metalness Map --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Metalness Map");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##MetalnessMapPath_input", &m_metalnessMapPath);
			ImGui::SameLine();
			if (ImGui::Button("Browse...##MetalnessMapFile_button")) {
				std::filesystem::path dir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "textures";
				std::string absPath;
				if (std::filesystem::exists(dir)) {
					absPath = FileDialogs::openFile({ {"Axion Texture Asset", "*.axtex"} }, dir.string());
				}
				else {
					absPath = FileDialogs::openFile({ {"Axion Texture Asset", "*.axtex"} }, ProjectManager::getProject()->getAssetsPath());
				}
				if (!absPath.empty()) m_metalnessMapPath = absPath;
			}

			// -- Roughness Map --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Roughness Map");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##RoughnessMapPath_input", &m_roughnessMapPath);
			ImGui::SameLine();
			if (ImGui::Button("Browse...##RoughnessMapFile_button")) {
				std::filesystem::path dir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "textures";
				std::string absPath;
				if (std::filesystem::exists(dir)) {
					absPath = FileDialogs::openFile({ {"Axion Texture Asset", "*.axtex"} }, dir.string());
				}
				else {
					absPath = FileDialogs::openFile({ {"Axion Texture Asset", "*.axtex"} }, ProjectManager::getProject()->getAssetsPath());
				}
				if (!absPath.empty()) m_roughnessMapPath = absPath;
			}

			// -- Occlusion Map --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Occlusion Map");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##OcclusionMapPath_input", &m_occlusionMapPath);
			ImGui::SameLine();
			if (ImGui::Button("Browse...##OcclusionMapFile_button")) {
				std::filesystem::path dir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "textures";
				std::string absPath;
				if (std::filesystem::exists(dir)) {
					absPath = FileDialogs::openFile({ {"Axion Texture Asset", "*.axtex"} }, dir.string());
				}
				else {
					absPath = FileDialogs::openFile({ {"Axion Texture Asset", "*.axtex"} }, ProjectManager::getProject()->getAssetsPath());
				}
				if (!absPath.empty()) m_occlusionMapPath = absPath;
			}

			// -- Emissive Map --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Emissive Map");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##EmissiveMapPath_input", &m_emissiveMapPath);
			ImGui::SameLine();
			if (ImGui::Button("Browse...##EmissiveMapFile_button")) {
				std::filesystem::path dir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "textures";
				std::string absPath;
				if (std::filesystem::exists(dir)) {
					absPath = FileDialogs::openFile({ {"Axion Texture Asset", "*.axtex"} }, dir.string());
				}
				else {
					absPath = FileDialogs::openFile({ {"Axion Texture Asset", "*.axtex"} }, ProjectManager::getProject()->getAssetsPath());
				}
				if (!absPath.empty()) m_emissiveMapPath = absPath;
			}

			// -- Pipeline path --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Pipeline");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##MatPipelinePath_input", &m_pipelinePath);
			ImGui::SameLine();
			if (ImGui::Button("Browse...##MatPipelineFile_button")) {
				std::filesystem::path dir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "pipelines";
				std::string absPath;
				if (std::filesystem::exists(dir)) {
					absPath = FileDialogs::openFile({ {"Axion Pipeline Asset", "*.axpso"} }, dir.string());
				}
				else {
					absPath = FileDialogs::openFile({ {"Axion Pipeline Asset", "*.axpso"} }, ProjectManager::getProject()->getAssetsPath());
				}
				if (!absPath.empty()) m_pipelinePath = absPath;
			}


			// -- Output path --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Output Location");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##MatOutputPath_input", &m_outputPath);
			ImGui::SameLine();
			if (ImGui::Button("Browse...##MatOutputDir_button")) {
				std::filesystem::path dir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "materials";
				std::string absPath;
				if (std::filesystem::exists(dir)) {
					absPath = FileDialogs::openFolder(dir.string());
				}
				else {
					absPath = FileDialogs::openFolder(ProjectManager::getProject()->getAssetsPath());
				}
				if (!absPath.empty()) m_outputPath = absPath;
			}

			ImGui::EndTable();

			// -- Validate input --
			std::string finalName = m_name + ".axmat";
			std::filesystem::path finalPath = std::filesystem::path(m_outputPath) / finalName;

			bool hasPipeline = !m_pipelinePath.empty();
			bool pipelineExists = true;
			bool pipelineIsFile = true;
			if (hasPipeline) {
				pipelineExists = std::filesystem::exists(m_pipelinePath);
				pipelineIsFile = std::filesystem::is_regular_file(m_pipelinePath);
			}

			bool hasTexAlbedo = !m_albedoMapPath.empty();
			bool texAlbedoExists = true;
			bool texAlbedoIsFile = true;
			if (hasTexAlbedo) {
				texAlbedoExists = std::filesystem::exists(m_albedoMapPath);
				texAlbedoIsFile = std::filesystem::is_regular_file(m_albedoMapPath);
			}

			bool hasTexNormal = !m_normalMapPath.empty();
			bool texNormalExists = true;
			bool texNormalIsFile = true;
			if (hasTexNormal) {
				texNormalExists = std::filesystem::exists(m_normalMapPath);
				texNormalIsFile = std::filesystem::is_regular_file(m_normalMapPath);
			}

			bool hasTexMetalness = !m_metalnessMapPath.empty();
			bool texMetalnessExists = true;
			bool texMetalnessIsFile = true;
			if (hasTexMetalness) {
				texMetalnessExists = std::filesystem::exists(m_metalnessMapPath);
				texMetalnessIsFile = std::filesystem::is_regular_file(m_metalnessMapPath);
			}

			bool hasTexRoughness = !m_roughnessMapPath.empty();
			bool texRoughnessExists = true;
			bool texRoughnessIsFile = true;
			if (hasTexRoughness) {
				texRoughnessExists = std::filesystem::exists(m_roughnessMapPath);
				texRoughnessIsFile = std::filesystem::is_regular_file(m_roughnessMapPath);
			}

			bool hasTexOcclusion = !m_occlusionMapPath.empty();
			bool texOcclusionExists = true;
			bool texOcclusionIsFile = true;
			if (hasTexOcclusion) {
				texOcclusionExists = std::filesystem::exists(m_occlusionMapPath);
				texOcclusionIsFile = std::filesystem::is_regular_file(m_occlusionMapPath);
			}

			bool hasTexEmissive = !m_emissiveMapPath.empty();
			bool texEmissiveExists = true;
			bool texEmissiveIsFile = true;
			if (hasTexEmissive) {
				texEmissiveExists = std::filesystem::exists(m_emissiveMapPath);
				texEmissiveIsFile = std::filesystem::is_regular_file(m_emissiveMapPath);
			}

			bool outputExists = std::filesystem::exists(m_outputPath);
			bool outputIsDirectory = std::filesystem::is_directory(m_outputPath);
			bool invalidOutFileName = std::filesystem::exists(finalPath);

			bool disabled = (
				m_name.empty() ||
				m_outputPath.empty() ||
				!pipelineExists ||
				!pipelineIsFile ||
				!texAlbedoExists ||
				!texAlbedoIsFile ||
				!texNormalExists ||
				!texNormalIsFile ||
				!texMetalnessExists ||
				!texMetalnessIsFile ||
				!texRoughnessExists ||
				!texRoughnessIsFile ||
				!texOcclusionExists ||
				!texOcclusionIsFile ||
				!texEmissiveExists ||
				!texEmissiveIsFile ||
				!outputExists ||
				!outputIsDirectory ||
				invalidOutFileName
			);

			if (disabled) {
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 50, 50, 255));
				if (m_name.empty()) ImGui::Text("No name is set.");
				else if (m_outputPath.empty()) ImGui::Text("No output directory is set.");
				else if (!pipelineExists) ImGui::Text("Pipeline file does not exist.");
				else if (!pipelineIsFile) ImGui::Text("Pipeline path is not a file.");
				else if (!texAlbedoExists) ImGui::Text("Albedo map file does not exist.");
				else if (!texAlbedoIsFile) ImGui::Text("Albedo map is not a file.");
				else if (!texNormalExists) ImGui::Text("Normal map file does not exist.");
				else if (!texNormalIsFile) ImGui::Text("Normal map is not a file.");
				else if (!texMetalnessExists) ImGui::Text("Metalness map file does not exist.");
				else if (!texMetalnessIsFile) ImGui::Text("Metalness map is not a file.");
				else if (!texRoughnessExists) ImGui::Text("Roughness map file does not exist.");
				else if (!texRoughnessIsFile) ImGui::Text("Roughness map is not a file.");
				else if (!texOcclusionExists) ImGui::Text("Occlusion map file does not exist.");
				else if (!texOcclusionIsFile) ImGui::Text("Occlusion map is not a file.");
				else if (!texEmissiveExists) ImGui::Text("Emissive map file does not exist.");
				else if (!texEmissiveIsFile) ImGui::Text("Emissive map is not a file.");
				else if (!outputExists) ImGui::Text("Output directory does not exist.");
				else if (!outputIsDirectory) ImGui::Text("Output is not a directory.");
				else if (invalidOutFileName) ImGui::Text("Asset with this name already exists.");
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

				AAP::MaterialAssetData data;
				data.uuid = newAssetUUID;
				data.name = m_name;
				if (!m_pipelinePath.empty()) {
					data.pipelineAsset = AssetManager::getRelativeToAssets(m_pipelinePath);
				}

				MaterialProperties prop;
				prop.albedoColor = m_albedoColor;
				prop.metalness = m_metalness;
				prop.roughness = m_roughness;
				prop.emissionStrength = m_emission;
				prop.tiling = m_tiling;
				prop.useNormalMap = 0.0f;
				prop.useMetalnessMap = 0.0f;
				prop.useRoughnessMap = 0.0f;
				prop.useOcclusionMap = 0.0f;

				if (hasTexAlbedo && texAlbedoExists && texAlbedoIsFile) {
					data.textures[TextureSlot::Albedo] = AssetManager::getRelativeToAssets(m_albedoMapPath);
				}

				if (hasTexNormal && texNormalExists && texNormalIsFile) {
					data.textures[TextureSlot::Normal] = AssetManager::getRelativeToAssets(m_normalMapPath);
					prop.useNormalMap = 1.0f;
				}

				if (hasTexMetalness && texMetalnessExists && texMetalnessIsFile) {
					data.textures[TextureSlot::Metalness] = AssetManager::getRelativeToAssets(m_metalnessMapPath);
					prop.useMetalnessMap = 1.0f;
				}

				if (hasTexRoughness && texRoughnessExists && texRoughnessIsFile) {
					data.textures[TextureSlot::Roughness] = AssetManager::getRelativeToAssets(m_roughnessMapPath);
					prop.useRoughnessMap = 1.0f;
				}

				if (hasTexOcclusion && texOcclusionExists && texOcclusionIsFile) {
					data.textures[TextureSlot::Occlusion] = AssetManager::getRelativeToAssets(m_occlusionMapPath);
					prop.useOcclusionMap = 1.0f;
				}

				if (hasTexEmissive && texEmissiveExists && texEmissiveIsFile) {
					data.textures[TextureSlot::Emissive] = AssetManager::getRelativeToAssets(m_emissiveMapPath);
					//prop.useEmissiveMap = 1.0f; // TODO: why no emissive map field?
				}

				data.properties = prop;
				AAP::MaterialParser::createTextFile(data, finalPath.string());

				AssetMetadata metadata;
				metadata.handle = newAssetUUID;
				metadata.type = AssetType::Material;
				metadata.filePath = AssetManager::getRelativeToAssets(finalPath.string());

				auto registry = ProjectManager::getProject()->getAssetRegistry();
				registry->add(metadata);
				registry->serialize((std::filesystem::path(ProjectManager::getProject()->getProjectPath()) / "AssetRegistry.yaml").string());

				close();
			}
			ImGui::EndDisabled();
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) {
				close();
			}

			// -- Version --
			std::string versionText = "v" + std::to_string(ASSET_VERSION_MATERIAL);
			float textWidth = ImGui::CalcTextSize(versionText.c_str()).x;
			float windowWidth = ImGui::GetWindowWidth();
			ImGui::SameLine(windowWidth - textWidth - ImGui::GetStyle().WindowPadding.x);
			ImGui::TextDisabled("%s", versionText.c_str());

		}

	}

	void MaterialImportModal::resetInputs() {
		m_name.clear();
		m_pipelinePath.clear();
		m_outputPath.clear();
		m_albedoColor = Vec4::one();
		m_metalness = 0.0f;
		m_roughness = 0.0f;
		m_emission = 0.0f;
		m_tiling = 0.0f;
		m_albedoMapPath.clear();
		m_normalMapPath.clear();
		m_metalnessMapPath.clear();
		m_roughnessMapPath.clear();
		m_occlusionMapPath.clear();
		m_emissiveMapPath.clear();
	}

}
