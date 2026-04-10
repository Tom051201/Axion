#include "ProjectPanel.h"

#include "AxionEngine/Vendor/imgui/imgui.h"
#include "AxionEngine/Vendor/imgui/misc/cpp/imgui_stdlib.h"

#include "AxionEngine/Source/EngineConfig.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionEngine/Source/scene/SceneManager.h"

#include "AxionStudio/Source/core/EditorConfig.h"

#include "AxionAssetPipeline/Source/core/AssetPackager.h"

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
			m_rootDirectory = m_project->getProjectPath().parent_path();

			m_projectFileRelative = std::filesystem::relative(project->getProjectPath(), m_rootDirectory);
			m_assetsRelative = std::filesystem::relative(project->getAssetsPath(), m_rootDirectory);
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
			std::string projectName = m_project->getName();
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			if (ImGui::InputText("Project Name", &projectName)) {
				m_project->setName(projectName);
			}


			// -- Game Version --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Version");
			ImGui::TableSetColumnIndex(1);
			Version projectVersion = m_project->getVersion();
			int versionArr[3] = { (int)projectVersion.major, (int)projectVersion.minor, (int)projectVersion.patch };
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			if (ImGui::InputInt3("##ProjectVersion", versionArr)) {
				projectVersion.major = std::max(0, versionArr[0]);
				projectVersion.minor = std::max(0, versionArr[1]);
				projectVersion.patch = std::max(0, versionArr[2]);
				m_project->setVersion(projectVersion);
			}


			// -- App Icon --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("App Icon (.ico)");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);

			std::filesystem::path currentIcon = m_project->getAppIconPath();
			std::string iconDisplay = currentIcon.empty() ? "None" : currentIcon.filename().string();

			ImGui::Text(iconDisplay.c_str());
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 65.0f);
			if (ImGui::Button("Browse##Icon")) {
				std::filesystem::path path = FileDialogs::openFile({ {"Windows Icon", "*.ico"} });
				if (!path.empty()) {
					m_project->setAppIconPath(path);
					ProjectManager::saveProject(ProjectManager::getProjectFilePath());
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("X##ClearIcon")) {
				m_project->setAppIconPath("");
				ProjectManager::saveProject(ProjectManager::getProjectFilePath());
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


			// -- Default scene --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Default Scene");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			std::filesystem::path currentDefault = m_project->getDefaultScene();
			std::string displayString = "None (Drag .axscene here)";
			if (!currentDefault.empty()) {
				std::filesystem::path defaultScenePath = AssetManager::getRelativeToAssets(currentDefault);
				displayString = defaultScenePath.filename().string();
			}

			ImGui::Button(displayString.c_str(), ImVec2(ImGui::GetContentRegionAvail().x - 100.0f, 0.0f));
			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
					std::filesystem::path path = static_cast<const char*>(payload->Data);
					if (path.extension() == ".axscene") {
						std::filesystem::path absPath = AssetManager::getAbsolute(path);
						m_project->setDefaultScene(absPath);
						ProjectManager::saveProject(ProjectManager::getProjectFilePath());
					}
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::SameLine();
			if (ImGui::Button("Set Current")) {
				std::filesystem::path currentScenePath = SceneManager::getScenePath();
				if (!currentScenePath.empty()) {
					std::filesystem::path absPath = AssetManager::getAbsolute(currentScenePath);
					m_project->setDefaultScene(absPath);
					ProjectManager::saveProject(ProjectManager::getProjectFilePath());
				}
			}


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
			ImGui::SameLine();
			if (ImGui::Button("Export")) {
				if (m_openExportModalCallback) {
					m_openExportModalCallback();
				}
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
