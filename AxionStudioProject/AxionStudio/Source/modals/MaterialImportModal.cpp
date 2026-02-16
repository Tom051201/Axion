#include "MaterialImportModal.h"

#include "AxionEngine/Vendor/imgui/imgui.h"

#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionAssetPipeline/Source/AxMaterial.h"

namespace Axion {

	constexpr float inputFieldWidth = 200.0f;

	MaterialImportModal::MaterialImportModal(const char* name) : Modal(name) {}

	MaterialImportModal::~MaterialImportModal() {}

	void MaterialImportModal::close() {
		Modal::close();
		clearBuffers();
	}

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
			ImGui::InputText("##MatName_input", m_nameBuffer, sizeof(m_nameBuffer));


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
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::DragFloat("##tiling_drag", &m_tiling, 0.05f, 0.0f, 1.0f);


			// -- Use normal map --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Use a Normal map");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::Checkbox("##useNormal_check", &m_useNormalMap);


			// -- Use metalness map --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Use a Metalness map");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::Checkbox("##useMetalness_check", &m_useMetalnessMap);


			// -- Use roughness map --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Use a Roughness map");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::Checkbox("##useRoughness_check", &m_useRoughnessMap);


			// -- Use occlusion map --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Use an Occlusion map");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::Checkbox("##useOcclusion_check", &m_useOcclusionMap);

			// -- Albedo Map --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Albedo Map");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##AlbedoMapPath_input", m_albedoMapPathBuffer, sizeof(m_albedoMapPathBuffer));
			ImGui::SameLine();
			if (ImGui::Button("Browse##AlbedoMapFile_button")) {
				std::filesystem::path dir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "textures";
				std::string absPath = FileDialogs::openFile({ {"Axion Texture Asset", "*.axtex"} }, dir.string());
				if (!absPath.empty()) {
					strcpy_s(m_albedoMapPathBuffer, IM_ARRAYSIZE(m_albedoMapPathBuffer), absPath.c_str());
					m_albedoMapPathBuffer[IM_ARRAYSIZE(m_albedoMapPathBuffer) - 1] = '\0';
				}
			}

			// -- Normal Map --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Normal Map");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##NormalMapPath_input", m_normalMapPathBuffer, sizeof(m_normalMapPathBuffer));
			ImGui::SameLine();
			if (ImGui::Button("Browse##...File_button")) {
				std::filesystem::path dir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "textures";
				std::string absPath = FileDialogs::openFile({ {"Axion Texture Asset", "*.axtex"} }, dir.string());
				if (!absPath.empty()) {
					strcpy_s(m_normalMapPathBuffer, IM_ARRAYSIZE(m_normalMapPathBuffer), absPath.c_str());
					m_normalMapPathBuffer[IM_ARRAYSIZE(m_normalMapPathBuffer) - 1] = '\0';
				}
			}

			// -- Metalness Map --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Metalness Map");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##MetalnessMapPath_input", m_metalnessMapPathBuffer, sizeof(m_metalnessMapPathBuffer));
			ImGui::SameLine();
			if (ImGui::Button("Browse##MetalnessMapFile_button")) {
				std::filesystem::path dir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "textures";
				std::string absPath = FileDialogs::openFile({ {"Axion Texture Asset", "*.axtex"} }, dir.string());
				if (!absPath.empty()) {
					strcpy_s(m_metalnessMapPathBuffer, IM_ARRAYSIZE(m_metalnessMapPathBuffer), absPath.c_str());
					m_metalnessMapPathBuffer[IM_ARRAYSIZE(m_metalnessMapPathBuffer) - 1] = '\0';
				}
			}

