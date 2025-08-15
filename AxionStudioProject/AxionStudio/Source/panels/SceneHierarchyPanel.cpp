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
			if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
				m_selectedEntity = {};
			}

		}
		ImGui::End();

		if (ImGui::Begin("Properties")) {
			if (m_selectedEntity) {
				displayComponents(m_selectedEntity);
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

	void SceneHierarchyPanel::displayComponents(Entity entity) {
		if (entity.hasComponent<TagComponent>()) {
			auto& tag = entity.getComponent<TagComponent>().tag;

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), tag.c_str());

			if (ImGui::InputText("Tag", buffer, sizeof(buffer))) {
				tag = std::string(buffer);
			}
		}

		if (entity.hasComponent<TransformComponent>()) {
			if (ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Transform")) {
				auto& component = entity.getComponent<TransformComponent>();
				auto& position = component.position;
				auto& rotation = component.rotation;
				auto& scale = component.scale;

				ImGui::DragFloat3("Position", position.data(), 0.05f);
				ImGui::DragFloat3("Rotation", rotation.data(), 0.05f);
				ImGui::DragFloat3("Scale", scale.data(), 0.05f, 0.0f, 100.0f);

				ImGui::TreePop();
			}
		}

		if (entity.hasComponent<MeshComponent>()) {
			if (ImGui::TreeNodeEx((void*)typeid(MeshComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Mesh")) {
				ImGui::TreePop();
			}
		}

		if (entity.hasComponent<MaterialComponent>()) {
			if (ImGui::TreeNodeEx((void*)typeid(MaterialComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Material")) {
				auto& component = entity.getComponent<MaterialComponent>();
				ImGui::Text(component.getName().c_str());
				ImGui::Text(component.material->getShader()->getName().c_str());
				ImGui::ColorPicker4("Color", component.getColor().data());

				ImGui::TreePop();
			}
		}

		if (entity.hasComponent<CameraComponent>()) {
			if (ImGui::TreeNodeEx((void*)typeid(CameraComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Camera")) {
				ImGui::TreePop();
			}
		}

	}

}
