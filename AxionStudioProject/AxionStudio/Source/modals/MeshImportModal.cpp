#include "MeshImportModal.h"

#include "AxionEngine/Vendor/imgui/imgui.h"

#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionAssetPipeline/Source/AxMesh.h"

namespace Axion {

	constexpr float inputFieldWidth = 200.0f;

	MeshImportModal::MeshImportModal(const char* name) : Modal(name) {}

	MeshImportModal::~MeshImportModal() {}

	void MeshImportModal::close() {
		Modal::close();
		clearBuffers();
	}

	void MeshImportModal::presetFromFile(const std::filesystem::path& sourceFile) {
		clearBuffers();

		// -- Source Path --
		std::string abs = sourceFile.string();
		strcpy_s(m_sourcePathBuffer, IM_ARRAYSIZE(m_sourcePathBuffer), abs.c_str());

		// -- Default output folder --
		auto meshDir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "meshes";
		strcpy_s(m_outputPathBuffer, IM_ARRAYSIZE(m_outputPathBuffer), meshDir.string().c_str());

		// -- Default name --
		std::string name = sourceFile.stem().string();
		strcpy_s(m_nameBuffer, IM_ARRAYSIZE(m_nameBuffer), name.c_str());

		// -- Type --
		std::string typeStr = sourceFile.extension().string();
		if (typeStr == ".obj") {
			m_importType = 0;
		}
		else {
			AX_CORE_LOG_WARN("Unable to identify automatically type of mesh");
		}
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
			ImGui::InputText("##MeshName_input", m_nameBuffer, sizeof(m_nameBuffer));


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
			ImGui::InputText("##MeshSourcePath_input", m_sourcePathBuffer, sizeof(m_sourcePathBuffer));
			ImGui::SameLine();
			if (ImGui::Button("Browse##MeshSourceFile_button")) {
				std::filesystem::path meshDir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "meshes";
				std::string absPath = FileDialogs::openFile({ {"OBJ File", "*.obj"} }, meshDir.string());
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
			ImGui::InputText("##MeshOutputPath_input", m_outputPathBuffer, sizeof(m_outputPathBuffer));
			ImGui::SameLine();
			if (ImGui::Button("Browse##MeshOutputDir_button")) {
				std::filesystem::path meshDir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "meshes";
				std::string absPath = FileDialogs::openFolder(meshDir.string());
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
			bool validOutputFile = !std::filesystem::exists(outputDirPath / (std::string(m_nameBuffer) + ".axmesh"));

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
				std::filesystem::path outFile = outDir / (std::string(m_nameBuffer) + ".axmesh");

				AAP::MeshAssetData data;
				data.name = m_nameBuffer;
				data.fileFormat = m_types[m_importType];
				data.filePath = AssetManager::getRelativeToAssets(std::string(m_sourcePathBuffer));

				AAP::MeshParser::createAxMeshFile(data, outFile.string());
				close();
			}
			ImGui::EndDisabled();
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) {
				close();
			}
		}

	}

	void MeshImportModal::clearBuffers() {
		m_nameBuffer[0] = '\0';
		m_sourcePathBuffer[0] = '\0';
		m_outputPathBuffer[0] = '\0';
		m_importType = 0;
	}

}
