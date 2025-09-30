#include "ProjectPanel.h"

#include "AxionEngine/Vendor/imgui/imgui.h"

#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/project/ProjectManager.h"

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
		ImGui::SeparatorText("Project Overview");

		// ----- Draw info when no project is selected -----
		if (!ProjectManager::hasProject()) {
			ImGui::TextWrapped("No Project Loaded. \nPlease load or create a project first.");
			ImGui::End();
			return;
		}


		if (ImGui::BeginTable("ProjectOverviewTable", 2, ImGuiTableFlags_BordersInnerV)) {
			ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 120.0f);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

			// -- Title --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Name");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			static char titleBuffer[256];
			strcpy_s(titleBuffer, sizeof(titleBuffer), m_project->getName().c_str());
			titleBuffer[sizeof(titleBuffer) - 1] = '\0';
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			if (ImGui::InputText("Project Name", titleBuffer, sizeof(titleBuffer))) {
				m_project->setName(titleBuffer);
			}


			// -- Project file --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Project");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s", m_projectFileRelative.string().c_str());


			// -- Assets folder --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Assets");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s", m_assetsRelative.string().c_str());


			// -- Scenes folder --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Scenes");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s", m_scenesRelative.string().c_str());


			// -- Default scene --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Default Scene");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			std::filesystem::path defaultScenePath = AssetManager::getRelativeToAssets(m_project->getDefaultScene());
			std::string fileName = defaultScenePath.filename().string();
			ImGui::Text(fileName.c_str());


			// -- Show in Explorer --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Show in Explorer");
			ImGui::TableSetColumnIndex(1);
			if (ImGui::Button("Project")) {
				PlatformUtils::openFolderInFileExplorer(m_project->getProjectPath());
			}
			ImGui::SameLine();
			if (ImGui::Button("Assets")) {
				PlatformUtils::openFolderInFileExplorer(m_project->getAssetsPath());
			}
			ImGui::SameLine();
			if (ImGui::Button("Scenes")) {
				PlatformUtils::openFolderInFileExplorer(m_project->getScenesPath());
			}


			// -- Options --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Options");
			ImGui::TableSetColumnIndex(1);
			if (ImGui::Button("Save")) {
				ProjectManager::saveProject(ProjectManager::getProjectFilePath());
			}
			ImGui::SameLine();
			if (ImGui::Button("Set As Startup")) {
				EditorConfig::startupProjectPath = ProjectManager::getProjectFilePath();
			}

			ImGui::EndTable();
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
