#include "axpch.h"
#include "ExportProjectModal.h"
#include "AxionEngine/Vendor/imgui/imgui.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionAssetPipeline/Source/core/AssetPackager.h"
#include <filesystem>

namespace Axion {

	ExportProjectModal::ExportProjectModal(const char* name) : Modal(name) {}

	void ExportProjectModal::open() {
		Modal::open();

		// Suggest an "Export" folder next to the project by default
		if (ProjectManager::hasProject()) {
			std::filesystem::path projPath = ProjectManager::getProject()->getProjectPath();
			std::string defaultExport = (projPath.parent_path() / "Export").string();
			strcpy_s(m_exportPathBuffer, sizeof(m_exportPathBuffer), defaultExport.c_str());
		}
	}

	void ExportProjectModal::renderContent() {
		ImGui::SeparatorText("Export Project (Windows x64)");
		ImGui::Spacing();

		auto project = ProjectManager::getProject();
		if (!project) {
			ImGui::TextColored(ImVec4(0.9f, 0.2f, 0.2f, 1.0f), "No active project to export!");
			if (ImGui::Button("Close")) close();
			return;
		}

		// ----- UX Warning: Check for Default Scene -----
		if (project->getDefaultScene().empty()) {
			ImGui::TextColored(ImVec4(0.9f, 0.8f, 0.2f, 1.0f), "[WARNING] No Default Scene is set!");
			ImGui::TextWrapped("The standalone game will not know which scene to load. Please set a Default Scene in the Project Panel.");
			ImGui::Spacing();
		}

		// ----- Build Summary Information -----
		ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "Build Summary:");
		ImGui::Indent();
		ImGui::Text("Game: %s", project->getName().c_str());
		ImGui::Text("Version: %s", project->getVersion().c_str());
		ImGui::TextDisabled("Powered by Axion Engine v%s", project->getEngineVersion().c_str());

		if (!project->getAppIconPath().empty()) {
			ImGui::Text("Custom Icon: Yes");
		}
		else {
			ImGui::TextDisabled("Custom Icon: No (Using Default)");
		}
		ImGui::Unindent();
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		if (ImGui::BeginTable("##ExportProjectTable", 2, ImGuiTableFlags_BordersInnerV)) {
			ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 100.0f);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

			// -- Export Path --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Export Path");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(250.0f);
			ImGui::InputText("##ExportPath", m_exportPathBuffer, sizeof(m_exportPathBuffer));
			ImGui::SameLine();
			if (ImGui::Button("Browse...##ExpLoc")) {
				std::string folder = FileDialogs::openFolder();
				if (!folder.empty()) {
					strcpy_s(m_exportPathBuffer, sizeof(m_exportPathBuffer), folder.c_str());
				}
			}

			// -- Options --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Options");
			ImGui::TableSetColumnIndex(1);
			ImGui::Checkbox("Open Folder After Export", &m_openAfterExport);

			ImGui::EndTable();
		}

		bool disabled = (strlen(m_exportPathBuffer) == 0);

		ImGui::Separator();
		ImGui::BeginDisabled(disabled);
		if (ImGui::Button("Package Project", ImVec2(120, 0))) {
			AAP::AssetPackager::packageProject(m_exportPathBuffer);

			if (m_openAfterExport) {
				PlatformUtils::openFolderInFileExplorer(m_exportPathBuffer);
			}
			close();
		}
		ImGui::EndDisabled();

		ImGui::SameLine();
		if (ImGui::Button("Cancel")) {
			close();
		}
	}
}
