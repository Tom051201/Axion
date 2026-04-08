#include "ExportProjectModal.h"

#include "AxionEngine/Vendor/imgui/imgui.h"
#include "AxionEngine/Vendor/imgui/misc/cpp/imgui_stdlib.h"

#include "AxionEngine/Source/AxionSettings.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionAssetPipeline/Source/core/AssetPackager.h"

#include <filesystem>

namespace Axion {

	void ExportProjectModal::renderContent() {
		ImGui::SeparatorText("Export Project (Windows x64)");
		ImGui::Spacing();

		auto project = ProjectManager::getProject();
		if (!project) {
			ImGui::TextColored(ImVec4(0.9f, 0.2f, 0.2f, 1.0f), "No active project to export!");
			if (ImGui::Button("Close")) close();
			return;
		}

		// ----- Build Summary Information -----
		ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "Build Summary:");
		ImGui::Indent();
		ImGui::Text("Name: %s", project->getName().c_str());
		ImGui::Text("Version: %s", project->getVersion().c_str());
		ImGui::TextDisabled("Powered by Axion Engine v%s", AX_ENGINE_VERSION);
		if (!project->getAppIconPath().empty()) {
			std::string iconName = project->getAppIconPath().stem().string();
			ImGui::Text("Custom Icon: %s", iconName.c_str());
		}
		else {
			ImGui::TextDisabled("No Custom Icon (Using Default)");
		}
		if (!project->getDefaultScene().empty()) {
			std::string fileName = project->getDefaultScene().stem().string();
			ImGui::Text("Default Scene: %s", fileName.c_str());
		}
		else {
			ImGui::Text("No Default Scene selected!");
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
			ImGui::InputText("##ExportPath", &m_exportPath);
			ImGui::SameLine();
			if (ImGui::Button("Browse...##ExpLoc")) {
				std::filesystem::path exportDir = ProjectManager::getProject()->getProjectPath();
				std::filesystem::path absPath = FileDialogs::openFolder(exportDir);
				if (!absPath.empty()) m_exportPath = absPath.string();
			}

			// -- Options --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Options");
			ImGui::TableSetColumnIndex(1);
			ImGui::Checkbox("Open Folder After Export", &m_openAfterExport);

			ImGui::EndTable();
		}

		// -- Validate --
		std::filesystem::path exportDirPath = m_exportPath;
		bool validExportPath = std::filesystem::exists(exportDirPath);
		bool hasDefaultScene = !project->getDefaultScene().empty();
		bool validDefaultScene = std::filesystem::exists(project->getDefaultScene());

		bool disabled = (
			m_exportPath.empty() ||
			!hasDefaultScene ||
			!validDefaultScene ||
			!validExportPath
		);

		if (disabled) {
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 50, 50, 255));
			if (m_exportPath.empty()) ImGui::Text("Export path needs to be set.");
			else if (!validExportPath) ImGui::Text("Export directory does not exist.");
			else if (!hasDefaultScene) ImGui::Text("Unable exporting without a default scene.");
			else if (!validDefaultScene) ImGui::Text("Default scene does not exist.");
			ImGui::PopStyleColor();
		}

		ImGui::Separator();
		ImGui::BeginDisabled(disabled);
		if (ImGui::Button("Package Project", ImVec2(120, 0))) {
			AAP::AssetPackager::packageProject(m_exportPath);

			if (m_openAfterExport) {
				PlatformUtils::openFolderInFileExplorer(m_exportPath);
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
