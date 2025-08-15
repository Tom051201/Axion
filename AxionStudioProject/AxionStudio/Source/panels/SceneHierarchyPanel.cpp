#include "SceneHierarchyPanel.h"

#include "AxionEngine/Source/scene/Components.h"

#include "AxionEngine/Vendor/imgui/imgui.h"

namespace Axion {

	SceneHierarchyPanel::SceneHierarchyPanel() {}

	SceneHierarchyPanel::~SceneHierarchyPanel() {
		shutdown();
	}

	void SceneHierarchyPanel::setup(const Ref<Scene>& activeScene) {
		setContext(activeScene);
	}

	void SceneHierarchyPanel::shutdown() {
	
	}

	void SceneHierarchyPanel::setContext(const Ref<Scene>& context) {
		m_context = context;
	}

	void SceneHierarchyPanel::onGuiRender() {
		if (ImGui::Begin("Scene Hierarchy")) {

			for (auto e : m_context->getRegistry().view<entt::entity>()) {
				Entity entity{ e, m_context.get() };

				displayEntity(entity);
			
			}

		}
		ImGui::End();
	}

	void SceneHierarchyPanel::displayEntity(Entity entity) {
		auto& name = entity.getComponent<TagComponent>().tag;

		ImGuiTreeNodeFlags flags = ((m_selectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, name.c_str());

		if (ImGui::IsItemClicked()) {
			m_selectedEntity = entity;
		}

		if (opened) {
			ImGui::TreePop();
		}

	}

}