			// -- Roughness Map --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Roughness Map");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##RoughnessMapPath_input", m_roughnessMapPathBuffer, sizeof(m_roughnessMapPathBuffer));
			ImGui::SameLine();
			if (ImGui::Button("Browse##RoughnessMapFile_button")) {
				std::filesystem::path dir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "textures";
				std::string absPath = FileDialogs::openFile({ {"Axion Texture Asset", "*.axtex"} }, dir.string());
				if (!absPath.empty()) {
					strcpy_s(m_roughnessMapPathBuffer, IM_ARRAYSIZE(m_roughnessMapPathBuffer), absPath.c_str());
					m_roughnessMapPathBuffer[IM_ARRAYSIZE(m_roughnessMapPathBuffer) - 1] = '\0';
				}
			}

			// -- Occlusion Map --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Occlusion Map");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##OcclusionMapPath_input", m_occlusionMapPathBuffer, sizeof(m_occlusionMapPathBuffer));
			ImGui::SameLine();
			if (ImGui::Button("Browse##OcclusionMapFile_button")) {
				std::filesystem::path dir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "textures";
				std::string absPath = FileDialogs::openFile({ {"Axion Texture Asset", "*.axtex"} }, dir.string());
				if (!absPath.empty()) {
					strcpy_s(m_occlusionMapPathBuffer, IM_ARRAYSIZE(m_occlusionMapPathBuffer), absPath.c_str());
					m_occlusionMapPathBuffer[IM_ARRAYSIZE(m_occlusionMapPathBuffer) - 1] = '\0';
				}
			}

			// -- Emissive Map --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Emissive Map");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##EmissiveMapPath_input", m_emissiveMapPathBuffer, sizeof(m_emissiveMapPathBuffer));
			ImGui::SameLine();
			if (ImGui::Button("Browse##EmissiveMapFile_button")) {
				std::filesystem::path dir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "textures";
				std::string absPath = FileDialogs::openFile({ {"Axion Texture Asset", "*.axtex"} }, dir.string());
				if (!absPath.empty()) {
					strcpy_s(m_emissiveMapPathBuffer, IM_ARRAYSIZE(m_emissiveMapPathBuffer), absPath.c_str());
					m_emissiveMapPathBuffer[IM_ARRAYSIZE(m_emissiveMapPathBuffer) - 1] = '\0';
				}
			}

			// -- Shader path --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Shader");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##MatShaderPath_input", m_sourcePathBuffer, sizeof(m_sourcePathBuffer));
			ImGui::SameLine();
			if (ImGui::Button("Browse##MatShaderFile_button")) {
				std::filesystem::path matDir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "shaders";
				std::string absPath = FileDialogs::openFile({ {"Axion Shader Asset", "*.axshader"} }, matDir.string());
				if (!absPath.empty()) {
					strcpy_s(m_sourcePathBuffer, IM_ARRAYSIZE(m_sourcePathBuffer), absPath.c_str());
					m_sourcePathBuffer[IM_ARRAYSIZE(m_sourcePathBuffer) - 1] = '\0';
				}
			}


			// -- Output path --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Output Location");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##MatOutputPath_input", m_outputPathBuffer, sizeof(m_outputPathBuffer));
			ImGui::SameLine();
			if (ImGui::Button("Browse##MatOutputDir_button")) {
				std::filesystem::path matDir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "materials";
				std::string absPath = FileDialogs::openFolder(matDir.string());
				if (!absPath.empty()) {
					strcpy_s(m_outputPathBuffer, IM_ARRAYSIZE(m_outputPathBuffer), absPath.c_str());
					m_outputPathBuffer[IM_ARRAYSIZE(m_outputPathBuffer) - 1] = '\0';
				}
			}

			ImGui::EndTable();

			// -- Validate input --
			std::filesystem::path sourceFilePath = std::string(m_sourcePathBuffer);
			bool validSource = std::filesystem::exists(sourceFilePath);

			std::filesystem::path outputDirPath = std::string(m_outputPathBuffer);
			bool validOutputPath = std::filesystem::exists(outputDirPath);
			bool validOutputFile = !std::filesystem::exists(outputDirPath / (std::string(m_nameBuffer) + ".axmat"));

			bool disabled = (
				strlen(m_nameBuffer) == 0 ||
				strlen(m_sourcePathBuffer) == 0 ||
				strlen(m_outputPathBuffer) == 0 ||
				!validSource ||
				!validOutputPath ||
				!validOutputFile
			);

