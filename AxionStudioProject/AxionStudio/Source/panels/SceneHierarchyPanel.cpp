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

	void SceneHierarchyPanel::setup() {

		// TODO: TEMP

		ShaderSpecification shaderSpec;
		shaderSpec.name = "Shader3D";
		shaderSpec.vertexLayout = {
			{ "POSITION", Axion::ShaderDataType::Float3 },
			{ "NORMAL", Axion::ShaderDataType::Float3 },
			{ "TEXCOORD", Axion::ShaderDataType::Float2 }
		};
		Ref<Shader> shader = Shader::create(shaderSpec);
		shader->compileFromFile("AxionStudio/Assets/shaders/PositionShader.hlsl");
		m_basicMaterial = Material::create("BasicMaterial", { 0.0f, 1.0f, 0.0f, 1.0f }, shader);

		// TEMP END
	}

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
		// ----- Properties Panel -----
		if (ImGui::Begin("Properties")) {
			if (m_selectedEntity) {
				displayComponents(m_selectedEntity);

				if (ImGui::Button("Add Component")) ImGui::OpenPopup("AddComponent");
				if (ImGui::BeginPopup("AddComponent")) {

					drawAddComponent<TransformComponent>("Transform");
					drawAddComponent<MeshComponent>("Mesh");
					drawAddComponent<MaterialComponent>("Material");
					drawAddComponent<CameraComponent>("Camera");
					drawAddComponent<ConstantBufferComponent>("Upload Buffer");

					ImGui::EndPopup();
				}

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

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });

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

		ImGui::PopStyleVar();
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
			strcpy_s(buffer, sizeof(buffer), tag.c_str());

			if (ImGui::InputText("Tag", buffer, sizeof(buffer))) {
				tag = std::string(buffer);
			}
		}

		// ----- UUIDComponent -----
		if (entity.hasComponent<UUIDComponent>()) {
			auto& uuid = entity.getComponent<UUIDComponent>().id;
			ImGui::Text(uuid.toString().c_str());
		}

		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;

		// ----- TransformComponent -----
		if (entity.hasComponent<TransformComponent>()) {
			// Draw TransformComponent manual to disable removement
			
			// -- Creates treenode and + button --
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			bool open = ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap, "Transform");
			ImGui::SameLine(ImGui::GetWindowWidth() - 25.0f);
			if (ImGui::Button("+", ImVec2{ 20, 20 })) { ImGui::OpenPopup("ComponentSettings"); }
			ImGui::PopStyleVar();

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

				drawVec3Control("Position", position);
				drawVec3Control("Rotation", rotation);
				drawVec3Control("Scale", scale, 1.0f);

				ImGui::TreePop();
			}
		}

		// ----- MeshComponent -----
		drawComponentInfo<MeshComponent>("Mesh", m_selectedEntity, [this]() {
			auto& component = m_selectedEntity.getComponent<MeshComponent>();
			if (component.mesh) {
				ImGui::Text(component.mesh->getHandle().path.c_str());
			}
			else {
				if (ImGui::Button("Open Mesh...")) {
					std::string filePath = FileDialogs::openFile({ {"OBJ File", "*.obj"} });
					if (!filePath.empty()) {
						AssetHandle<Mesh> handle(filePath);
						if (!AssetManager::hasMesh(handle)) {
							AssetManager::loadMesh(filePath);
						}
						component.mesh = AssetManager::getMesh(handle);
					}
				}

				if (ImGui::BeginDragDropTarget()) {

					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
						std::string path = static_cast<const char*>(payload->Data);
						std::string filePath = ProjectManager::getProject()->getAssetsPath() + "\\" + path;

						// TODO: add validation that its the correct file type

						AssetHandle<Mesh> handle(filePath);
						if (!AssetManager::hasMesh(handle)) {
							AssetManager::loadMesh(filePath);
						}
						component.mesh = AssetManager::getMesh(handle);
					}
					ImGui::EndDragDropTarget();
				}
			}
		});

		// ----- MaterialComponent -----
		drawComponentInfo<MaterialComponent>("Material", m_selectedEntity, [this]() {
			auto& component = m_selectedEntity.getComponent<MaterialComponent>();
			if (component.material) {
				// Material is not a nullptr
				ImGui::Text(component.getName().c_str());
				ImGui::Text(component.material->getShader()->getName().c_str());
				ImGui::ColorPicker4("Color", component.getColor().data());
			}
			else {
				// Material is a nullptr
				ImGui::Text("Unknown Material");
				ImGui::Text("Unknown Shader");
				ImGui::Text("Unknown Color");
				if (ImGui::Button("Load 'Basic Material'")) { component.material = m_basicMaterial; }
			}
		});

		// ----- CameraComponent -----
		drawComponentInfo<CameraComponent>("Camera", m_selectedEntity, []() {
		});

		// ----- ConstantBufferComponent -----
		drawComponentInfo<ConstantBufferComponent>("Upload Buffer", m_selectedEntity, [this]() {
			auto& component = m_selectedEntity.getComponent<ConstantBufferComponent>();
			if (component.uploadBuffer) {
				ImGui::Text(std::to_string(component.uploadBuffer->getSize()).c_str());
			}
			else {
				component.uploadBuffer = ConstantBuffer::create(sizeof(ObjectBuffer));
			}

		});

	}

	bool SceneHierarchyPanel::onSceneChanged(SceneChangedEvent& e) {
		setContext(SceneManager::getScene());

		return false;
	}

}
