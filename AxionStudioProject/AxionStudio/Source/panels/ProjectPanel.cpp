#include "ProjectPanel.h"

#include "AxionEngine/Vendor/imgui/imgui.h"

namespace Axion {

	ProjectPanel::ProjectPanel() {}

	ProjectPanel::~ProjectPanel() {
		shutdown();
	}

	void ProjectPanel::setup() {}

	void ProjectPanel::shutdown() {}

	void ProjectPanel::setProject(const Ref<Project>& project) {
		m_project = project;
	}

	void ProjectPanel::onGuiRender() {
		ImGui::Begin("Project Overview");

		ImGui::SeparatorText("General");
		{
			static char nameBuffer[256];
			strncpy(nameBuffer, m_project->getName().c_str(), sizeof(nameBuffer));
			nameBuffer[sizeof(nameBuffer) - 1] = '\0';

			if (ImGui::InputText("Project Name", nameBuffer, sizeof(nameBuffer))) {
				m_project->setName(nameBuffer);
			}

			ImGui::Text("Project File: %s", m_project->getProjectPath().c_str());
		}

		ImGui::End();
	}

}
