#include "TextureCubeImportModal.h"

#include "AxionEngine/Vendor/imgui/imgui.h"
#include "AxionEngine/Vendor/imgui/misc/cpp/imgui_stdlib.h"

#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/AssetVersions.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionAssetPipeline/Source/AxTextureCube.h"

namespace Axion {

	constexpr float inputFieldWidth = 200.0f;

	void TextureCubeImportModal::renderContent() {
		ImGui::SeparatorText("Import TextureCube Asset");
		ImGui::Spacing();

		if (ImGui::BeginTable("##ImportTexCubeTable", 2, ImGuiTableFlags_BordersInnerV)) {
			ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 120.0f);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);


			// -- Name --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Name");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##TexCubeName_input", &m_name);


			// -- Type --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Type");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::Combo("##TexCubeType_combo", &m_importType, m_types, IM_ARRAYSIZE(m_types));


			// -- Source path --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Source File");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##TexCubeSourcePath_input", &m_sourcePath);
			ImGui::SameLine();
			if (ImGui::Button("Browse##TexCubeSourceFile_button")) {
				std::filesystem::path dir = ProjectManager::getProject()->getAssetsPath() / "textures";
				std::filesystem::path absPath;
				if (std::filesystem::exists(dir)) {
					absPath = FileDialogs::openFile({ {"PNG File", "*.png"} }, dir); // TODO: add hdr later
				}
				else {
					absPath = FileDialogs::openFile({ {"PNG File", "*.png"} }, ProjectManager::getProject()->getAssetsPath()); // TODO: add hdr later
				}
				if (!absPath.empty()) m_sourcePath = absPath.string();
			}


			// -- Output path --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Output Location");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##TexCubeOutputPath_input", &m_outputPath);
			ImGui::SameLine();
			if (ImGui::Button("Browse##TexCubeOutputDir_button")) {
				std::filesystem::path dir = ProjectManager::getProject()->getAssetsPath() / "textures";
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
			std::string finalName = m_name + ".axtcube";
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
				UUID newAssetUUID = UUID::generate();

				AAP::TextureCubeAssetData data;
				data.uuid = newAssetUUID;
				data.name = m_name;
				data.fileFormat = m_types[m_importType];
				data.filePath = AssetManager::getRelativeToAssets(m_sourcePath);

				AAP::TextureCubeParser::createTextFile(data, finalPath);

				AssetMetadata metadata;
				metadata.handle = newAssetUUID;
				metadata.type = AssetType::TextureCube;
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
			std::string versionText = "v" + std::to_string(ASSET_VERSION_TEXTURE_CUBE);
			float textWidth = ImGui::CalcTextSize(versionText.c_str()).x;
			float windowWidth = ImGui::GetWindowWidth();
			ImGui::SameLine(windowWidth - textWidth - ImGui::GetStyle().WindowPadding.x);
			ImGui::TextDisabled("%s", versionText.c_str());

		}
	}

	void TextureCubeImportModal::resetInputs() {
		m_name.clear();
		m_sourcePath.clear();
		m_outputPath.clear();
		m_importType = 0;
	}

}
