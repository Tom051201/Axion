#include "PipelineImportModal.h"

#include "AxionEngine/Vendor/imgui/imgui.h"
#include "AxionEngine/Vendor/imgui/misc/cpp/imgui_stdlib.h"

#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionAssetPipeline/Source/AxPipeline.h"

namespace Axion {

	constexpr float inputFieldWidth = 200.0f;

	void PipelineImportModal::renderContent() {

		ImGui::SeparatorText("Import Pipeline Asset");
		ImGui::Spacing();

		if (ImGui::BeginTable("##ImportPipelineTable", 2, ImGuiTableFlags_BordersInnerV)) {
			ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 120.0f);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

			// -- Name --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Name");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##PipelineName_input", &m_name);


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


			// -- Depth write --
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


			// -- Num render targets --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Render Targets");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputInt("##RenderTargetsCount_input", &m_renderTargetsCount);
			if (m_renderTargetsCount < 0) m_renderTargetsCount = 0;


			// -- Buffer layout --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Buffer Layout");
			ImGui::TableSetColumnIndex(1);

			int elementToRemove = -1;

			if (ImGui::BeginTable("##BufferLayoutTable", 5, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg)) {
				ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 100.0f);
				ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 80.0f);
				ImGui::TableSetupColumn("Norm", ImGuiTableColumnFlags_WidthFixed, 40.0f);
				ImGui::TableSetupColumn("Inst", ImGuiTableColumnFlags_WidthFixed, 40.0f);
				ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 25.0f);

				// Existing elements
				for (size_t i = 0; i < m_bufferElements.size(); i++) {
					auto& element = m_bufferElements[i];
					ImGui::PushID((int)i);
					ImGui::TableNextRow();

					// -- Name --
					ImGui::TableSetColumnIndex(0);
					ImGui::SetNextItemWidth(100.0f);
					ImGui::InputText("##ElemName", &element.name);

					// -- Type --
					ImGui::TableSetColumnIndex(1);
					int typeIndex = static_cast<int>(element.type);
					ImGui::SetNextItemWidth(80.0f);
					if (ImGui::Combo("##ElemType", &typeIndex, m_shaderDataTypeNames, IM_ARRAYSIZE(m_shaderDataTypeNames))) {
						element.type = static_cast<ShaderDataType>(typeIndex);
						element.size = ShaderDataTypeSize(element.type);
					}

					// -- Normalized --
					ImGui::TableSetColumnIndex(2);
					ImGui::Checkbox("##ElemNorm", &element.normalized);

					// -- Instanced --
					ImGui::TableSetColumnIndex(3);
					ImGui::Checkbox("##ElemInst", &element.instanced);

					// -- Remove --
					ImGui::TableSetColumnIndex(4);
					if (ImGui::Button(("X##RemoveElem" + std::to_string(i)).c_str())) {
						elementToRemove = static_cast<int>(i);
					}

					ImGui::PopID();
				}

				ImGui::EndTable();
			}

			if (elementToRemove >= 0) {
				m_bufferElements.erase(m_bufferElements.begin() + elementToRemove);
			}

			if (ImGui::Button("Add Attribute")) {
				m_bufferElements.emplace_back("Attribute", ShaderDataType::Float3);
			}


			// -- Shader path --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Separator();
			ImGui::Text("Shader File");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##ShaderPath_input", &m_shaderPath);
			ImGui::SameLine();
			if (ImGui::Button("Browse...##ShaderFile_button")) {
				std::filesystem::path shaderDir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "shaders";
				std::string absPath;
				if (std::filesystem::exists(shaderDir)) {
					absPath = FileDialogs::openFile({ {"Shader Asset", "*.axshader"} }, shaderDir.string());
				}
				else {
					absPath = FileDialogs::openFile({ {"Shader Asset", "*.axshader"} }, ProjectManager::getProject()->getAssetsPath());
				}
				if (!absPath.empty()) m_shaderPath = absPath;
			}


			// -- Output path --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Output Location");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##PipelineOutputPath_input", &m_outputPath);
			ImGui::SameLine();
			if (ImGui::Button("Browse...##PipelineOutputDir_button")) {
				std::filesystem::path pipelineDir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "pipelines";
				std::string absPath;
				if (std::filesystem::exists(pipelineDir)) {
					absPath = FileDialogs::openFolder(pipelineDir.string());
				}
				else {
					absPath = FileDialogs::openFolder(ProjectManager::getProject()->getAssetsPath());
				}
				if (!absPath.empty()) m_outputPath = absPath;
			}

			ImGui::EndTable();

			// -- Validate input --
			std::string finalName = m_name + ".axpso";
			std::filesystem::path finalPath = std::filesystem::path(m_outputPath) / finalName;

			bool shaderExists = std::filesystem::exists(m_shaderPath);
			bool shaderIsFile = std::filesystem::is_regular_file(m_shaderPath);
			bool outputExists = std::filesystem::exists(m_outputPath);
			bool outputIsDirectory = std::filesystem::is_directory(m_outputPath);
			bool invalidOutFileName = std::filesystem::exists(finalPath);

			bool disabled = (
				m_name.empty() ||
				m_shaderPath.empty() ||
				m_outputPath.empty() ||
				!shaderExists ||
				!shaderIsFile ||
				!outputExists ||
				!outputIsDirectory ||
				invalidOutFileName
			);

			if (disabled) {
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 50, 50, 255));
				if (m_name.empty()) ImGui::Text("No Name is set.");
				else if (m_shaderPath.empty()) ImGui::Text("No shader file is set.");
				else if (m_outputPath.empty()) ImGui::Text("No output directory is set.");
				else if (!shaderExists) ImGui::Text("Shader file does not exist.");
				else if (!shaderIsFile) ImGui::Text("Shader is not a file.");
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

				PipelineSpecification spec = {};
				spec.colorFormat = m_colorFormats[m_colorFormatIndex];
				spec.depthStencilFormat = m_depthFormats[m_depthFormatIndex];
				spec.depthTest = m_depthTest;
				spec.depthWrite = m_depthWrite;
				spec.depthFunction = m_depthCompares[m_depthCompareIndex];
				spec.stencilEnabled = m_stencilEnabled;
				spec.sampleCount = m_sampleCount;
				spec.cullMode = m_cullModes[m_cullModeIndex];
				spec.topology = m_topologies[m_topologyIndex];
				spec.numRenderTargets = m_renderTargetsCount;
				BufferLayout layout(m_bufferElements);
				spec.vertexLayout = layout;

				UUID newAssetUUID = UUID::generate();

				AAP::PipelineAssetData data;
				data.uuid = newAssetUUID;
				data.shaderFilePath = AssetManager::getRelativeToAssets(m_shaderPath);
				data.name = m_name;
				data.spec = spec;

				AAP::PipelineParser::createTextFile(data, finalPath.string());

				AssetMetadata metadata;
				metadata.handle = newAssetUUID;
				metadata.type = AssetType::Pipeline;
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

		}

	}

	void PipelineImportModal::resetInputs() {
		m_name.clear();
		m_shaderPath.clear();
		m_outputPath.clear();
		m_colorFormatIndex = 1;
		m_depthFormatIndex = 2;
		m_depthTest = true;
		m_depthWrite = true;
		m_depthCompareIndex = 1;
		m_stencilEnabled = false;
		m_sampleCount = 1;
		m_cullModeIndex = 2;
		m_topologyIndex = 3;
		m_renderTargetsCount = 1;
		m_bufferElements.clear();
	}

}
