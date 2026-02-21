#include "SceneHierarchyPanel.h"

#include "AxionEngine/Vendor/imgui/imgui.h"
#include "AxionEngine/Vendor/imgui/imgui_internal.h"

#include "AxionEngine/Source/scene/Components.h"
#include "AxionEngine/Source/scene/SceneManager.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/EnumUtils.h"
#include "AxionEngine/Source/render/GraphicsContext.h"
#include "AxionEngine/Source/render/Renderer3D.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionStudio/Source/core/EditorResourceManager.h"

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
			m_selectedEntity.hasComponent<SpriteComponent>() &&
			m_selectedEntity.hasComponent<MaterialComponent>() &&
			m_selectedEntity.hasComponent<CameraComponent>() &&
			m_selectedEntity.hasComponent<AudioComponent>() &&
			m_selectedEntity.hasComponent<NativeScriptComponent>() &&
			m_selectedEntity.hasComponent<DirectionalLightComponent>() &&
			m_selectedEntity.hasComponent<PointLightComponent>() &&
			m_selectedEntity.hasComponent<SpotLightComponent>();

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
			drawAddComponent<SpriteComponent>("Sprite");
			drawAddComponent<MaterialComponent>("Material");
			drawAddComponent<CameraComponent>("Camera");
			drawAddComponent<AudioComponent>("Audio");
			drawAddComponent<NativeScriptComponent>("Native Script");
			drawAddComponent<DirectionalLightComponent>("Directional Light");
			drawAddComponent<PointLightComponent>("Point Light");
			drawAddComponent<SpotLightComponent>("Spot Light");

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
				drawVec3Control("Rotation", rotation, 0.0f, 0.0f, 0.0f, 70.0f);
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

		// ----- SpriteComponent -----
		drawComponentInfo<SpriteComponent>("Sprite", m_selectedEntity, [this]() {
			auto& component = m_selectedEntity.getComponent<SpriteComponent>();
			if (component.texture.isValid()) {
				if (ImGui::BeginTable("SpriteTable", 2, ImGuiTableFlags_BordersInnerV)) {
					Ref<Texture2D> sprite = AssetManager::get<Texture2D>(component.texture);
					ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 120.0f);
					ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

					// -- UUID --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("UUID");
					ImGui::Separator();
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
			}
			else {
				// -- Load Button --
				if (ImGui::Button("Open Texture2D...")) {
					std::filesystem::path texDir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "textures";
					std::string absPath = FileDialogs::openFile({ {"Axion Texture Asset", "*.axtex"} }, texDir.string());
					if (!absPath.empty()) {
						AssetHandle<Texture2D> handle = AssetManager::load<Texture2D>(absPath);
						component.texture = handle;
					}
				}

				// -- Drag drop on button --
				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
						std::string relPath = static_cast<const char*>(payload->Data);
						std::string absPath = AssetManager::getAbsolute(relPath);
						if (absPath.find(".axtex") != std::string::npos) {
							AssetHandle<Texture2D> handle = AssetManager::load<Texture2D>(absPath);
							component.texture = handle;
						}
					}
					ImGui::EndDragDropTarget();
				}
			}

			ImGui::ColorPicker4("##ColorPickerTint", component.tint.data());

			});

		// ----- MaterialComponent -----
		drawComponentInfo<MaterialComponent>("Material", m_selectedEntity, [this]() {
			auto& component = m_selectedEntity.getComponent<MaterialComponent>();
			if (component.handle.isValid()) {
				if (ImGui::BeginTable("MaterialTable", 2, ImGuiTableFlags_BordersInnerV)) {
					ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 120.0f);
					ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

					// -- Name --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Name");
					ImGui::Separator();
					ImGui::TableSetColumnIndex(1);
					ImGui::Text(AssetManager::get<Material>(component.handle)->getName().c_str());
					
					// -- Pipeline --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Pipeline (Shader)");
					ImGui::TableSetColumnIndex(1);
					ImGui::Text(AssetManager::get<Pipeline>(AssetManager::get<Material>(component.handle)->getPipelineHandle())->getSpecification().shader->getName().c_str());
					
					// -- AlbedoColor --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Albedo Color");
					ImGui::Separator();
					ImGui::TableSetColumnIndex(1);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					ImGui::ColorEdit4("##ColorEdit", AssetManager::get<Material>(component.handle)->getAlbedoColor().data());
					
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
				ImGui::Separator();
				ImGui::TableSetColumnIndex(1);
				ImGui::Checkbox("##Primary_check", &component.isPrimary);

				// -- Fixed aspect ratio toggle --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Fixed Aspect Ratio");
				ImGui::Separator();
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

					// -- Name --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Name");
					ImGui::TableSetColumnIndex(1);
					std::filesystem::path path = std::filesystem::path(AssetManager::get<AudioClip>(component.audio->getClipHandle())->getPath());
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
					std::string mode = EnumUtils::toString(AssetManager::get<AudioClip>(component.audio->getClipHandle())->getMode());
					ImGui::Text(mode.c_str());

					// -- Playback controls --
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Playback");
					ImGui::TableSetColumnIndex(1);
					if (ImGui::Button("Play")) component.audio->play();
					ImGui::SameLine();
					if (ImGui::Button("Stop")) component.audio->stop();
					ImGui::SameLine();
					if (component.audio->isPaused()) {
						if (ImGui::Button("Resume")) component.audio->resume();
					} else {
						if (ImGui::Button("Pause")) component.audio->pause();
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

					// -- Velocity --
					// TODO: add velocity

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
						AssetHandle<AudioClip> handle = AssetManager::load<AudioClip>(absPath);
						component.audio = std::make_shared<AudioSource>(handle);
					}
				}

				// -- Drag drop on button --
				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
						std::string relPath = static_cast<const char*>(payload->Data);
						std::string absPath = AssetManager::getAbsolute(relPath);
						if (absPath.find(".axaudio") != std::string::npos) {
							AssetHandle<AudioClip> handle = AssetManager::load<AudioClip>(absPath);
							component.audio = std::make_shared<AudioSource>(handle);
						}
					}
					ImGui::EndDragDropTarget();
				}

			}
		});

		// ----- NativeScriptComponent -----
		drawComponentInfo<NativeScriptComponent>("Native Script", m_selectedEntity, [this]() {

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
				ImGui::Separator();
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
				ImGui::Separator();
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
				ImGui::Separator();
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::DragFloat("##PLRadiusDrag", &component.radius, 1.0f, 0.0f, 100.0f);

				// -- Falloff --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Falloff");
				ImGui::Separator();
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
				ImGui::Separator();
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
				ImGui::Separator();
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::DragFloat("##SLRangeDrag", &component.range, 1.0f, 0.0f, 100.0f);

				// -- InnerConeAngle --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Inner Cone Angle");
				ImGui::Separator();
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::DragFloat("##SLICADrag", &component.innerConeAngle, 0.5f, 0.0f, 20.0f);

				// -- OuterConeAngle --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Outer Cone Angle");
				ImGui::Separator();
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::DragFloat("##SLOCADrag", &component.outerConeAngle, 0.5f, 0.0f, 20.0f);

				ImGui::EndTable();
			}
		});

	}

	bool SceneHierarchyPanel::onSceneChanged(SceneChangedEvent& e) {
		setContext(SceneManager::getScene());
		return false;
	}

}