			ImGui::Separator();
			ImGui::BeginDisabled(disabled);
			if (ImGui::Button("Create")) {
				std::filesystem::path outDir = std::string(m_outputPathBuffer);
				std::filesystem::path outFile = outDir / (std::string(m_nameBuffer) + ".axmat");

				AAP::MaterialAssetData data;
				data.name = m_nameBuffer;
				data.shaderAsset = AssetManager::getRelativeToAssets(std::string(m_sourcePathBuffer));

				MaterialProperties prop;
				prop.albedoColor = m_albedoColor;
				prop.metalness = m_metalness;
				prop.roughness = m_roughness;
				prop.emissionStrength = m_emission;
				prop.tiling = m_tiling;
				prop.useNormalMap = m_useNormalMap ? 1.0f : 0.0f;
				prop.useMetalnessMap = m_useMetalnessMap ? 1.0f : 0.0f;
				prop.useRoughnessMap = m_useRoughnessMap ? 1.0f : 0.0f;
				prop.useOcclusionMap = m_useOcclusionMap ? 1.0f : 0.0f;

				data.properties = prop;

				if (strlen(m_albedoMapPathBuffer) != 0 && std::filesystem::exists(m_albedoMapPathBuffer)) {
					data.textures[TextureSlot::Albedo] = AssetManager::getRelativeToAssets(std::string(m_albedoMapPathBuffer));
				}

				if (strlen(m_normalMapPathBuffer) != 0 && std::filesystem::exists(m_normalMapPathBuffer)) {
					data.textures[TextureSlot::Normal] = AssetManager::getRelativeToAssets(std::string(m_normalMapPathBuffer));
				}

				if (strlen(m_metalnessMapPathBuffer) != 0 && std::filesystem::exists(m_metalnessMapPathBuffer)) {
					data.textures[TextureSlot::Metalness] = AssetManager::getRelativeToAssets(std::string(m_metalnessMapPathBuffer));
				}

				if (strlen(m_roughnessMapPathBuffer) != 0 && std::filesystem::exists(m_roughnessMapPathBuffer)) {
					data.textures[TextureSlot::Roughness] = AssetManager::getRelativeToAssets(std::string(m_roughnessMapPathBuffer));
				}

				if (strlen(m_occlusionMapPathBuffer) != 0 && std::filesystem::exists(m_occlusionMapPathBuffer)) {
					data.textures[TextureSlot::Occlusion] = AssetManager::getRelativeToAssets(std::string(m_occlusionMapPathBuffer));
				}

				if (strlen(m_emissiveMapPathBuffer) != 0 && std::filesystem::exists(m_emissiveMapPathBuffer)) {
					data.textures[TextureSlot::Emissive] = AssetManager::getRelativeToAssets(std::string(m_emissiveMapPathBuffer));
				}


				AAP::MaterialParser::createAxMatFile(data, outFile.string());
				close();
			}
			ImGui::EndDisabled();
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) {
				close();
			}

		}

	}

	void MaterialImportModal::clearBuffers() {
		m_nameBuffer[0] = '\0';
		m_sourcePathBuffer[0] = '\0';
		m_outputPathBuffer[0] = '\0';
		m_albedoColor = Vec4::one();
		m_metalness = 0.0f;
		m_roughness = 0.0f;
		m_emission = 0.0f;
		m_tiling = 0.0f;
		m_useNormalMap = false;
		m_useMetalnessMap = false;
		m_useRoughnessMap = false;
		m_useOcclusionMap = false;
		m_albedoMapPathBuffer[0] = '\0';
		m_normalMapPathBuffer[0] = '\0';
		m_metalnessMapPathBuffer[0] = '\0';
		m_roughnessMapPathBuffer[0] = '\0';
		m_occlusionMapPathBuffer[0] = '\0';
		m_emissiveMapPathBuffer[0] = '\0';
	}

}
