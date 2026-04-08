#include "ShaderImportModal.h"

#include "AxionEngine/Vendor/imgui/imgui.h"
#include "AxionEngine/Vendor/imgui/misc/cpp/imgui_stdlib.h"

#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/AssetVersions.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionAssetPipeline/Source/AxShader.h"

namespace Axion {

	constexpr float inputFieldWidth = 200.0f;

	void ShaderImportModal::renderContent() {

		ImGui::SeparatorText("Import Shader Asset");
		ImGui::Spacing();

		if (ImGui::BeginTable("##ImportShaderTable", 2, ImGuiTableFlags_BordersInnerV)) {
			ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 120.0f);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

			// -- Name --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Name");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##ShaderName_input", &m_name);


			// -- Format --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Format");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::Combo("##ShaderFormat_combo", &m_formatIndex, m_formats, IM_ARRAYSIZE(m_formats));


			// -- Batch textures --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Batch Textures");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputInt("##BatchTexturesCount_input", &m_batchTexturesCount);
			if (m_batchTexturesCount < 1) m_batchTexturesCount = 1;


			// -- Source path --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Source File");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##ShaderSourcePath_input", &m_sourcePath);
			ImGui::SameLine();
			if (ImGui::Button("Browse...##ShaderSourceFile_button")) {
				std::filesystem::path shaderDir = ProjectManager::getProject()->getAssetsPath() / "shaders";
				std::filesystem::path absPath;
				if (std::filesystem::exists(shaderDir)) {
					absPath = FileDialogs::openFile({ {"Shader Source", "*.hlsl;*.glsl"} }, shaderDir);
				}
				else {
					absPath = FileDialogs::openFile({ {"Shader Source", "*.hlsl;*.glsl"} }, ProjectManager::getProject()->getAssetsPath());
				}
				if (!absPath.empty()) m_sourcePath = absPath.string();
			}


			// -- Output path --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Output Location");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##ShaderOutputPath_input", &m_outputPath);
			ImGui::SameLine();
			if (ImGui::Button("Browse...##ShaderOutputDir_button")) {
				std::filesystem::path shaderDir = ProjectManager::getProject()->getAssetsPath() / "shaders";
				std::filesystem::path absPath;
				if (std::filesystem::exists(shaderDir)) {
					absPath = FileDialogs::openFolder(shaderDir);
				}
				else {
					absPath = FileDialogs::openFolder(ProjectManager::getProject()->getAssetsPath());
				}
				if (!absPath.empty()) m_outputPath = absPath.string();
			}

			ImGui::EndTable();

			// -- Validate input --
			std::string finalName = m_name + ".axshader";
			std::filesystem::path finalPath = std::filesystem::path(m_outputPath) / finalName;

			bool sourceExists = std::filesystem::exists(m_sourcePath);
			bool sourceIsFile = std::filesystem::is_regular_file(m_sourcePath);
			bool outputExists = std::filesystem::exists(m_outputPath);
			bool outputIsDirectory = std::filesystem::is_directory(m_outputPath);
			bool invalidOutFileName = std::filesystem::exists(finalPath);

			bool disabled = (
				m_name.empty() ||
				m_sourcePath.empty() ||
				m_outputPath.empty() ||
				!sourceExists ||
				!sourceIsFile ||
				!outputExists ||
				!outputIsDirectory ||
				invalidOutFileName
			);

			if (disabled) {
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 50, 50, 255));
				if (m_name.empty()) ImGui::Text("No Name is set.");
				else if (m_sourcePath.empty()) ImGui::Text("No source file is set.");
				else if (m_outputPath.empty()) ImGui::Text("No output directory is set.");
				else if (!sourceExists) ImGui::Text("Source file does not exist.");
				else if (!sourceIsFile) ImGui::Text("Source is not a file.");
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

				ShaderSpecification spec = {};
				spec.name = m_name;
				spec.batchTextures = m_batchTexturesCount;

				UUID newAssetUUID = UUID::generate();

				AAP::ShaderAssetData data;
				data.uuid = newAssetUUID;
				data.filePath = AssetManager::getRelativeToAssets(m_sourcePath);
				data.fileFormat = m_formats[m_formatIndex];
				data.spec = spec;

				AAP::ShaderParser::createTextFile(data, finalPath);

				AssetMetadata metadata;
				metadata.handle = newAssetUUID;
				metadata.type = AssetType::Shader;
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
			std::string versionText = "v" + std::to_string(ASSET_VERSION_SHADER);
			float textWidth = ImGui::CalcTextSize(versionText.c_str()).x;
			float windowWidth = ImGui::GetWindowWidth();
			ImGui::SameLine(windowWidth - textWidth - ImGui::GetStyle().WindowPadding.x);
			ImGui::TextDisabled("%s", versionText.c_str());

		}
	}
	
	void ShaderImportModal::resetInputs() {
		m_name.clear();
		m_sourcePath.clear();
		m_outputPath.clear();
		m_formatIndex = 0;
		m_batchTexturesCount = 1;
	}

}
