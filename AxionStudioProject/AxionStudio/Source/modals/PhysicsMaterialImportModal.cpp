#include "PhysicsMaterialImportModal.h"

#include "AxionEngine/Vendor/imgui/imgui.h"

#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionAssetPipeline/Source/AxPhysicsMaterial.h"

namespace Axion {

	constexpr float inputFieldWidth = 200.0f;

	PhysicsMaterialImportModal::PhysicsMaterialImportModal(const char* name) : Modal(name) {}

	PhysicsMaterialImportModal::~PhysicsMaterialImportModal() {}

	void PhysicsMaterialImportModal::close() {
		Modal::close();
		clearBuffers();
	}

	void PhysicsMaterialImportModal::renderContent() {
		ImGui::SeparatorText("Import Physics Material Asset");
		ImGui::Spacing();

		if (ImGui::BeginTable("##ImportPhysicsMaterialTable", 2, ImGuiTableFlags_BordersInnerV)) {
			ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 120.0f);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

			// -- Name --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Name");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##PhyMatName_input", m_nameBuffer, sizeof(m_nameBuffer));


			// -- Static Friction --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Static Friction");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::DragFloat("##staticFric_drag", &m_staticFriction, 0.05f, 0.0f, 1.0f);


			// -- Dyanamic Friction --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Dynamic Friction");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::DragFloat("##dynamicFric_drag", &m_dynamicFriction, 0.05f, 0.0f, 1.0f);


			// -- Restitution --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Restitution");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::DragFloat("##Restitution_drag", &m_restitution, 0.05f, 0.0f, 1.0f);


			// -- Output path --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Separator();
			ImGui::Text("Output Location");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##PhyMatOutputPath_input", m_outputPathBuffer, sizeof(m_outputPathBuffer));
			ImGui::SameLine();
			if (ImGui::Button("Browse##PhyMatOutputDir_button")) {
				std::filesystem::path phymatDir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "physics";
				std::string absPath = FileDialogs::openFolder(phymatDir.string());
				if (!absPath.empty()) {
					strcpy_s(m_outputPathBuffer, IM_ARRAYSIZE(m_outputPathBuffer), absPath.c_str());
					m_outputPathBuffer[IM_ARRAYSIZE(m_outputPathBuffer) - 1] = '\0';
				}
			}

			ImGui::EndTable();

			// -- Validate input --
			std::filesystem::path outputDirPath = std::string(m_outputPathBuffer);
			bool validOutputPath = std::filesystem::exists(outputDirPath);
			bool validOutputFile = !std::filesystem::exists(outputDirPath / (std::string(m_nameBuffer) + ".axpmat"));

			bool disabled = (
				strlen(m_nameBuffer) == 0 ||
				strlen(m_outputPathBuffer) == 0 ||
				!validOutputPath ||
				!validOutputFile
			);

			ImGui::Separator();
			ImGui::BeginDisabled(disabled);
			if (ImGui::Button("Create")) {
				std::filesystem::path outDir = std::string(m_outputPathBuffer);
				std::filesystem::path outFile = outDir / (std::string(m_nameBuffer) + ".axpmat");

				AAP::PhysicsMaterialAssetData data;
				data.name = m_nameBuffer;
				data.staticFriction = m_staticFriction;
				data.dynamicFriction = m_dynamicFriction;
				data.restitution = m_restitution;

				AAP::PhysicsMaterialParser::createAxPhyMatFile(data, outFile.string());
				close();
			}
			ImGui::EndDisabled();
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) {
				close();
			}

		}
	}

	void PhysicsMaterialImportModal::clearBuffers() {
		m_nameBuffer[0] = '\0';
		m_outputPathBuffer[0] = '\0';
		m_staticFriction = 0.5f;
		m_dynamicFriction = 0.5f;
		m_restitution = 0.05f;
	}

}
