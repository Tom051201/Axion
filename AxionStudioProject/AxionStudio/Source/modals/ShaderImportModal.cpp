#include "ShaderImportModal.h"

#include "AxionEngine/Vendor/imgui/imgui.h"

#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionAssetPipeline/Source/AxShader.h"

namespace Axion {

	constexpr float inputFieldWidth = 200.0f;

	ShaderImportModal::ShaderImportModal(const char* name) : Modal(name) {}

	ShaderImportModal::~ShaderImportModal() {}

	void ShaderImportModal::close() {
		Modal::close();
		clearBuffers();
	}

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
			ImGui::InputText("##ShaderName_input", m_nameBuffer, sizeof(m_nameBuffer));


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
			ImGui::Separator();
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
			ImGui::InputText("##ShaderSourcePath_input", m_sourcePathBuffer, sizeof(m_sourcePathBuffer));
			ImGui::SameLine();
			if (ImGui::Button("Browse##ShaderSourceFile_button")) {
				std::filesystem::path shaderDir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "shaders";
				std::string absPath = FileDialogs::openFile({ {"Shader Source", "*.hlsl;*.glsl"} }, shaderDir.string());
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
			ImGui::InputText("##ShaderOutputPath_input", m_outputPathBuffer, sizeof(m_outputPathBuffer));
			ImGui::SameLine();
			if (ImGui::Button("Browse##ShaderOutputDir_button")) {
				std::filesystem::path shaderDir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "shaders";
				std::string absPath = FileDialogs::openFolder(shaderDir.string());
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
			bool validOutputFile = !std::filesystem::exists(outputDirPath / (std::string(m_nameBuffer) + ".axshader"));

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
				std::filesystem::path outFile = outDir / (std::string(m_nameBuffer) + ".axshader");

				ShaderSpecification spec = {};
				spec.name = m_nameBuffer;
				spec.batchTextures = m_batchTexturesCount;

				AAP::ShaderAssetData data;
				data.filePath = AssetManager::getRelativeToAssets(std::string(m_sourcePathBuffer));
				data.fileFormat = m_formats[m_formatIndex];
				data.spec = spec;

				AAP::ShaderParser::createAxShaderFile(data, outFile.string());
				close();
			}
			ImGui::EndDisabled();
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) {
				close();
			}

		}
	}
	
	void ShaderImportModal::clearBuffers() {
		m_nameBuffer[0] = '\0';
		m_sourcePathBuffer[0] = '\0';
		m_outputPathBuffer[0] = '\0';
		m_formatIndex = 0;
		m_batchTexturesCount = 1;
	}

}
