#include "ProjectPanel.h"

#include "AxionEngine/Vendor/imgui/imgui.h"

#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionEngine/Source/core/PlatformUtils.h"

#include "AxionStudio/Source/core/EditorConfig.h"

namespace Axion {

	ProjectPanel::ProjectPanel(const std::string& name) : Panel(name) {}

	ProjectPanel::~ProjectPanel() {
		shutdown();
	}

	void ProjectPanel::setup() {}

	void ProjectPanel::shutdown() {}

	void ProjectPanel::setProject(const Ref<Project>& project) {
		m_project = project;
		if (ProjectManager::hasProject()) {
			m_rootDirectory = std::filesystem::path(m_project->getProjectPath()).parent_path();

			m_projectFileRelative = std::filesystem::relative(project->getProjectPath(), m_rootDirectory);
			m_assetsRelative = std::filesystem::relative(project->getAssetsPath(), m_rootDirectory);
			m_scenesRelative = std::filesystem::relative(project->getScenesPath(), m_rootDirectory);
		}

	}

	void ProjectPanel::onGuiRender() {
		ImGui::Begin("Project Overview");

		// ----- Draw info when no project is selected -----
		if (!ProjectManager::hasProject()) {
			ImGui::TextWrapped("No Project Loaded. \nPlease load or create a project first.");
			ImGui::End();
			return;
		}

		ImGui::SeparatorText("General");
		{
			// ----- Project name -----
			static char nameBuffer[256];
			strcpy_s(nameBuffer, sizeof(nameBuffer), m_project->getName().c_str());
			nameBuffer[sizeof(nameBuffer) - 1] = '\0';

			if (ImGui::InputText("Project Name", nameBuffer, sizeof(nameBuffer))) {
				m_project->setName(nameBuffer);
			}
		}

		ImGui::SeparatorText("Paths");
		if (ImGui::BeginTable("ProjectPaths", 3, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg)) {
			ImGui::TableSetupColumn("Type");
			ImGui::TableSetupColumn("Path");
			ImGui::TableSetupColumn("Action");
			ImGui::TableHeadersRow();

			// ----- Project row -----
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Project");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s", m_projectFileRelative.string().c_str());
			ImGui::TableSetColumnIndex(2);
			if (ImGui::Button("Open Project Folder")) {
				PlatformUtils::openFolderInFileExplorer(m_project->getProjectPath());
			}

			// ----- Assets row -----
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Assets");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s", m_assetsRelative.string().c_str());
			ImGui::TableSetColumnIndex(2);
			if (ImGui::Button("Open Assets")) {
				PlatformUtils::openFolderInFileExplorer(m_project->getAssetsPath());
			}

			// ----- Scenes row -----
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Scenes");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s", m_scenesRelative.string().c_str());
			ImGui::TableSetColumnIndex(2);
			if (ImGui::Button("Open Scenes")) {
				PlatformUtils::openFolderInFileExplorer(m_project->getScenesPath());
			}

			ImGui::EndTable();
		}

		// Default scene
		ImGui::Text("Default Scene: ");
		ImGui::SameLine();
		ImGui::Text(m_project->getDefaultScene().c_str());
		ImGui::SameLine();
		if (ImGui::Button("Select")) {
			
		}

		ImGui::Separator();
		{
			if (ImGui::Button("Save Project")) {
				ProjectManager::saveProject(ProjectManager::getProjectFilePath());
			}

			ImGui::SameLine();
			if (ImGui::Button("Set As Startup")) {
				EditorConfig::startupProjectPath = m_project->getProjectPath() + "\\" + m_project->getName() + ".axproj";
			}
		}

		ImGui::End();
	}

	void ProjectPanel::onEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<ProjectChangedEvent>(AX_BIND_EVENT_FN(ProjectPanel::onProjectChanged));
	}

	bool ProjectPanel::onProjectChanged(ProjectChangedEvent& e) {
		setProject(ProjectManager::getProject());
		return false;
	}

}
