#include "SceneOverviewPanel.h"

#include "AxionEngine/Vendor/imgui/imgui.h"

#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/scene/SceneManager.h"

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

		if (m_activeScene) ImGui::Text(m_activeScene->getTitle().c_str());
		
		if (ImGui::Button("Select Skybox")) {
			std::string filePath = FileDialogs::openFile({ {"PNG", "*.png"} });
			if (!filePath.empty()) {
				m_activeScene->setSkybox(AssetHandle<Skybox>(filePath));
			}
		}
		
		if (m_activeScene && m_activeScene->hasSkybox()) {
			ImGui::Text(m_activeScene->getSkyboxPath().c_str());
		}

		if (m_activeScene) {
			ImGui::Text("Is new: %s", std::to_string(SceneManager::isNewScene()).c_str());
		}

		ImGui::End();
	}

	bool SceneOverviewPanel::onSceneChanged(SceneChangedEvent& e) {

		setScene(SceneManager::getScene());

		return false;
	}

}
