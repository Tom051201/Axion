#pragma once

#include "AxionStudio/Source/core/Panel.h"

#include "AxionEngine/Source/scene/Scene.h"
#include "AxionEngine/Source/scene/Entity.h"
#include "AxionEngine/Source/events/ApplicationEvent.h"

// TODO : TEMP
#include "AxionEngine/Source/render/Material.h"

namespace Axion {

	class SceneHierarchyPanel : public Panel {
	public:

		SceneHierarchyPanel(const std::string& name, const Ref<Scene>& activeScene);
		~SceneHierarchyPanel() override;

		void setup() override;
		void shutdown() override;
		void onEvent(Event& e) override;
		void onGuiRender() override;

		void setContext(const Ref<Scene>& context);


		Entity getSelectedEntity() const { return m_selectedEntity; }

		// TODO: TEMP
		Ref<Material> m_basicMaterial;

	private:

		Ref<Scene> m_context = nullptr;
		Entity m_selectedEntity;

		void drawVec3Control(const std::string& label, Vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f);
		
		template<typename T>
		void drawAddComponent(const char* name) {
			if (!m_selectedEntity.hasComponent<T>()) {
				if (ImGui::MenuItem(name)) {
					m_selectedEntity.addComponent<T>();
					ImGui::CloseCurrentPopup();
				}
			}
		}

		template<typename T>
		void drawComponentInfo(const char* name, Entity entity, std::function<void()> guiCode) {
			if (entity.hasComponent<T>()) {
				// creates treenode and + button
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
				bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap, name);
				ImGui::SameLine(ImGui::GetWindowWidth() - 25.0f);
				if (ImGui::Button("+", ImVec2{ 20, 20 })) { ImGui::OpenPopup("ComponentSettings"); }
				ImGui::PopStyleVar();

				// draws component settings on + button
				bool removeComponent = false;
				if (ImGui::BeginPopup("ComponentSettings")) {
					if (ImGui::MenuItem("Remove Component")) { removeComponent = true; }
					ImGui::EndPopup();
				}

				// draws the custom gui code
				if (open) {
					guiCode();
					ImGui::TreePop();
				}

				// removes component
				if (removeComponent) {
					entity.removeComponent<T>();
				}

			}
		}

		void displayEntity(Entity entity);
		void displayComponents(Entity entity);

		bool onSceneChanged(SceneChangedEvent& e);

	};

}
