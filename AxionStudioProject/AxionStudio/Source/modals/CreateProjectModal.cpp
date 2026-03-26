#include "CreateProjectModal.h"

#include "AxionEngine/Vendor/imgui/imgui.h"
#include "AxionEngine/Vendor/imgui/misc/cpp/imgui_stdlib.h"

#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/project/ProjectManager.h"

namespace Axion {

	constexpr float inputFieldWidth = 200.0f;

	void CreateProjectModal::renderContent() {

		ImGui::SeparatorText("Create New Project");
		ImGui::Spacing();

		if (ImGui::BeginTable("##CreateProjectTable", 2, ImGuiTableFlags_BordersInnerV)) {
			ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 100.0f);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

			// -- Name --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Project Name");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##ProjName", &m_name);

			// -- Location --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Location");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##ProjLoc", &m_outputPath);
			ImGui::SameLine();
			if (ImGui::Button("Browse...##Loc")) {
				std::string folder = FileDialogs::openFolder();
				if (!folder.empty()) m_outputPath = folder;
			}

			// -- Author --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Author");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##ProjAuth", &m_author);

			// -- Company --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Company");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##ProjComp", &m_company);

			// -- Company --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Description");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##ProjDesc", &m_description);

			ImGui::EndTable();
		}

		// ----- Validation -----
		std::filesystem::path outpath = m_outputPath;
		bool validLocation = std::filesystem::is_directory(outpath);
		std::filesystem::path projectFolder = outpath / m_name;
		bool invalidName = std::filesystem::exists(projectFolder);

		bool disabled = (
			m_name.empty() ||
			m_outputPath.empty() ||
			!validLocation ||
			invalidName);

		if (disabled) {
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 50, 50, 255));
			if (m_name.empty()) ImGui::Text("Name needs to be set.");
			else if (!validLocation) ImGui::Text("Selected Location is not a folder");
			else if (invalidName) ImGui::Text("Project with this name already exists.");
			ImGui::PopStyleColor();
		}

		ImGui::Separator();
		ImGui::BeginDisabled(disabled);
		if (ImGui::Button("Create Project")) {
			ProjectSpecification spec;
			spec.name = m_name;
			spec.location = m_outputPath;
			spec.author = m_author;
			spec.company = m_company;
			spec.description = m_description;

			ProjectManager::newProject(spec);
			close();
		}
		ImGui::EndDisabled();

		ImGui::SameLine();
		if (ImGui::Button("Cancel")) {
			close();
		}
	}

	void CreateProjectModal::resetInputs() {
		m_name.clear();
		m_outputPath.clear();
		m_author.clear();
		m_company.clear();
		m_description.clear();
	}

}
