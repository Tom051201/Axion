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


			// -- Color format --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Color Format");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::Combo("##ColorFormat_combo", &m_colorFormatIndex, m_colorFormatsNames, IM_ARRAYSIZE(m_colorFormatsNames));


			// -- Depth stencil format --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Depth Stencil");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::Combo("##DepthFormat_combo", &m_depthFormatIndex, m_depthFormatsNames, IM_ARRAYSIZE(m_depthFormatsNames));


			// -- Depth test --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Depth Test");
			ImGui::TableSetColumnIndex(1);
			ImGui::Checkbox("##DepthTest_check", &m_depthTest);


			// -- Depth test --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Depth Write");
			ImGui::TableSetColumnIndex(1);
			ImGui::Checkbox("##DepthWrite_check", &m_depthWrite);


			// -- Depth compare --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Depth Compare");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::Combo("##DepthCompare_combo", &m_depthCompareIndex, m_depthCompareNames, IM_ARRAYSIZE(m_depthCompareNames));


			// -- Stencil --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Stencil");
			ImGui::TableSetColumnIndex(1);
			ImGui::Checkbox("##Stencil_check", &m_stencilEnabled);


			// -- Sample count --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Sample Count");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputInt("##SampleCount_input", &m_sampleCount);
			if (m_sampleCount < 1) m_sampleCount = 1;


			// -- Cull Mode --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Cull Mode");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::Combo("##CullMode_combo", &m_cullModeIndex, m_cullModesNames, IM_ARRAYSIZE(m_cullModesNames));


			// -- Primitive topology --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Topology");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::Combo("##Topology_combo", &m_topologyIndex, m_topologiesNames, IM_ARRAYSIZE(m_topologiesNames));

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Max Batch Textures");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputInt("##BatchTexturesCount_input", &m_batchTexturesCount);
			if (m_batchTexturesCount < 1) m_batchTexturesCount = 1;


			// -- Buffer layout --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Buffer Layout");
			ImGui::TableSetColumnIndex(1);
			
			if (ImGui::BeginTable("##BufferLayoutTable", 3, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg)) {
				ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 100.0f);
				ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 80.0f);
				ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 20.0f);

				// Existing elements
				for (size_t i = 0; i < m_bufferElements.size(); i++) {
					auto& element = m_bufferElements[i];
					ImGui::TableNextRow();

					// -- Name --
					ImGui::TableSetColumnIndex(0);
					char buf[64];
					strcpy_s(buf, element.name.c_str());
					ImGui::SetNextItemWidth(100.0f);
					if (ImGui::InputText(("##ElemName" + std::to_string(i)).c_str(), buf, sizeof(buf))) {
						element.name = buf;
					}

					// -- Type --
					ImGui::TableSetColumnIndex(1);
					int typeIndex = static_cast<int>(element.type);
					ImGui::SetNextItemWidth(80.0f);
					if (ImGui::Combo(("##ElemType" + std::to_string(i)).c_str(), &typeIndex, m_shaderDataTypeNames, IM_ARRAYSIZE(m_shaderDataTypeNames))) {
						element.type = static_cast<ShaderDataType>(typeIndex);
						element.size = ShaderDataTypeSize(element.type);
					}

					// -- Remove --
					ImGui::TableSetColumnIndex(2);
					if (ImGui::Button(("X##RemoveElem" + std::to_string(i)).c_str())) {
						m_bufferElements.erase(m_bufferElements.begin() + i);
						i--;
					}
				}

				ImGui::EndTable();
			}
			if (ImGui::Button("Add Attribute")) {
				m_bufferElements.emplace_back("Attribute", ShaderDataType::Float3);
			}


			// -- Source path --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Separator();
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
				spec.colorFormat = m_colorFormats[m_colorFormatIndex];
				spec.depthStencilFormat = m_depthFormats[m_depthFormatIndex];
				spec.depthTest = m_depthTest;
				spec.depthWrite = m_depthWrite;
				spec.depthFunction = m_depthCompares[m_depthCompareIndex];
				spec.stencilEnabled = m_stencilEnabled;
				spec.sampleCount = m_sampleCount;
				spec.cullMode = m_cullModes[m_cullModeIndex];
				spec.topology = m_topologies[m_topologyIndex];
				spec.batchTextures = m_batchTexturesCount;
				BufferLayout layout(m_bufferElements);
				spec.vertexLayout = layout;

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
		m_colorFormatIndex = 1;
		m_depthFormatIndex = 2;
		m_depthTest = true;
		m_depthWrite = true;
		m_depthCompareIndex = 1;
		m_stencilEnabled = false;
		m_sampleCount = 1;
		m_cullModeIndex = 2;
		m_topologyIndex = 3;
		m_batchTexturesCount = 1;
		m_bufferElements.clear();
	}

}
