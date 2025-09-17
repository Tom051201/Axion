#include "SceneOverviewPanel.h"

#include "AxionEngine/Vendor/imgui/imgui.h"

#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/scene/SceneManager.h"
#include "AxionEngine/Source/project/ProjectManager.h"

namespace Axion {

	SceneOverviewPanel::SceneOverviewPanel(const std::string& name) : Panel(name) {}

	SceneOverviewPanel::~SceneOverviewPanel() {
		shutdown();
	}

	void SceneOverviewPanel::setup() {}

	void SceneOverviewPanel::shutdown() {}

	void SceneOverviewPanel::onEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<SceneChangedEvent>(AX_BIND_EVENT_FN(SceneOverviewPanel::onSceneChanged));
	}

	void SceneOverviewPanel::onGuiRender() {
		ImGui::Begin("Scene Overview");

		if (!ProjectManager::hasProject()) {
			ImGui::TextWrapped("No Project Loaded. \nPlease load or create a project first.");
			ImGui::End();
			return;
		}

		if (!m_activeScene) {
			ImGui::TextWrapped("No Scene loaded");
			ImGui::End();
			return;
		}

		// -- Title --
		ImGui::TextUnformatted("Title");
		ImGui::SameLine();
		strcpy_s(m_titleBuffer, sizeof(m_titleBuffer), m_activeScene->getTitle().c_str());
		m_titleBuffer[sizeof(m_titleBuffer) - 1] = '\0';
		if (ImGui::InputText("##sceneTitle", m_titleBuffer, sizeof(m_titleBuffer))) {
			m_activeScene->setTitle(m_titleBuffer);
		}

		ImGui::SeparatorText("Skybox");
		if (m_activeScene->hasSkybox()) {
			// -- Has a skybox --
			std::filesystem::path skyPath = std::filesystem::path(m_activeScene->getSkyboxPath());
			std::filesystem::path skyRel = std::filesystem::relative(skyPath, ProjectManager::getProject()->getAssetsPath());
			ImGui::Text("Title: %s", skyPath.stem().string().c_str());
			ImGui::Text("Path: %s", skyRel.string().c_str());
		} 
		else {
			// -- Does not have a skybox --
			ImGui::Text("No Skybox has been selected");
		}


		if (ImGui::Button("Select Skybox")) {
			std::string filePath = FileDialogs::openFile({ {"PNG", "*.png"} }, ProjectManager::getProject()->getAssetsPath() + "\\skybox");
			if (!filePath.empty()) {
				m_activeScene->setSkybox(AssetHandle<Skybox>(filePath));
			}
		}

		ImGui::SameLine();
		if (ImGui::Button("Remove")) {
			m_activeScene->setSkybox(nullptr);
		}

		ImGui::End();
	}

	bool SceneOverviewPanel::onSceneChanged(SceneChangedEvent& e) {

		setScene(SceneManager::getScene());

		return false;
	}

}
