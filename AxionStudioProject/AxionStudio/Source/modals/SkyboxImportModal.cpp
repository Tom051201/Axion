#include "SkyboxImportModal.h"

#include "AxionEngine/Vendor/imgui/imgui.h"
#include "AxionEngine/Vendor/imgui/misc/cpp/imgui_stdlib.h"

#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/AssetVersions.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionAssetPipeline/Source/AxSkybox.h"

namespace Axion {

	constexpr float inputFieldWidth = 200.0f;

	void SkyboxImportModal::renderContent() {

		ImGui::SeparatorText("Import Skybox Asset");
		ImGui::Spacing();

		if (ImGui::BeginTable("##ImportSkyboxTable", 2, ImGuiTableFlags_BordersInnerV)) {
			ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 120.0f);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

			// -- Name --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Name");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##SkyboxName_input", &m_name);


			// -- Texture Cube path --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Texture Cube");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##SkyboxSourcePath_input", &m_texturePath);
			ImGui::SameLine();
			if (ImGui::Button("Browse...##SkyboxSourceFile_button")) {
				std::filesystem::path dir = ProjectManager::getProject()->getAssetsPath() / "textures";
				std::filesystem::path absPath;
				if (std::filesystem::exists(dir)) {
					absPath = FileDialogs::openFile({ {"Axion Texture File", "*.axtcube"} }, dir);
				}
				else {
					absPath = FileDialogs::openFile({ {"Axion Texture File", "*.axtcube"} }, ProjectManager::getProject()->getAssetsPath());
				}
				if (!absPath.empty()) m_texturePath = absPath.string();
			}


			// -- Pipeline path --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Pipeline");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##SkyboxPipelinePath_input", &m_pipelinePath);
			ImGui::SameLine();
			if (ImGui::Button("Browse...##SkyboxPipelineFile_button")) {
				std::filesystem::path dir = ProjectManager::getProject()->getAssetsPath() / "pipelines";
				std::filesystem::path absPath;
				if (std::filesystem::exists(dir)) {
					absPath = FileDialogs::openFile({ {"Axion Pipeline Asset", "*.axpso"} }, dir);
				}
				else {
					absPath = FileDialogs::openFile({ {"Axion Pipeline Asset", "*.axpso"} }, ProjectManager::getProject()->getAssetsPath());
				}
				if (!absPath.empty()) m_pipelinePath = absPath.string();
			}


			// -- Output path --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Output Location");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##SkyboxOutputPath_input", &m_outputPath);
			ImGui::SameLine();
			if (ImGui::Button("Browse...##SkyboxOutputDir_button")) {
				std::filesystem::path dir = ProjectManager::getProject()->getAssetsPath() / "skybox";
				std::filesystem::path absPath;
				if (std::filesystem::exists(dir)) {
					absPath = FileDialogs::openFolder(dir);
				}
				else {
					absPath = FileDialogs::openFolder(ProjectManager::getProject()->getAssetsPath());
				}
				if (!absPath.empty()) m_outputPath = absPath.string();
			}

			ImGui::EndTable();

			// -- Validate input --
			std::string finalName = m_name + ".axsky";
			std::filesystem::path finalPath = std::filesystem::path(m_outputPath) / finalName;

			bool hasPipeline = !m_pipelinePath.empty();
			bool pipelineExists = true;
			bool pipelineIsFile = true;
			if (hasPipeline) {
				pipelineExists = std::filesystem::exists(m_pipelinePath);
				pipelineIsFile = std::filesystem::is_regular_file(m_pipelinePath);
			}

			bool textureExists = std::filesystem::exists(m_texturePath);
			bool textureIsFile = std::filesystem::is_regular_file(m_texturePath);
			bool outputExists = std::filesystem::exists(m_outputPath);
			bool outputIsDirectory = std::filesystem::is_directory(m_outputPath);
			bool invalidOutFileName = std::filesystem::exists(finalPath);

			bool disabled = (
				m_name.empty() ||
				m_texturePath.empty() ||
				m_outputPath.empty() ||
				!pipelineExists ||
				!pipelineIsFile ||
				!textureExists ||
				!textureIsFile ||
				!outputExists ||
				!outputIsDirectory ||
				invalidOutFileName
			);

			if (disabled) {
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 50, 50, 255));
				if (m_name.empty()) ImGui::Text("No Name is set.");
				else if (m_texturePath.empty()) ImGui::Text("No texture file is set.");
				else if (m_outputPath.empty()) ImGui::Text("No output directory is set.");
				else if (!pipelineExists) ImGui::Text("Pipeline file does not exist.");
				else if (!pipelineIsFile) ImGui::Text("Pipeline is not a file.");
				else if (!textureExists) ImGui::Text("Texture file does not exist.");
				else if (!textureIsFile) ImGui::Text("Texture is not a file.");
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

				AAP::SkyboxAssetData data;
				data.uuid = newAssetUUID;
				data.name = m_name;
				data.textureCubePath = AssetManager::getRelativeToAssets(m_texturePath);
				if (!m_pipelinePath.empty()) {
					data.pipelinePath = AssetManager::getRelativeToAssets(m_pipelinePath);
				}

				AAP::SkyboxParser::createTextFile(data, finalPath);

				AssetMetadata metadata;
				metadata.handle = newAssetUUID;
				metadata.type = AssetType::Skybox;
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
			std::string versionText = "v" + std::to_string(ASSET_VERSION_SKYBOX);
			float textWidth = ImGui::CalcTextSize(versionText.c_str()).x;
			float windowWidth = ImGui::GetWindowWidth();
			ImGui::SameLine(windowWidth - textWidth - ImGui::GetStyle().WindowPadding.x);
			ImGui::TextDisabled("%s", versionText.c_str());

		}

	}

	void SkyboxImportModal::resetInputs() {
		m_name.clear();
		m_texturePath.clear();
		m_pipelinePath.clear();
		m_outputPath.clear();
	}

}
