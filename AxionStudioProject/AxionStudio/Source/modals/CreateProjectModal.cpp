#include "CreateProjectModal.h"

#include "AxionEngine/Vendor/imgui/imgui.h"

#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/project/ProjectManager.h"

namespace Axion {

	constexpr float inputFieldWidth = 200.0f;

	CreateProjectModal::CreateProjectModal(const char* name) : Modal(name) {}

	CreateProjectModal::~CreateProjectModal() {}

	void CreateProjectModal::close() {
		Modal::close();
		clearBuffers();
	}

	void CreateProjectModal::renderContent() {

		ImGui::SeparatorText("Create New Project");
		ImGui::Spacing();

		bool textChanged = false;

		if (ImGui::BeginTable("##CreateProjectTable", 2, ImGuiTableFlags_BordersInnerV)) {
			ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 100.0f);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

			// -- Name --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Project Name");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			textChanged |= ImGui::InputText("##ProjName", m_nameBuffer, sizeof(m_nameBuffer));

			// -- Location --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Location");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			textChanged |= ImGui::InputText("##ProjLoc", m_locationBuffer, sizeof(m_locationBuffer));
			ImGui::SameLine();
			if (ImGui::Button("Browse...##Loc")) {
				std::string folder = FileDialogs::openFolder();
				if (!folder.empty()) {
					strcpy_s(m_locationBuffer, sizeof(m_locationBuffer), folder.c_str());
				}
			}

			// -- Author (Optional) --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Author");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##ProjAuth", m_authorBuffer, sizeof(m_authorBuffer));

			// -- Company (Optional) --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Company");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##ProjComp", m_companyBuffer, sizeof(m_companyBuffer));

			ImGui::EndTable();
		}

		// ----- Validation -----
		std::filesystem::path locPath(m_locationBuffer);
		bool validLocation = std::filesystem::is_directory(locPath);
		std::filesystem::path projectFolder = locPath / m_nameBuffer;
		bool invalidName = std::filesystem::exists(projectFolder);

		bool disabled = (strlen(m_nameBuffer) == 0 || strlen(m_locationBuffer) == 0 || !validLocation || invalidName);

		if (invalidName && strlen(m_nameBuffer) > 0) {
			ImGui::TextColored(ImVec4(0.9f, 0.2f, 0.2f, 1.0f), "A folder with this name already exists at this location!");
		}

		ImGui::Separator();
		ImGui::BeginDisabled(disabled);
		if (ImGui::Button("Create Project")) {
			ProjectSpecification spec;
			spec.name = m_nameBuffer;
			spec.location = m_locationBuffer;
			spec.author = m_authorBuffer;
			spec.company = m_companyBuffer;

			ProjectManager::newProject(spec);
			close();
		}
		ImGui::EndDisabled();

		ImGui::SameLine();
		if (ImGui::Button("Cancel")) {
			close();
		}
	}

	void CreateProjectModal::clearBuffers() {
		m_nameBuffer[0] = '\0';
		m_locationBuffer[0] = '\0';
		m_authorBuffer[0] = '\0';
		m_companyBuffer[0] = '\0';
		m_descriptionBuffer[0] = '\0';
	}

}
