#include "SceneHierarchyPanel.h"

#include "AxionEngine/Source/scene/Components.h"

#include "AxionEngine/Vendor/imgui/imgui.h"

namespace Axion {

	SceneHierarchyPanel::SceneHierarchyPanel() {}

	SceneHierarchyPanel::~SceneHierarchyPanel() {
		shutdown();
	}

	void SceneHierarchyPanel::setup(Ref<Scene> activeScene) {
		m_activeScene = activeScene;
	}

	void SceneHierarchyPanel::shutdown() {
	
	}

	void SceneHierarchyPanel::onGuiRender() {
		if (ImGui::Begin("Scene Hierarchy")) {

			for (auto e : m_activeScene->getRegistry().view<entt::entity>()) {
				Entity entity{ e, m_activeScene.get() };

				displayEntity(entity);
			
			}

		}
		ImGui::End();
	}

	void SceneHierarchyPanel::displayEntity(Entity entity) {
		std::string name = entity.getComponent<TagComponent>().tag;
		ImGui::Text(name.c_str());
	}

}
