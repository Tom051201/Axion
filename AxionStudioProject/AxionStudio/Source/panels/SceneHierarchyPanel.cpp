#include "SceneHierarchyPanel.h"

#include "AxionEngine/Vendor/imgui/imgui.h"
#include "AxionEngine/Vendor/imgui/imgui_internal.h"

#include "AxionEngine/Source/scene/Components.h"
#include "AxionEngine/Source/scene/SceneManager.h"
#include "AxionEngine/Source/scene/SceneSerializer.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/EnumUtils.h"
#include "AxionEngine/Source/render/GraphicsContext.h"
#include "AxionEngine/Source/render/Renderer3D.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionEngine/Source/scripting/ScriptEngine.h"

#include "AxionStudio/Source/core/EditorResourceManager.h"

#include "AxionAssetPipeline/Source/AxPrefab.h"

namespace Axion {

	SceneHierarchyPanel::SceneHierarchyPanel(const std::string& name, const Ref<Scene>& activeScene) : Panel(name) {
		setContext(activeScene);
	}

	SceneHierarchyPanel::~SceneHierarchyPanel() {
		shutdown();
	}

	void SceneHierarchyPanel::setup() {
		EditorResourceManager::loadIcon("AddComponentIcon", "AxionStudio/Resources/scenehierarchy/AddComponentIcon.png");
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
		// ----- Draw info when no project is selected -----
		if (!ProjectManager::hasProject()) {
			ImGui::Begin("Properties");
			ImGui::TextWrapped("No Project Loaded. \nPlease load or create a project first.");
			ImGui::End();

			ImGui::Begin("Scene Hierarchy");
			ImGui::TextWrapped("No Project Loaded. \nPlease load or create a project first.");
			ImGui::End();

			return;
		}


		// ----- Properties Panel -----
		if (ImGui::Begin("Properties")) {
			if (m_selectedEntity) {
				displayComponents(m_selectedEntity);
			}
			else {
				m_selectedEntity = {};
			}
		}
		ImGui::End();

		// ----- Scene Hierarchy Panel -----
		if (ImGui::Begin("Scene Hierarchy")) {

			for (auto e : m_context->getRegistry().view<TagComponent>()) {
				Entity entity{ e, m_context.get() };

				bool isRoot = true;
				if (entity.hasComponent<RelationshipComponent>()) {
					if (entity.getComponent<RelationshipComponent>().parent != entt::null) {
						isRoot = false;
					}
				}

				if (isRoot) {
					displayEntity(entity);
				}
			}

			if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
				m_selectedEntity = {};
			}

			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_ENTITY")) {
					Entity droppedEntity = *(Entity*)payload->Data;

					// -- Remove from old parent --
					if (droppedEntity.hasComponent<RelationshipComponent>()) {
						auto& rel = droppedEntity.getComponent<RelationshipComponent>();
						if (rel.parent != entt::null) {
							Entity parent = { rel.parent, m_context.get() };
							auto& parentRel = parent.getComponent<RelationshipComponent>();
							auto it = std::find(parentRel.children.begin(), parentRel.children.end(), (entt::entity)droppedEntity);
							if (it != parentRel.children.end()) parentRel.children.erase(it);
							rel.parent = entt::null;
						}
					}
				}
				ImGui::EndDragDropTarget();
			}

		}

		// ----- Right click on blank space -----
		if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
			if (ImGui::MenuItem("Empty Entity")) { m_context->createEntity("Empty Entity"); }
			ImGui::EndPopup();
		}
		ImGui::End();

	}

	void SceneHierarchyPanel::drawVec3Control(const std::string& label, Vec3& values, float resetX, float resetY, float resetZ, float columnWidth) {
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
		if (ImGui::Button("X", buttonSize)) { values.x = resetX; }
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		if (ImGui::Button("Y", buttonSize)) { values.y = resetY; }
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		if (ImGui::Button("Z", buttonSize)) { values.z = resetZ; }
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

		// -- Check if has children entities --
		bool hasChildren = false;
		if (entity.hasComponent<RelationshipComponent>()) {
			hasChildren = !entity.getComponent<RelationshipComponent>().children.empty();
		}

		// -- Setup flags --
		ImGuiTreeNodeFlags flags = ((m_selectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
		if (!hasChildren) {
			flags |= ImGuiTreeNodeFlags_Leaf;
		}

		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, name.c_str());

		if (ImGui::IsItemClicked()) { m_selectedEntity = entity; }

		// -- Drag and drop source - picking up this entity --
		if (ImGui::BeginDragDropSource()) {
			ImGui::SetDragDropPayload("SCENE_HIERARCHY_ENTITY", &entity, sizeof(Entity));
			ImGui::Text("%s", name.c_str());
			ImGui::EndDragDropSource();
		}

		// -- Drag and drop target - dropping another entity onto this one --
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_ENTITY")) {
				Entity droppedEntity = *(Entity*)payload->Data;

				// -- Prevent infinite circular loops --
				bool isDescendant = false;
				Entity current = entity;
				while (current) {
					if (current == droppedEntity) {
						isDescendant = true;
						break;
					}
					current = current.getParent();
				}

				if (!isDescendant) {
					if (droppedEntity.hasComponent<RelationshipComponent>()) {
						auto& rel = droppedEntity.getComponent<RelationshipComponent>();
						if (rel.parent != entt::null) {
							Entity parent = { rel.parent, m_context.get() };
							auto& parentRel = parent.getComponent<RelationshipComponent>();
							auto it = std::find(parentRel.children.begin(), parentRel.children.end(), (entt::entity)droppedEntity);
							if (it != parentRel.children.end()) parentRel.children.erase(it);
						}
					}

					droppedEntity.setParent(entity);
				}
			}
			ImGui::EndDragDropTarget();
		}

		// -- On right click --
		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem()) {

			// -- Delete entity --
			if (ImGui::MenuItem("Delete Entity")) { entityDeleted = true; }

			// -- Add an entity to it as a child --
			if (ImGui::MenuItem("Add Child")) {
				Entity child = m_context->createEntity("Child Entity");
				child.setParent(entity);
			}

			// -- Create prefab --
			ImGui::Separator();
			if (ImGui::MenuItem("Create Prefab")) {

				std::filesystem::path prefabDir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "prefabs";
				std::filesystem::create_directories(prefabDir);

				std::string savePath = FileDialogs::saveFile({ {"Axion Prefab Asset", ".axprefab"} }, prefabDir.string());

				if (!savePath.empty()) {
					UUID newAssetUUID = UUID::generate();

					AAP::PrefabAssetData data;
					data.uuid = newAssetUUID;
					data.name = entity.getComponent<TagComponent>().tag;
					data.scene = m_context;
					data.entity = entity;

					AAP::PrefabParser::createTextFile(data, savePath);

					AssetMetadata metadata;
					metadata.handle = newAssetUUID;
					metadata.type = AssetType::Prefab;
					metadata.filePath = AssetManager::getRelativeToAssets(savePath);

					auto registry = ProjectManager::getProject()->getAssetRegistry();
					registry->add(metadata);
					registry->serialize((std::filesystem::path(ProjectManager::getProject()->getProjectPath()) / "AssetRegistry.yaml").string());
				}
			}

			ImGui::EndPopup();
		}

		// --- Recursively draw children ---
		if (opened) {
			if (hasChildren) {
				auto childrenCopy = entity.getComponent<RelationshipComponent>().children;
				for (auto child : childrenCopy) {
					Entity childEntity = { child, m_context.get() };
					displayEntity(childEntity);
				}
			}
			ImGui::TreePop();
		}

		// ----- Deleting Entities -----
		if (entityDeleted) {
			// -- Remove ourselves from our parents children list --
			if (entity.hasComponent<RelationshipComponent>()) {
				auto& rel = entity.getComponent<RelationshipComponent>();
				if (rel.parent != entt::null) {
					Entity parent = { rel.parent, m_context.get() };
					auto& parentRel = parent.getComponent<RelationshipComponent>();
					auto it = std::find(parentRel.children.begin(), parentRel.children.end(), (entt::entity)entity);
					if (it != parentRel.children.end()) parentRel.children.erase(it);
				}

				// -- Unparent all our children --
				for (auto childHandle : rel.children) {
					Entity childEntity = { childHandle, m_context.get() };
					if (childEntity.hasComponent<RelationshipComponent>()) {
						childEntity.getComponent<RelationshipComponent>().parent = entt::null;
					}
				}
			}

			// -- Queue for deletion --
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
			m_selectedEntity.hasComponent<SpriteComponent>() &&
			m_selectedEntity.hasComponent<MaterialComponent>() &&
			m_selectedEntity.hasComponent<CameraComponent>() &&
			m_selectedEntity.hasComponent<AudioComponent>() &&
			m_selectedEntity.hasComponent<NativeScriptComponent>() &&
			m_selectedEntity.hasComponent<DirectionalLightComponent>() &&
			m_selectedEntity.hasComponent<PointLightComponent>() &&
			m_selectedEntity.hasComponent<SpotLightComponent>() &&
			m_selectedEntity.hasComponent<RigidBodyComponent>() &&
			m_selectedEntity.hasComponent<BoxColliderComponent>() &&
			m_selectedEntity.hasComponent<SphereColliderComponent>() &&
			m_selectedEntity.hasComponent<CapsuleColliderComponent>() &&
			m_selectedEntity.hasComponent<GravitySourceComponent>() &&
			m_selectedEntity.hasComponent<ScriptComponent>() &&
			m_selectedEntity.hasComponent<ParticleSystemComponent>();

		ImGui::SameLine();
		ImGui::BeginDisabled(hasAll);
		void* texID = GraphicsContext::get()->getImGuiTextureID(EditorResourceManager::getIcon("AddComponentIcon"));
		if (ImGui::ImageButton("##AddComponent_button", (ImTextureID)texID, {18, 18}, {0, 1}, {1, 0})) {
			ImGui::OpenPopup("AddComponent");
		}
		ImGui::EndDisabled();
		if (ImGui::BeginPopup("AddComponent")) {

			drawAddComponent<TransformComponent>("Transform");
			drawAddComponent<MeshComponent>("Mesh");
			drawAddComponent<MaterialComponent>("Material");
			drawAddComponent<SpriteComponent>("Sprite");
			drawAddComponent<CameraComponent>("Camera");
			drawAddComponent<AudioComponent>("Audio");
			if (!m_selectedEntity.hasComponent<DirectionalLightComponent>() ||
				!m_selectedEntity.hasComponent<PointLightComponent>() ||
				!m_selectedEntity.hasComponent<SpotLightComponent>()) {
				if (ImGui::BeginMenu("Lights##_menu")) {
					drawAddComponent<DirectionalLightComponent>("Directional Light");
					drawAddComponent<PointLightComponent>("Point Light");
					drawAddComponent<SpotLightComponent>("Spot Light");
					ImGui::EndMenu();
				}
			}
			
			if (!m_selectedEntity.hasComponent<RigidBodyComponent>() ||
				!m_selectedEntity.hasComponent<BoxColliderComponent>() ||
				!m_selectedEntity.hasComponent<SphereColliderComponent>() ||
				!m_selectedEntity.hasComponent<CapsuleColliderComponent>() ||
				!m_selectedEntity.hasComponent<GravitySourceComponent>()) {
				if (ImGui::BeginMenu("Physics##_menu")) {
					drawAddComponent<RigidBodyComponent>("Rigid Body");
					drawAddComponent<BoxColliderComponent>("Box Collider");
					drawAddComponent<SphereColliderComponent>("Sphere Collider");
					drawAddComponent<CapsuleColliderComponent>("Capsule Collider");
					drawAddComponent<GravitySourceComponent>("Gravity Source");
					ImGui::EndMenu();
				}
			}

			drawAddComponent<NativeScriptComponent>("Native Script");
			drawAddComponent<ScriptComponent>("C# Script");
			drawAddComponent<ParticleSystemComponent>("Particle System");

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
			const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
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

				drawVec3Control("Position", position, 0.0f, 0.0f, 0.0f, 70.0f);
				drawVec3Control("Rotation", rotation.toEulerAngles(), 0.0f, 0.0f, 0.0f, 70.0f);
				drawVec3Control("Scale", scale, 1.0f, 1.0f, 1.0f, 70.0f);

				ImGui::TreePop();
			}
			ImGui::PopID();
		}

		// ----- MeshComponent -----
		drawComponentInfo<MeshComponent>("Mesh", m_selectedEntity, [this]() {
			auto& component = m_selectedEntity.getComponent<MeshComponent>();

			if (component.handle.isValid()) {

				if (ImGui::BeginTable("MeshTable", 2, ImGuiTableFlags_BordersInnerV)) {
					Ref<Mesh> mesh = AssetManager::get<Mesh>(component.handle);
					ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 120.0f);
					ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

					// -- UUID --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("UUID");
					ImGui::Separator();
					ImGui::TableSetColumnIndex(1);
					ImGui::Text(component.handle.uuid.toString().c_str());

					// -- Vertices --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Vertices");
					ImGui::TableSetColumnIndex(1);
					ImGui::Text("%u", mesh->getVertexBuffer()->getVertexCount());

					// -- Indices --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Indices");
					ImGui::Separator();
					ImGui::TableSetColumnIndex(1);
					ImGui::Text("%u", mesh->getIndexCount());

					// -- Options --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Options");
					ImGui::TableSetColumnIndex(1);
					if (ImGui::Button("Remove")) {
						component.handle.invalidate();
					}

					ImGui::EndTable();
				}
			}
			else {
				// -- Load Button --
				if (ImGui::Button("Open Mesh...")) {
					std::filesystem::path meshDir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "meshes";
					std::string absPath = FileDialogs::openFile({ {"Axion Mesh Asset", "*.axmesh"} }, meshDir.string());
					if (!absPath.empty()) {
						UUID assetUUID = AssetManager::getAssetUUID(absPath);
						if (assetUUID.isValid()) component.handle = AssetManager::load<Mesh>(assetUUID);
					}
				}

				// -- Drag drop on button --
				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
						std::string relPath = static_cast<const char*>(payload->Data);
						std::string absPath = AssetManager::getAbsolute(relPath);
						if (absPath.find(".axmesh") != std::string::npos) {
							UUID assetUUID = AssetManager::getAssetUUID(absPath);
							if (assetUUID.isValid()) component.handle = AssetManager::load<Mesh>(assetUUID);
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
				if (ImGui::BeginTable("MaterialTable", 2, ImGuiTableFlags_BordersInnerV)) {
					ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 120.0f);
					ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

					Ref<Material> material = AssetManager::get<Material>(component.handle);
					Ref<Pipeline> pipeline = AssetManager::get<Pipeline>(material->getPipelineHandle());

					// -- Name --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Name");
					ImGui::Separator();
					ImGui::TableSetColumnIndex(1);
					ImGui::Text(material->getName().c_str());
					
					// -- Pipeline --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Pipeline (Shader)");
					ImGui::TableSetColumnIndex(1);
					if (pipeline != nullptr) {
						ImGui::Text(pipeline->getSpecification().shader->getName().c_str());
					}
					else {
						ImGui::Text("Internal Default Pipeline");
					}

					// -- AlbedoColor --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Albedo Color");
					ImGui::Separator();
					ImGui::TableSetColumnIndex(1);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					ImGui::ColorEdit4("##ColorEdit", material->getAlbedoColor().data());
					
					// -- Options --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Options");
					ImGui::TableSetColumnIndex(1);
					if (ImGui::Button("Remove")) {
						component.handle.invalidate();
					}

					ImGui::EndTable();
				}
			}
			else {
				// -- Load Button --
				if (ImGui::Button("Open Material...")) {
					std::filesystem::path materialDir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "materials";
					std::string absPath = FileDialogs::openFile({ {"Axion Material Asset", "*.axmat"} }, materialDir.string());
					if (!absPath.empty()) {
						UUID assetUUID = AssetManager::getAssetUUID(absPath);
						if (assetUUID.isValid()) component.handle = AssetManager::load<Material>(assetUUID);
					}
				}

				// -- Drag drop on button --
				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
						std::string relPath = static_cast<const char*>(payload->Data);
						std::string absPath = AssetManager::getAbsolute(relPath);
						if (absPath.find(".axmat") != std::string::npos) {
							UUID assetUUID = AssetManager::getAssetUUID(absPath);
							if (assetUUID.isValid()) component.handle = AssetManager::load<Material>(assetUUID);
						}
					}
					ImGui::EndDragDropTarget();
				}

			}

		});

		// ----- SpriteComponent -----
		drawComponentInfo<SpriteComponent>("Sprite", m_selectedEntity, [this]() {
			auto& component = m_selectedEntity.getComponent<SpriteComponent>();

			if (ImGui::BeginTable("SpriteTable", 2, ImGuiTableFlags_BordersInnerV)) {
				Ref<Texture2D> sprite = AssetManager::get<Texture2D>(component.texture);
				ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 120.0f);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

				// -- Color --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Color");
				ImGui::Separator();
				ImGui::TableSetColumnIndex(1);
				ImGui::ColorEdit4("##ColorEditTint", component.tint.data());

				if (component.texture.isValid()) {
					// -- UUID --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Texture UUID");
					ImGui::TableSetColumnIndex(1);
					ImGui::Text(component.texture.uuid.toString().c_str());

					// -- Options --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Options");
					ImGui::TableSetColumnIndex(1);
					if (ImGui::Button("Remove")) {
						component.texture.invalidate();
					}

					ImGui::EndTable();
				}
				else {
					ImGui::EndTable();

					// -- Load Button --
					if (ImGui::Button("Open Texture2D...")) {
						std::filesystem::path texDir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "textures";
						std::string absPath = FileDialogs::openFile({ {"Axion Texture Asset", "*.axtex"} }, texDir.string());
						if (!absPath.empty()) {
							UUID assetUUID = AssetManager::getAssetUUID(absPath);
							if (assetUUID.isValid()) component.texture = AssetManager::load<Texture2D>(assetUUID);
						}
					}

					// -- Drag drop on button --
					if (ImGui::BeginDragDropTarget()) {
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
							std::string relPath = static_cast<const char*>(payload->Data);
							std::string absPath = AssetManager::getAbsolute(relPath);
							if (absPath.find(".axtex") != std::string::npos) {
								UUID assetUUID = AssetManager::getAssetUUID(absPath);
								if (assetUUID.isValid()) component.texture = AssetManager::load<Texture2D>(assetUUID);
							}
						}
						ImGui::EndDragDropTarget();
					}
				}
			}

		});

		// ----- CameraComponent -----
		drawComponentInfo<CameraComponent>("Camera", m_selectedEntity, [this]() {
			auto& component = m_selectedEntity.getComponent<CameraComponent>();

			if (ImGui::BeginTable("CameraTable", 2, ImGuiTableFlags_BordersInnerV)) {
				ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 120.0f);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

				// -- Primary toggle --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Primary");
				ImGui::TableSetColumnIndex(1);
				ImGui::Checkbox("##Primary_check", &component.isPrimary);

				// -- Fixed aspect ratio toggle --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Fixed Aspect Ratio");
				ImGui::TableSetColumnIndex(1);
				ImGui::Checkbox("##FixedAspectRatio_check", &component.fixedAspectRatio);

				ImGui::EndTable();
			}

		});

		// ----- AudioComponent -----
		drawComponentInfo<AudioComponent>("Audio", m_selectedEntity, [this]() {
			auto& component = m_selectedEntity.getComponent<AudioComponent>();

			if (component.audio != nullptr) {
				if (ImGui::BeginTable("AudioTable", 2, ImGuiTableFlags_BordersInnerV)) {
					ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 120.0f);
					ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

					Ref<AudioClip> clip = AssetManager::get<AudioClip>(component.audio->getClipHandle());

					// -- Name --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Name");
					ImGui::TableSetColumnIndex(1);
					std::filesystem::path path = std::filesystem::path(clip->getPath());
					std::string name = path.filename().string();
					ImGui::Text(name.c_str());

					// -- UUID --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("UUID");
					ImGui::TableSetColumnIndex(1);
					std::string uuid = component.audio->getClipHandle().uuid.toString();
					ImGui::Text(uuid.c_str());

					// -- Mode --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Mode");
					ImGui::Separator();
					ImGui::TableSetColumnIndex(1);
					std::string mode = EnumUtils::toString(clip->getMode());
					ImGui::Text(mode.c_str());

					// -- Playback controls --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Playback");
					ImGui::TableSetColumnIndex(1);
					if (ImGui::Button("Play")) {
						component.audio->play();
					}
					ImGui::SameLine();
					if (ImGui::Button("Stop")) {
						component.audio->stop();
					}
					ImGui::SameLine();
					if (component.audio->isPaused()) {
						if (ImGui::Button("Resume")) {
							component.audio->resume();
						}
					} else {
						if (ImGui::Button("Pause")) {
							component.audio->pause();
						}
					}

					// -- Volume --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Volume");
					ImGui::TableSetColumnIndex(1);
					float vol = component.audio->getVolume();
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					if (ImGui::DragFloat("##Volume_drag", &vol, 0.05f, 0.0f, 4.0f, "%.2f")) {
						component.audio->setVolume(vol);
					}
					
					// -- Pitch --
					ImGui::TableNextColumn();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Pitch");
					ImGui::TableSetColumnIndex(1);
					float pitch = component.audio->getPitch();
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					if (ImGui::DragFloat("##Pitch_drag", &pitch, 0.05f, 0.0f, 4.0f, "%.2f")) {
						component.audio->setPitch(pitch);
					}

					// -- Pan --
					ImGui::TableNextColumn();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Pan");
					ImGui::TableSetColumnIndex(1);
					float pan = component.audio->getPan();
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					if (ImGui::DragFloat("##Pan_drag", &pan, 0.05f, -1.0f, 1.0f, "%.2f")) {
						component.audio->setPan(pan);
					}

					// -- Looping toggle --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Loop");
					ImGui::Separator();
					ImGui::TableSetColumnIndex(1);
					bool loop = component.audio->isLooping();
					if (ImGui::Checkbox("##Loop_check", &loop)) {
						if (loop) component.audio->loop(true);
						else component.audio->loop(false);
					}

					// -- Spatialization toggle --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Spatialize");
					ImGui::TableSetColumnIndex(1);
					bool spatial = component.audio->isSpatial();
					if (ImGui::Checkbox("##Spatial_check", &spatial)) {
						if (spatial) component.audio->enableSpatial();
						else component.audio->disableSpatial();
					}

					// -- Min distance --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Min Distance");
					ImGui::TableSetColumnIndex(1);
					float minDist = component.audio->getMinDistance();
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					if (ImGui::DragFloat("##MinDistance_drag", &minDist, 0.2f, 0.0f, 100.0f)) {
						component.audio->setMinDistance(minDist);
					}

					// -- Max distance --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Max Distance");
					ImGui::TableSetColumnIndex(1);
					float maxDist = component.audio->getMaxDistance();
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					if (ImGui::DragFloat("##MaxDistance_drag", &maxDist, 0.2f, 1.0f, 500.0f)) {
						component.audio->setMaxDistance(maxDist);
					}

					// -- Doppler --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Doppler");
					ImGui::Separator();
					ImGui::TableSetColumnIndex(1);
					float doppler = component.audio->getDopplerFactor();
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					if (ImGui::DragFloat("##Doppler_drag", &doppler, 0.05f, 0.0f, 4.0f)) {
						component.audio->setDopplerFactor(doppler);
					}

					// -- Options --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Options");
					ImGui::TableSetColumnIndex(1);
					if (ImGui::Button("Remove")) {
						component.audio = nullptr;
					}

					ImGui::EndTable();
				}
			}
			else {
				// -- Load Button --
				if (ImGui::Button("Load Audio Clip...")) {
					std::filesystem::path audioDir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "audio";
					std::string absPath = FileDialogs::openFile({ {"Axion Audio Asset", "*.axaudio"} }, audioDir.string());
					if (!absPath.empty()) {
						UUID assetUUID = AssetManager::getAssetUUID(absPath);
						if (assetUUID.isValid()) {
							AssetHandle<AudioClip> clipHandle = AssetManager::load<AudioClip>(assetUUID);
							component.audio = std::make_shared<AudioSource>(clipHandle);
						}
					}
				}

				// -- Drag drop on button --
				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
						std::string relPath = static_cast<const char*>(payload->Data);
						std::string absPath = AssetManager::getAbsolute(relPath);
						if (absPath.find(".axaudio") != std::string::npos) {
							UUID assetUUID = AssetManager::getAssetUUID(absPath);
							if (assetUUID.isValid()) {
								AssetHandle<AudioClip> clipHandle = AssetManager::load<AudioClip>(assetUUID);
								component.audio = std::make_shared<AudioSource>(clipHandle);
							}
						}
					}
					ImGui::EndDragDropTarget();
				}

			}

		});

		// ----- DirectionalLightComponent -----
		drawComponentInfo<DirectionalLightComponent>("Directional Light", m_selectedEntity, [this]() {
			auto& component = m_selectedEntity.getComponent<DirectionalLightComponent>();

			if (ImGui::BeginTable("DirectionalLightTable", 2, ImGuiTableFlags_BordersInnerV)) {
				ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 120.0f);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

				// -- Color --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Color");
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::ColorEdit4("##DLColorEdit", component.color.data());

				ImGui::EndTable();
			}

		});

		// ----- PointLightComponent -----
		drawComponentInfo<PointLightComponent>("Point Light", m_selectedEntity, [this]() {
			auto& component = m_selectedEntity.getComponent<PointLightComponent>();

			if (ImGui::BeginTable("PointLightTable", 2, ImGuiTableFlags_BordersInnerV)) {
				ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 120.0f);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

				// -- Color --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Color");
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::ColorEdit4("##PLColorEdit", component.color.data());

				// -- Intensity --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Intensity");
				ImGui::Separator();
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::DragFloat("##PLIntensityDrag", &component.intensity, 0.5f, 0.0f, 20.0f);

				// -- Radius --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Radius");
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::DragFloat("##PLRadiusDrag", &component.radius, 1.0f, 0.0f, 100.0f);

				// -- Falloff --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Falloff");
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::DragFloat("##PLFalloffDrag", &component.falloff, 0.5f, 0.0f, 20.0f);

				ImGui::EndTable();
			}

		});

		// ----- SpotLightComponent -----
		drawComponentInfo<SpotLightComponent>("Spot Light", m_selectedEntity, [this]() {
			auto& component = m_selectedEntity.getComponent<SpotLightComponent>();

			if (ImGui::BeginTable("SpotLightTable", 2, ImGuiTableFlags_BordersInnerV)) {
				ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 120.0f);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

				// -- Color --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Color");
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::ColorEdit4("##SLColorEdit", component.color.data());

				// -- Intensity --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Intensity");
				ImGui::Separator();
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::DragFloat("##SLIntensityDrag", &component.intensity, 0.5f, 0.0f, 20.0f);

				// -- Range --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Range");
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::DragFloat("##SLRangeDrag", &component.range, 1.0f, 0.0f, 100.0f);

				// -- InnerConeAngle --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Inner Cone Angle");
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::DragFloat("##SLICADrag", &component.innerConeAngle, 0.5f, 0.0f, 20.0f);

				// -- OuterConeAngle --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Outer Cone Angle");
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::DragFloat("##SLOCADrag", &component.outerConeAngle, 0.5f, 0.0f, 20.0f);

				ImGui::EndTable();
			}

		});

		// ----- RigidBodyComponent -----
		drawComponentInfo<RigidBodyComponent>("Rigid Body", m_selectedEntity, [this]() {
			auto& component = m_selectedEntity.getComponent<RigidBodyComponent>();

			if (ImGui::BeginTable("RigidBodyTable", 2, ImGuiTableFlags_BordersInnerV)) {
				ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 120.0f);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

				// -- Body Type --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Body Type");
				ImGui::TableSetColumnIndex(1);
				int currentItem = (component.type == RigidBodyComponent::BodyType::Static) ? 0 : 1;
				const char* bodyTypes[2] = { "Static", "Dynamic" };
				if (ImGui::Combo("##BodyTypeCombo", &currentItem, bodyTypes, IM_ARRAYSIZE(bodyTypes))) {
					if (currentItem == 0) {
						component.type = RigidBodyComponent::BodyType::Static;
					}
					else {
						component.type = RigidBodyComponent::BodyType::Dynamic;
					}
				}

				// -- IsKinematic --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Kinematic");
				ImGui::TableSetColumnIndex(1);
				ImGui::Checkbox("##kinematic_check", &component.isKinematic);

				// -- UseGlobalGravity --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Use Global Gravity");
				ImGui::TableSetColumnIndex(1);
				ImGui::Checkbox("##useGlobGrav_check", &component.useGlobalGravity);

				// -- EnableCCD --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Enable CCD");
				ImGui::Separator();
				ImGui::TableSetColumnIndex(1);
				ImGui::Checkbox("##enableCCD_check", &component.enableCCD);

				// -- Mass --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Mass");
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::DragFloat("##RBMassDrag", &component.mass, 0.5f, 0.0f, 200.0f);

				// -- LinearDamping --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Linear Damping");
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::DragFloat("##RBLDampingDrag", &component.linearDamping, 0.05f, 0.0f, 1.0f);

				// -- AngularDamping --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Angular Damping");
				ImGui::Separator();
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::DragFloat("##RBADampingDrag", &component.angularDamping, 0.05f, 0.0f, 1.0f);

				// -- FixedRotationX --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Fixed Rotation X");
				ImGui::TableSetColumnIndex(1);
				ImGui::Checkbox("##fixedRotationX_check", &component.fixedRotationX);

				// -- FixedRotationY --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Fixed Rotation Y");
				ImGui::TableSetColumnIndex(1);
				ImGui::Checkbox("##fixedRotationY_check", &component.fixedRotationY);

				// -- FixedRotationZ --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Fixed Rotation Z");
				ImGui::TableSetColumnIndex(1);
				ImGui::Checkbox("##fixedRotationZ_check", &component.fixedRotationZ);

				ImGui::EndTable();
			}

		});

		// ----- BoxColliderComponent -----
		drawComponentInfo<BoxColliderComponent>("Box Collider", m_selectedEntity, [this]() {
			auto& component = m_selectedEntity.getComponent<BoxColliderComponent>();

			if (ImGui::BeginTable("BoxColliderTable", 2, ImGuiTableFlags_BordersInnerV)) {
				ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 120.0f);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

				// -- HalfExtents --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Half Extents");
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				drawVec3Control("##halfExtentsVec", component.halfExtents, 0.0f, 0.0f, 0.0f);

				// -- Offset --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Offset");
				ImGui::Separator();
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				drawVec3Control("##OffsetVec", component.offset, 0.0f, 0.0f, 0.0f);

				// -- Is Trigger --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Is Trigger");
				ImGui::TableSetColumnIndex(1);
				ImGui::Checkbox("##isTriggerbox_check", &component.isTrigger);

				Ref<PhysicsMaterial> material = AssetManager::get<PhysicsMaterial>(component.material);

				if (material) {
					// -- Static Friction --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Static Friction");
					ImGui::TableSetColumnIndex(1);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					ImGui::BeginDisabled();
					ImGui::InputFloat("##StaticFrictionIn", &material->staticFriction);
					ImGui::EndDisabled();

					// -- Dynamic Friction --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Dynamic Friction");
					ImGui::TableSetColumnIndex(1);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					ImGui::BeginDisabled();
					ImGui::InputFloat("##DynamicFrictionIn", &material->dynamicFriction);
					ImGui::EndDisabled();

					// -- Restitution --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Restitution");
					ImGui::TableSetColumnIndex(1);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					ImGui::BeginDisabled();
					ImGui::InputFloat("##RestitutionIn", &material->restitution);
					ImGui::EndDisabled();

					// -- Options --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Options");
					ImGui::TableSetColumnIndex(1);
					if (ImGui::Button("Remove")) {
						component.material.invalidate();
					}

					ImGui::EndTable();
				}
				else {
					ImGui::EndTable();

					// -- Load Button --
					if (ImGui::Button("Load Material...")) {
						std::filesystem::path dir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "physics";
						std::string absPath = FileDialogs::openFile({ {"Axion Physics Material Asset", "*.axpmat"} }, dir.string());
						if (!absPath.empty()) {
							UUID assetUUID = AssetManager::getAssetUUID(absPath);
							if (assetUUID.isValid()) component.material = AssetManager::load<PhysicsMaterial>(assetUUID);
						}
					}

					// -- Drag drop on button --
					if (ImGui::BeginDragDropTarget()) {
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
							std::string relPath = static_cast<const char*>(payload->Data);
							std::string absPath = AssetManager::getAbsolute(relPath);
							if (absPath.find(".axpmat") != std::string::npos) {
								UUID assetUUID = AssetManager::getAssetUUID(absPath);
								if (assetUUID.isValid()) component.material = AssetManager::load<PhysicsMaterial>(assetUUID);
							}
						}
						ImGui::EndDragDropTarget();
					}
				}

			}

		});

		// ----- SphereColliderComponent -----
		drawComponentInfo<SphereColliderComponent>("Sphere Collider", m_selectedEntity, [this]() {
			auto& component = m_selectedEntity.getComponent<SphereColliderComponent>();

			if (ImGui::BeginTable("SphereColliderTable", 2, ImGuiTableFlags_BordersInnerV)) {
				ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 120.0f);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

				// -- Radius --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Radius");
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::DragFloat("####radiusSphereFloat", &component.radius, 0.5f, 0.0f, 100.0f);

				// -- Offset --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Offset");
				ImGui::Separator();
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				drawVec3Control("##OffsetVecSph", component.offset, 0.0f, 0.0f, 0.0f);

				// -- Is Trigger --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Is Trigger");
				ImGui::TableSetColumnIndex(1);
				ImGui::Checkbox("##isTriggersph_check", &component.isTrigger);

				Ref<PhysicsMaterial> material = AssetManager::get<PhysicsMaterial>(component.material);

				if (material) {
					// -- Static Friction --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Static Friction");
					ImGui::TableSetColumnIndex(1);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					ImGui::BeginDisabled();
					ImGui::InputFloat("##StaticFrictionInSph", &material->staticFriction);
					ImGui::EndDisabled();

					// -- Dynamic Friction --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Dynamic Friction");
					ImGui::TableSetColumnIndex(1);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					ImGui::BeginDisabled();
					ImGui::InputFloat("##DyanmicFrictionInSph", &material->dynamicFriction);
					ImGui::EndDisabled();

					// -- Restitution --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Restitution");
					ImGui::TableSetColumnIndex(1);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					ImGui::BeginDisabled();
					ImGui::InputFloat("##RestitutionInSph", &material->restitution);
					ImGui::EndDisabled();

					// -- Options --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Options");
					ImGui::TableSetColumnIndex(1);
					if (ImGui::Button("Remove")) {
						component.material.invalidate();
					}

					ImGui::EndTable();
				}
				else {
					ImGui::EndTable();

					// -- Load Button --
					if (ImGui::Button("Load Material...")) {
						std::filesystem::path dir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "physics";
						std::string absPath = FileDialogs::openFile({ {"Axion Physics Material Asset", "*.axpmat"} }, dir.string());
						if (!absPath.empty()) {
							UUID assetUUID = AssetManager::getAssetUUID(absPath);
							if (assetUUID.isValid()) component.material = AssetManager::load<PhysicsMaterial>(assetUUID);
						}
					}

					// -- Drag drop on button --
					if (ImGui::BeginDragDropTarget()) {
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
							std::string relPath = static_cast<const char*>(payload->Data);
							std::string absPath = AssetManager::getAbsolute(relPath);
							if (absPath.find(".axpmat") != std::string::npos) {
								UUID assetUUID = AssetManager::getAssetUUID(absPath);
								if (assetUUID.isValid()) component.material = AssetManager::load<PhysicsMaterial>(assetUUID);
							}
						}
						ImGui::EndDragDropTarget();
					}
				}

			}

		});

		// ----- CapsuleColliderComponent -----
		drawComponentInfo<CapsuleColliderComponent>("Capsule Collider", m_selectedEntity, [this]() {
			auto& component = m_selectedEntity.getComponent<CapsuleColliderComponent>();

			if (ImGui::BeginTable("CapsuleColliderTable", 2, ImGuiTableFlags_BordersInnerV)) {
				ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 120.0f);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

				// -- Radius --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Radius");
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::DragFloat("##radiusCapsuFloat", &component.radius, 0.5f, 0.0f, 100.0f);

				// -- Half Height --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("half Height");
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::DragFloat("##halfHeightCapsuFloat", &component.halfHeight, 0.5f, 0.0f, 100.0f);

				// -- Offset --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Offset");
				ImGui::Separator();
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				drawVec3Control("##OffsetVecCapsu", component.offset, 0.0f, 0.0f, 0.0f);

				// -- Is Trigger --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Is Trigger");
				ImGui::TableSetColumnIndex(1);
				ImGui::Checkbox("##isTriggercaps_check", &component.isTrigger);

				Ref<PhysicsMaterial> material = AssetManager::get<PhysicsMaterial>(component.material);

				if (material) {
					// -- Static Friction --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Static Friction");
					ImGui::TableSetColumnIndex(1);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					ImGui::BeginDisabled();
					ImGui::InputFloat("##StaticFrictionInCapsu", &material->staticFriction);
					ImGui::EndDisabled();

					// -- Dynamic Friction --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Dynamic Friction");
					ImGui::TableSetColumnIndex(1);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					ImGui::BeginDisabled();
					ImGui::InputFloat("##DyanmicFrictionInCapsu", &material->dynamicFriction);
					ImGui::EndDisabled();

					// -- Restitution --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Restitution");
					ImGui::TableSetColumnIndex(1);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					ImGui::BeginDisabled();
					ImGui::InputFloat("##RestitutionInCapsu", &material->restitution);
					ImGui::EndDisabled();

					// -- Options --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Options");
					ImGui::TableSetColumnIndex(1);
					if (ImGui::Button("Remove")) {
						component.material.invalidate();
					}

					ImGui::EndTable();
				}
				else {
					ImGui::EndTable();

					// -- Load Button --
					if (ImGui::Button("Load Material...")) {
						std::filesystem::path dir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "physics";
						std::string absPath = FileDialogs::openFile({ {"Axion Physics Material Asset", "*.axpmat"} }, dir.string());
						if (!absPath.empty()) {
							UUID assetUUID = AssetManager::getAssetUUID(absPath);
							if (assetUUID.isValid()) component.material = AssetManager::load<PhysicsMaterial>(assetUUID);
						}
					}

					// -- Drag drop on button --
					if (ImGui::BeginDragDropTarget()) {
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
							std::string relPath = static_cast<const char*>(payload->Data);
							std::string absPath = AssetManager::getAbsolute(relPath);
							if (absPath.find(".axpmat") != std::string::npos) {
								UUID assetUUID = AssetManager::getAssetUUID(absPath);
								if (assetUUID.isValid()) component.material = AssetManager::load<PhysicsMaterial>(assetUUID);
							}
						}
						ImGui::EndDragDropTarget();
					}
				}

			}

		});

		// ----- GravitySourceComponent -----
		drawComponentInfo<GravitySourceComponent>("Gravity Source", m_selectedEntity, [this]() {
			auto& component = m_selectedEntity.getComponent<GravitySourceComponent>();

			if (ImGui::BeginTable("GravitySourceTable", 2, ImGuiTableFlags_BordersInnerV)) {
				ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 120.0f);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

				// -- Type --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Directional");
				ImGui::TableSetColumnIndex(1);
				int currentItem = (component.type == GravitySourceComponent::Type::Directional) ? 0 : 1;
				const char* types[2] = { "Directional", "Point" };
				if (ImGui::Combo("##gravTypeCombo", &currentItem, types, IM_ARRAYSIZE(types))) {
					if (currentItem == 0) {
						component.type = GravitySourceComponent::Type::Directional;
					}
					else {
						component.type = GravitySourceComponent::Type::Point;
					}
				}

				// -- AffectKinematic --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Affect Kinematic");
				ImGui::Separator();
				ImGui::TableSetColumnIndex(1);
				ImGui::Checkbox("##affKinGrav_check", &component.affectKinematic);

				// -- Strength --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Strength");
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::DragFloat("##GravStrenDrag", &component.strength, 0.5f, 0.5f, 100.0f);

				// -- Radius --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Radius");
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::DragFloat("##GravRadDrag", &component.radius, 10.0f, 0.0f, 1000.0f);

				ImGui::EndTable();
			}

		});

		// ----- NativeScriptComponent -----
		drawComponentInfo<NativeScriptComponent>("Native Script", m_selectedEntity, [this]() {

		});

		// -- C# ScriptComponent --
		drawComponentInfo<ScriptComponent>("C# Script", m_selectedEntity, [this]() {
			auto& component = m_selectedEntity.getComponent<ScriptComponent>();

			if (ImGui::BeginTable("ScriptComponentTable", 2, ImGuiTableFlags_BordersInnerV)) {
				ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 120.0f);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

				// -- Class Name --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Class Name");
				ImGui::Separator();
				ImGui::TableSetColumnIndex(1);

				char buffer[256];
				memset(buffer, 0, sizeof(buffer));
				strncpy_s(buffer, sizeof(buffer), component.className.c_str(), sizeof(buffer));

				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				if (ImGui::InputText("##ScriptClassName", buffer, sizeof(buffer))) {
					component.className = std::string(buffer);
				}

				// -- State --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("State");
				ImGui::TableSetColumnIndex(1);
				if (component.isInstantiated) {
					ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Running");
				}
				else {
					ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Waiting to start");
				}

				// -- Metadata --
				const auto& fields = ScriptEngine::getScriptFields(component.className);

				if (!fields.empty()) {
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "--- Script Variables ---");
					ImGui::TableSetColumnIndex(1);
					ImGui::Text("");

					// -- PLAY MODE: Read and write live --
					if (component.isInstantiated && component.gcHandle) {
						for (const auto& field : fields) {
							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text(field.name.c_str());
							ImGui::TableSetColumnIndex(1);

							std::string id = "##" + field.name;

							if (field.type == ScriptFieldType::Float) {
								float value = ScriptEngine::getFieldValueFloat(component.gcHandle, field.name);
								ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
								if (ImGui::DragFloat(id.c_str(), &value, 0.1f)) {
									ScriptEngine::setFieldValueFloat(component.gcHandle, field.name, value);
								}
							}
							else if (field.type == ScriptFieldType::Vector3) {
								Vec3 value = ScriptEngine::getFieldValueVector3(component.gcHandle, field.name);
								float valArray[3] = { value.x, value.y, value.z };
								ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
								if (ImGui::DragFloat3(id.c_str(), valArray, 0.1f)) {
									ScriptEngine::setFieldValueVector3(component.gcHandle, field.name, Vec3(valArray[0], valArray[1], valArray[2]));
								}
							}
						}
					}

					// -- EDIT MODE: Show them, but disable editing --
					else {
						ImGui::BeginDisabled();
						for (const auto& field : fields) {
							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text(field.name.c_str());
							ImGui::TableSetColumnIndex(1);

							std::string id = "##" + field.name;

							if (field.type == ScriptFieldType::Float) {
								float dummy = 0.0f;
								ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
								ImGui::DragFloat(id.c_str(), &dummy);
							}
							else if (field.type == ScriptFieldType::Vector3) {
								float dummy[3] = { 0.0f, 0.0f, 0.0f };
								ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
								ImGui::DragFloat3(id.c_str(), dummy);
							}
						}
						ImGui::EndDisabled();
					}
				}

				ImGui::EndTable();
			}

		});

		// -- ParticleSystemComponent --
		drawComponentInfo<ParticleSystemComponent>("Particle System", m_selectedEntity, [this]() {
			auto& component = m_selectedEntity.getComponent<ParticleSystemComponent>();

			if (ImGui::BeginTable("ParticleSystemTable", 2, ImGuiTableFlags_BordersInnerV)) {
				ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 120.0f);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

				// -- Velocity Variation --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Velocity Var");
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				drawVec3Control("##VelocityVar", component.velocityVariation, 1.0f, 1.0f, 1.0f);

				// -- Color Begin --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Start Color");
				ImGui::Separator();
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::ColorEdit4("##ColorBeginEdit", component.colorBegin.data());

				// -- Color End --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("End Color");
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::ColorEdit4("##ColorEndEdit", component.colorEnd.data());

				// -- Size Begin --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Start Size");
				ImGui::Separator();
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::DragFloat("##SizeBeginDrag", &component.sizeBegin, 0.05f, 0.0f, 10.0f);

				// -- Size End --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("End Size");
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::DragFloat("##SizeEndDrag", &component.sizeEnd, 0.05f, 0.0f, 10.0f);

				// -- Life Time --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Lifetime");
				ImGui::Separator();
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::DragFloat("##LifeTimeDrag", &component.lifeTime, 0.05f, 0.0f, 10.0f);

				// -- Texture --
				if (component.texture.isValid()) {
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Texture UUID");
					ImGui::TableSetColumnIndex(1);
					ImGui::Text(component.texture.uuid.toString().c_str());

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Options");
					ImGui::TableSetColumnIndex(1);
					if (ImGui::Button("Remove Texture")) {
						component.texture.invalidate();
					}
				}
				else {
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Texture");
					ImGui::TableSetColumnIndex(1);
					if (ImGui::Button("Open Texture2D...")) {
						std::filesystem::path texDir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "textures";
						std::string absPath = FileDialogs::openFile({ {"Axion Texture Asset", "*.axtex"} }, texDir.string());
						if (!absPath.empty()) {
							UUID assetUUID = AssetManager::getAssetUUID(absPath);
							if (assetUUID.isValid()) component.texture = AssetManager::load<Texture2D>(assetUUID);
						}
					}

					if (ImGui::BeginDragDropTarget()) {
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
							std::string relPath = static_cast<const char*>(payload->Data);
							std::string absPath = AssetManager::getAbsolute(relPath);
							if (absPath.find(".axtex") != std::string::npos) {
								UUID assetUUID = AssetManager::getAssetUUID(absPath);
								if (assetUUID.isValid()) component.texture = AssetManager::load<Texture2D>(assetUUID);
							}
						}
						ImGui::EndDragDropTarget();
					}
				}

				ImGui::EndTable();
			}
		});



	}

	bool SceneHierarchyPanel::onSceneChanged(SceneChangedEvent& e) {
		setContext(SceneManager::getScene());
		return false;
	}

}
