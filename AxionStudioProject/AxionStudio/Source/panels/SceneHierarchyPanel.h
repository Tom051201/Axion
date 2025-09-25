#pragma once

#include "AxionStudio/Source/core/Panel.h"

#include "AxionEngine/Source/scene/Scene.h"
#include "AxionEngine/Source/scene/Entity.h"
#include "AxionEngine/Source/events/ApplicationEvent.h"

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

	private:

		Ref<Scene> m_context = nullptr;
		Entity m_selectedEntity;

		void drawVec3Control(const std::string& label, Vec3& values, float resetX = 0.0f, float resetY = 0.0f, float resetZ = 0.0f, float columnWidth = 100.0f);
		
		// Returns true if the entity already has the component
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
			const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
			if (entity.hasComponent<T>()) {
				// creates treenode and + button
				ImGui::PushID((void*)typeid(T).hash_code());
				ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
				float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

				ImGui::SeparatorText("");
				bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name);
				ImGui::PopStyleVar();
				ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
				if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight })) { ImGui::OpenPopup("ComponentSettings"); }

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
				ImGui::PopID();
			}
		}

		void displayEntity(Entity entity);
		void displayComponents(Entity entity);

		bool onSceneChanged(SceneChangedEvent& e);

	};

}
