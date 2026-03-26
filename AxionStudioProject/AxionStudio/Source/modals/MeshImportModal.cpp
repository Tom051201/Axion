#include "MeshImportModal.h"

#include "AxionEngine/Vendor/imgui/imgui.h"
#include "AxionEngine/Vendor/imgui/misc/cpp/imgui_stdlib.h"

#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/AssetVersions.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionAssetPipeline/Source/AxMesh.h"

namespace Axion {

	constexpr float inputFieldWidth = 200.0f;

	void MeshImportModal::presetFromFile(const std::filesystem::path& sourceFile) {
		resetInputs();

		// -- Source Path --
		m_sourcePath = sourceFile.string();

		// -- Default output folder --
		auto meshDir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "meshes";
		m_outputPath = meshDir.string();

		// -- Default name --
		m_name = sourceFile.stem().string();

		// -- Type --
		std::string typeStr = sourceFile.extension().string();
		std::transform(typeStr.begin(), typeStr.end(), typeStr.begin(), [](unsigned char c) { return std::tolower(c); });
		if (typeStr == ".obj") m_importType = 0;
		else AX_CORE_LOG_WARN("Unable to identify automatically type of mesh");
	}

	void MeshImportModal::renderContent() {
		
		ImGui::SeparatorText("Import Mesh Asset");
		ImGui::Spacing();

		if (ImGui::BeginTable("##ImportMeshTable", 2, ImGuiTableFlags_BordersInnerV)) {
			ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 120.0f);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

			// -- Name --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Name");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##MeshName_input", &m_name);


			// -- Type --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Type");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::Combo("##MeshType_combo", &m_importType, m_types, IM_ARRAYSIZE(m_types));


			// -- Source path --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Source File");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##MeshSourcePath_input", &m_sourcePath);
			ImGui::SameLine();
			if (ImGui::Button("Browse...##MeshSourceFile_button")) {
				std::filesystem::path meshDir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "meshes";
				std::string absPath;
				if (std::filesystem::exists(meshDir)) {
					absPath = FileDialogs::openFile({ {"OBJ File", "*.obj"} }, meshDir.string());
				}
				else {
					absPath = FileDialogs::openFile({ {"OBJ File", "*.obj"} }, ProjectManager::getProject()->getAssetsPath());
				}
				if (!absPath.empty()) m_sourcePath = absPath;
			}


			// -- Output path --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Output Location");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##MeshOutputPath_input", &m_outputPath);
			ImGui::SameLine();
			if (ImGui::Button("Browse...##MeshOutputDir_button")) {
				std::filesystem::path meshDir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "meshes";
				std::string absPath;
				if (std::filesystem::exists(meshDir)) {
					absPath = FileDialogs::openFolder(meshDir.string());
				}
				else {
					absPath = FileDialogs::openFolder(ProjectManager::getProject()->getAssetsPath());
				}
				if (!absPath.empty()) m_outputPath = absPath;
			}

			ImGui::EndTable();

			// -- Validate input --
			std::string finalName = m_name + ".axmesh";
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

				AAP::MeshAssetData data;
				data.uuid = newAssetUUID;
				data.name = m_name;
				data.fileFormat = m_types[m_importType];
				data.filePath = AssetManager::getRelativeToAssets(m_sourcePath);

				AAP::MeshParser::createTextFile(data, finalPath.string());

				AssetMetadata metadata;
				metadata.handle = newAssetUUID;
				metadata.type = AssetType::Mesh;
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
			std::string versionText = "v" + std::to_string(ASSET_VERSION_MESH);
			float textWidth = ImGui::CalcTextSize(versionText.c_str()).x;
			float windowWidth = ImGui::GetWindowWidth();
			ImGui::SameLine(windowWidth - textWidth - ImGui::GetStyle().WindowPadding.x);
			ImGui::TextDisabled("%s", versionText.c_str());

		}

	}

	void MeshImportModal::resetInputs() {
		m_name.clear();
		m_sourcePath.clear();
		m_outputPath.clear();
		m_importType = 0;
	}

}
