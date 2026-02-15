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
	}

}
