#include "SceneHierarchyPanel.h"

#include "AxionEngine/Vendor/imgui/imgui.h"
#include "AxionEngine/Vendor/imgui/imgui_internal.h"

#include "AxionEngine/Source/scene/Components.h"
#include "AxionEngine/Source/scene/SceneManager.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/project/ProjectManager.h"

namespace Axion {

	SceneHierarchyPanel::SceneHierarchyPanel(const std::string& name, const Ref<Scene>& activeScene) : Panel(name) {
		setContext(activeScene);
	}

	SceneHierarchyPanel::~SceneHierarchyPanel() {
		shutdown();
	}

	void SceneHierarchyPanel::setup() {}

	void SceneHierarchyPanel::shutdown() {}

	void SceneHierarchyPanel::onEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<SceneChangedEvent>(AX_BIND_EVENT_FN(SceneHierarchyPanel::onSceneChanged));
	}

	void SceneHierarchyPanel::setContext(const Ref<Scene>& context) {
		m_context = context;
		m_selectedEntity = {};
	}

	void SceneHierarchyPanel::onGuiRender() {
		// ----- Draw info when no project is selected -----
		if (!ProjectManager::hasProject()) {
			if (ImGui::Begin("Properties")) {
				ImGui::TextWrapped("No Project Loaded. \nPlease load or create a project first.");
				ImGui::End();
			}
			if (ImGui::Begin("Scene Hierarchy")) {
				ImGui::TextWrapped("No Project Loaded. \nPlease load or create a project first.");
				ImGui::End();
				return;
			}

		}


		// ----- Properties Panel -----
		if (ImGui::Begin("Properties")) {
			if (m_selectedEntity) {
				displayComponents(m_selectedEntity);
			}
		}
		ImGui::End();

		// ----- Scene Hierarchy Panel -----
		if (ImGui::Begin("Scene Hierarchy")) {

			for (auto e : m_context->getRegistry().view<entt::entity>()) {
				Entity entity{ e, m_context.get() };
				displayEntity(entity);
			}
			if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
				m_selectedEntity = {};
			}

		}

		// ----- Right click on blank space -----
		if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
			if (ImGui::MenuItem("Empty Entity")) { m_context->createEntity("Empty Entity"); }
			ImGui::EndPopup();
		}
		ImGui::End();

	}

	void SceneHierarchyPanel::drawVec3Control(const std::string& label, Vec3& values, float resetValue, float columnWidth) {
		ImGui::PushID(label.c_str());

		ImGui::Columns(2, 0, false);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y + 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		if (ImGui::Button("X", buttonSize)) { values.x = resetValue; }
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		if (ImGui::Button("Y", buttonSize)) { values.y = resetValue; }
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		if (ImGui::Button("Z", buttonSize)) { values.z = resetValue; }
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar(2);
		ImGui::Columns(1);
		ImGui::PopID();
	}

	void SceneHierarchyPanel::displayEntity(Entity entity) {
		auto& name = entity.getComponent<TagComponent>().tag;

		ImGuiTreeNodeFlags flags = ((m_selectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, name.c_str());

		if (ImGui::IsItemClicked()) { m_selectedEntity = entity; }

		// ----- Delete and Entity on right click -----
		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem()) {
			if (ImGui::MenuItem("Delete Entity")) { entityDeleted = true; }
			ImGui::EndPopup();
		}

		if (opened) {
			ImGui::TreePop();
		}

		// ----- Deleting Entities -----
		if (entityDeleted) {
			// delete entity at the end if it was deleted to allow child entities to work
			m_context->destroyEntity(entity);
			if (m_selectedEntity == entity) {
				m_selectedEntity = {};
			}
		}

	}

	void SceneHierarchyPanel::displayComponents(Entity entity) {

		// ----- TagComponent -----
		if (entity.hasComponent<TagComponent>()) {
			auto& tag = entity.getComponent<TagComponent>().tag;

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strncpy_s(buffer, sizeof(buffer), tag.c_str(), sizeof(buffer));

			if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) {
				tag = std::string(buffer);
			}
		}

		// ------ Add Component -----
		bool hasAll =
			m_selectedEntity.hasComponent<TransformComponent>() &&
			m_selectedEntity.hasComponent<MeshComponent>() &&
			m_selectedEntity.hasComponent<MaterialComponent>() &&
			m_selectedEntity.hasComponent<ConstantBufferComponent>() &&
			m_selectedEntity.hasComponent<CameraComponent>();

		ImGui::SameLine();
		ImGui::BeginDisabled(hasAll);
		if (ImGui::Button("Add Component")) ImGui::OpenPopup("AddComponent");
		ImGui::EndDisabled();
		if (ImGui::BeginPopup("AddComponent")) {

			drawAddComponent<TransformComponent>("Transform");
			drawAddComponent<MeshComponent>("Mesh");
			drawAddComponent<MaterialComponent>("Material");
			drawAddComponent<ConstantBufferComponent>("Upload Buffer");
			drawAddComponent<CameraComponent>("Camera");

			ImGui::EndPopup();
		}

		// ----- UUIDComponent -----
		if (entity.hasComponent<UUIDComponent>()) {
			auto& uuid = entity.getComponent<UUIDComponent>().id;
			ImGui::TextDisabled(uuid.toString().c_str());
		}

		// ----- TransformComponent -----
		if (entity.hasComponent<TransformComponent>()) {
			// Draw TransformComponent manual to disable removement

			// -- Creates treenode and + button --
			const ImGuiTreeNodeFlags treeNodeFlags = /*ImGuiTreeNodeFlags_DefaultOpen | */ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
			ImGui::PushID((void*)typeid(TransformComponent).hash_code());
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

			ImGui::SeparatorText("");
			bool open = ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), treeNodeFlags, "Transform");
			ImGui::PopStyleVar();
			ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
			if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight })) { ImGui::OpenPopup("ComponentSettings"); }

			// -- Draws component settings on + button --
			if (ImGui::BeginPopup("ComponentSettings")) {
				ImGui::EndPopup();
			}

			// -- Draws the custom gui code --
			if (open) {
				auto& component = m_selectedEntity.getComponent<TransformComponent>();
				auto& position = component.position;
				auto& rotation = component.rotation;
				auto& scale = component.scale;

				drawVec3Control("Position", position, 0.0f, 70.0f);
				drawVec3Control("Rotation", rotation, 0.0f, 70.0f);
				drawVec3Control("Scale", scale, 1.0f, 70.0f);

				ImGui::TreePop();
			}
			ImGui::PopID();
		}

		// ----- MeshComponent -----
		drawComponentInfo<MeshComponent>("Mesh", m_selectedEntity, [this]() {
			auto& component = m_selectedEntity.getComponent<MeshComponent>();
			if (component.handle.isValid()) {
				Ref<Mesh> mesh = AssetManager::get<Mesh>(component.handle);
				ImGui::Text("UUID: %s ", component.handle.uuid.toString().c_str());
				ImGui::Text("Vertices: %u", mesh->getVertexBuffer()->getVertexCount());
				ImGui::Text("Indices: %u", mesh->getIndexCount());
				if (ImGui::Button("Remove")) {
					component.handle.invalidate();
				}
			}
			else {
				// -- Load Button --
				if (ImGui::Button("Open Mesh...")) {
					std::string absPath = FileDialogs::openFile({ {"Axion Mesh Asset", "*.axmesh"} }, ProjectManager::getProject()->getAssetsPath() + "\\meshes");
					if (!absPath.empty()) {
						AssetHandle<Mesh> handle = AssetManager::load<Mesh>(absPath);
						component.handle = handle;
					}
				}

				// -- Drag drop on button --
				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
						std::string relPath = static_cast<const char*>(payload->Data);
						std::string absPath = AssetManager::getAbsolute(relPath);
						if (absPath.find(".axmesh") != std::string::npos) {
							AssetHandle<Mesh> handle = AssetManager::load<Mesh>(absPath);
							component.handle = handle;
						}
					}
					ImGui::EndDragDropTarget();
				}

			}
		});

		// ----- MaterialComponent -----
		drawComponentInfo<MaterialComponent>("Material", m_selectedEntity, [this]() {
			auto& component = m_selectedEntity.getComponent<MaterialComponent>();
			if (component.handle.isValid()) {
				ImGui::Text(AssetManager::get<Material>(component.handle)->getName().c_str());
				ImGui::Text(AssetManager::get<Shader>(AssetManager::get<Material>(component.handle)->getShaderHandle())->getName().c_str());
				ImGui::ColorEdit4("##ColorEdit", AssetManager::get<Material>(component.handle)->getColor().data());
				if (ImGui::Button("Remove")) {
					component.handle.invalidate();
				}
			}
			else {
				// -- Load Button --
				if (ImGui::Button("Open Material...")) {
					std::string absPath = FileDialogs::openFile({ {"Axion Material Asset", "*.axmat"} }, ProjectManager::getProject()->getAssetsPath() + "\\materials"); //TODO: remove back slashes here and above
					if (!absPath.empty()) {
						AssetHandle<Material> handle = AssetManager::load<Material>(absPath);
						component.handle = handle;
					}
				}

				// -- Drag drop on button --
				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
						std::string relPath = static_cast<const char*>(payload->Data);
						std::string absPath = AssetManager::getAbsolute(relPath);
						if (absPath.find(".axmat") != std::string::npos) {
							AssetHandle<Material> handle = AssetManager::load<Material>(absPath);
							component.handle = handle;
						}
					}
					ImGui::EndDragDropTarget();
				}

			}
		});

		// ----- ConstantBufferComponent -----
		drawComponentInfo<ConstantBufferComponent>("Upload Buffer", m_selectedEntity, [this]() {
			auto& component = m_selectedEntity.getComponent<ConstantBufferComponent>();
			if (component.uploadBuffer) {
				ImGui::Text("Upload size: %s", std::to_string(component.uploadBuffer->getSize()).c_str());
			}
			else {
				component.uploadBuffer = ConstantBuffer::create(sizeof(ObjectBuffer));
			}

		});

		// ----- CameraComponent -----
		drawComponentInfo<CameraComponent>("Camera", m_selectedEntity, []() {});

	}

	bool SceneHierarchyPanel::onSceneChanged(SceneChangedEvent& e) {
		setContext(SceneManager::getScene());
		return false;
	}

}
