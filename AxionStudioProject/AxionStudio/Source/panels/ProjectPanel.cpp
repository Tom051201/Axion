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
		m_rootDirectory = std::filesystem::path(m_project->getProjectPath()).parent_path();

		auto projectFileRelative = std::filesystem::relative(project->getProjectPath(), m_rootDirectory);
		auto assetsRelative = std::filesystem::relative(project->getAssetsPath(), m_rootDirectory);
		auto scenesRelative = std::filesystem::relative(project->getScenesPath(), m_rootDirectory);

		m_projectFileDisplay = (std::filesystem::path(m_rootDirectory.filename().string()) / projectFileRelative).string();
		m_assetsDisplay = (std::filesystem::path(m_rootDirectory.filename().string()) / assetsRelative).string();
		m_scenesDisplay = (std::filesystem::path(m_rootDirectory.filename().string()) / scenesRelative).string();
	}

	void ProjectPanel::onGuiRender() {
		ImGui::Begin("Project Overview");

		// ----- Draw info when no project is selected -----
		if (!ProjectManager::hasActiveProject()) {
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
			ImGui::Text("%s", m_projectFileDisplay.c_str());
			ImGui::TableSetColumnIndex(2);
			if (ImGui::Button("Open Project Folder")) {
				PlatformUtils::openFolderInFileExplorer(m_project->getProjectPath());
			}

			// ----- Assets row -----
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Assets");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s", m_assetsDisplay.c_str());
			ImGui::TableSetColumnIndex(2);
			if (ImGui::Button("Open Assets")) {
				PlatformUtils::openFolderInFileExplorer(m_project->getAssetsPath());
			}

			// ----- Scenes row -----
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Scenes");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s", m_scenesDisplay.c_str());
			ImGui::TableSetColumnIndex(2);
			if (ImGui::Button("Open Scenes")) {
				PlatformUtils::openFolderInFileExplorer(m_project->getScenesPath());
			}

			ImGui::EndTable();
		}

		ImGui::Separator();
		{
			// TODO: save project button
			if (ImGui::Button("Save Project")) {

			}

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
		setProject(ProjectManager::getActiveProject());
		return false;
	}

}
