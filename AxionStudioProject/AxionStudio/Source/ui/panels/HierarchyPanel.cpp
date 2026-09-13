#include "studiopch.h"
#include "HierarchyPanel.h"

#include <Silica/include/Theme.h>
#include <Silica/include/Renderer.h>
#include <Silica/include/SScrollBox.h>
#include <Silica/include/SVerticalBox.h>
#include <Silica/include/SHorizontalBox.h>
#include <Silica/include/SButton.h>
#include <Silica/include/STextBlock.h>
#include <Silica/include/STreeNode.h>
#include <Silica/include/SMenuAnchor.h>
#include <Silica/include/SBox.h>
#include <Silica/include/SBorderLayout.h>
#include <Silica/include/SAlign.h>

#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/Application.h"
#include "AxionEngine/Source/scene/Components.h"
#include "AxionEngine/Source/scene/SceneManager.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionAssetPipeline/Source/parser/PrefabParser.h"

#include "AxionStudio/Source/core/EditorEvents.h"
#include "AxionStudio/Source/core/EditorCommand.h"
#include "AxionStudio/Source/core/EditorActionQueue.h"
#include "AxionStudio/Source/ui/SilicaHelpers.h"

namespace {
	constexpr float PADDING_SMALL = 5.0f;
	constexpr float SPACING_SMALL = 2.0f;
	constexpr float SPACING_MEDIUM = 5.0f;
	constexpr float BTN_PAD_X = 8.0f;
	constexpr float BTN_PAD_Y = 4.0f;
	constexpr float TREE_NODE_Y_OFFSET = 16.0f;
}

namespace Axion {

	// ----- ECS Logic -----
	void HierarchyPanel::detachEntityFromParent(Entity entity) {
		if (entity.hasComponent<RelationshipComponent>()) {
			auto& rel = entity.getComponent<RelationshipComponent>();
			if (rel.parent != entt::null) {
				Entity parent = { rel.parent, m_scene.get() };
				auto& parentRel = parent.getComponent<RelationshipComponent>();
				auto it = std::find(parentRel.children.begin(), parentRel.children.end(), (entt::entity)entity);
				if (it != parentRel.children.end()) parentRel.children.erase(it);
				rel.parent = entt::null;
			}
		}
	}

	void HierarchyPanel::attachEntityToParent(Entity child, Entity parent) {
		Entity current = parent;
		while (current) {
			if (current == child) return;
			current = current.getParent();
		}

		detachEntityFromParent(child);
		child.setParent(parent);
	}



	// ----- UI Implementation -----
	Silica::WidgetPtr HierarchyPanel::getWidget() {
		if (!m_uiRoot) {
			m_contentBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 0.0f });
			m_uiRoot = Silica::MakeWidget<Silica::SBox>({
				.borderThickness = Silica::GetTheme().Border_Thickness,
				.onDragOver = [](const Silica::DragDropPayload& payload) {
					if (payload.type == "Entity") return Silica::EventReply::handled();
					return Silica::EventReply::unhandled();
				},
				.onDrop = [this](const Silica::DragDropPayload& payload) mutable {
					if (payload.type == "Entity") {
						Entity draggedEntity = std::any_cast<Entity>(payload.data);
						EditorActionQueue::push([this, draggedEntity]() mutable {
							detachEntityFromParent(draggedEntity);
							rebuildUI();
						});
						return Silica::EventReply::handled();
					}
					return Silica::EventReply::unhandled();
				},
				.child = m_contentBox
			});
			rebuildUI();
		}
		return m_uiRoot;
	}

	void HierarchyPanel::setScene(Shared<Scene> scene) {
		m_scene = scene;
		refresh();
	}

	void HierarchyPanel::refresh() {
		EditorActionQueue::push([this]() { rebuildUI(); });
	}

	void HierarchyPanel::onEvent(Event& ev) {
		EventDispatcher dispatcher(ev);
		dispatcher.dispatch<SceneChangedEvent>(AX_BIND_EVENT_FN(HierarchyPanel::onSceneChanged));
		dispatcher.dispatch<ProjectChangedEvent>(AX_BIND_EVENT_FN(HierarchyPanel::onProjectChanged));
		dispatcher.dispatch<EntitySelectedEvent>(AX_BIND_EVENT_FN(HierarchyPanel::onEntitySelected));
		dispatcher.dispatch<EditorHistoryChangedEvent>(AX_BIND_EVENT_FN(HierarchyPanel::onEditorHistoryChanged));
	}

	EventReply HierarchyPanel::onSceneChanged(SceneChangedEvent& ev) { setScene(SceneManager::getScene()); return EventReply::unhandled(); }
	EventReply HierarchyPanel::onProjectChanged(ProjectChangedEvent& ev) { setScene(SceneManager::getScene()); return EventReply::unhandled(); }
	EventReply HierarchyPanel::onEntitySelected(EntitySelectedEvent& ev) { m_selectedEntity = ev.getEntity(); refresh(); return EventReply::unhandled(); }
	EventReply HierarchyPanel::onEditorHistoryChanged(EditorHistoryChangedEvent& ev) { refresh(); return EventReply::unhandled(); }

	Silica::WidgetPtr HierarchyPanel::buildEntityNode(Entity entity) {
		std::string tag = entity.hasComponent<TagComponent>() ? entity.getComponent<TagComponent>().tag : "Unnamed Entity";
		bool hasChildren = entity.hasComponent<RelationshipComponent>() && !entity.getComponent<RelationshipComponent>().children.empty();

		auto MakeCtxBtn = [&](const std::string& text, std::function<void()> action, Silica::Color hoverColor = Silica::GetTheme().Accent_Primary) {
			return Silica::MakeWidget<Silica::SButton>({
				.padding = { BTN_PAD_X, BTN_PAD_Y },
				.color = Silica::Color::transparent(),
				.hoverColor = hoverColor,
				.onClick = [action]() {
					EditorActionQueue::push([action]() {
						Silica::Renderer::closePopups();
						action();
					});
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = text})
			});
		};

		// -- Right Click Context Menu --
		auto contextMenu = Silica::MakeWidget<Silica::SBox>({
			.padding = { PADDING_SMALL, PADDING_SMALL },
			.borderThickness = Silica::GetTheme().Border_Thickness, .backgroundColor = Silica::GetTheme().Background_Popup,
			.child = Silica::MakeWidget<Silica::SVerticalBox>({
				.spacing = SPACING_SMALL,
				.slots = {
					{ {0,0}, MakeCtxBtn("Delete Entity", [this, entity]() mutable {
						auto cmd = MakeShared<DeleteEntityCommand>(m_scene, entity);
						cmd->execute();
						EditorCommandManager::push(cmd);
						EntitySelectedEvent ev({});
						m_eventCallback(ev);
					}, Silica::GetTheme().Accent_Danger) },

					{ {0,0}, MakeCtxBtn("Add Child", [this, entity]() mutable {
						Entity child = m_scene->createEntity("Child Entity");
						child.setParent(entity);
						rebuildUI();
					}) },

					{ {0,0}, MakeCtxBtn("Create Prefab", [this, entity, tag]() mutable {
						std::filesystem::path prefabDir = ProjectManager::getProject()->getAssetsPath() / "prefabs";
						std::filesystem::create_directories(prefabDir);
						std::filesystem::path savePath = FileDialogs::saveFile({ {"Axion Prefab Asset", "*.axprefab"} }, prefabDir);

						if (!savePath.empty()) {
							UUID newAssetUUID = UUID::generate();
							AAP::PrefabAssetData data{ newAssetUUID, tag, m_scene, entity };
							AAP::PrefabParser::createTextFile(data, savePath);

							AssetMetadata metadata{ newAssetUUID, AssetType::Prefab, AssetManager::getRelativeToAssets(savePath) };
							auto registry = ProjectManager::getProject()->getAssetRegistry();
							registry->add(metadata);
							registry->serialize(ProjectManager::getProject()->getProjectPath() / "AssetRegistry.yaml");
						}
					}) }
				}
			})
		});

		// -- Create TreeNode And Drag / Drop Logic --
		auto treeNode = Silica::MakeWidget<Silica::STreeNode>({
			.label = tag,
			.yTextOffset = TREE_NODE_Y_OFFSET,
			.initiallyOpen = m_openNodes.find((entt::entity)entity) != m_openNodes.end(),
			.isSelected = m_selectedEntity == entity,
			.isLeaf = !hasChildren,
			.isDragged = [entity]() { return Silica::DragDrop::isDraggingType("Entity") && std::any_cast<Entity>(Silica::DragDrop::getPayload().data) == entity; },
			.onClicked = [this, entity]() {
				m_selectedEntity = entity;
				EntitySelectedEvent ev(entity);
				m_eventCallback(ev);
				rebuildUI();
			},
			.onDragStart = [entity, tag]() { Silica::DragDrop::beginDrag("Entity", entity, tag, Silica::GetTheme().Font_Default); },
			.onDragOver = [this, entity](const Silica::DragDropPayload& payload) {
				if (payload.type == "Entity") {
					Entity draggedEntity = std::any_cast<Entity>(payload.data);
					entt::entity currentParent = draggedEntity.hasComponent<RelationshipComponent>() ? draggedEntity.getComponent<RelationshipComponent>().parent : entt::null;

					if (draggedEntity == entity || currentParent == (entt::entity)entity) return Silica::EventReply::unhandled();

					Entity current = entity;
					while (current) {
						if (current == draggedEntity) return Silica::EventReply::unhandled();
						current = current.getParent();
					}
					return Silica::EventReply::handled();
				}
				return Silica::EventReply::unhandled();
			},
			.onDrop = [this, entity](const Silica::DragDropPayload& payload) mutable {
				if (payload.type == "Entity") {
					Entity draggedEntity = std::any_cast<Entity>(payload.data);
					EditorActionQueue::push([this, entity, draggedEntity]() mutable {
						attachEntityToParent(draggedEntity, entity);
						rebuildUI();
					});
					return Silica::EventReply::handled();
				}
				return Silica::EventReply::unhandled();
			},
			.onToggleOpen = [this, entity](bool isOpen) {
				if (isOpen) m_openNodes.insert((entt::entity)entity);
				else m_openNodes.erase((entt::entity)entity);
			}
		});

		// -- Attach Children --
		if (entity.hasComponent<RelationshipComponent>()) {
			for (entt::entity childHandle : entity.getComponent<RelationshipComponent>().children) {
				Entity childEntity{ childHandle, m_scene.get() };
				treeNode->addChild(buildEntityNode(childEntity));
			}
		}

		return Silica::MakeWidget<Silica::SMenuAnchor>({
			.openOnHover = false,
			.openOnRightClick = true,
			.openAtMousePos = true,
			.anchorContent = treeNode,
			.menuContent = contextMenu
		});
	}

	void HierarchyPanel::rebuildUI() {
		if (!m_uiRoot || !m_contentBox) return;

		// -- No project loaded --
		if (!ProjectManager::hasProject()) {
			m_uiRoot->setChild(SilicaHelpers::MakeEmptyState("No Project Loaded.\n\nPlease load or create a project from the top menu bar to view entities."));
			return;
		}

		m_uiRoot->setChild(m_contentBox);
		m_contentBox->clearSlots();

		if (!m_scene) return;

		auto addEntityButton = Silica::MakeWidget<Silica::SButton>({
			.padding = { BTN_PAD_X, BTN_PAD_Y }, .hoverColor = Silica::GetTheme().Accent_Primary,
			.onClick = [this]() mutable {
				EditorActionQueue::push([this]() { m_scene->createEntity("Empty Entity"); rebuildUI(); });
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "+ Add Entity" })
		});

		auto topBarBox = Silica::MakeWidget<Silica::SBox>({
			.padding = { PADDING_SMALL, PADDING_SMALL }, .backgroundColor = Silica::GetTheme().Surface_Tertiary,
			.onDragOver = [](const Silica::DragDropPayload& payload) {
				if (payload.type == "Entity") return Silica::EventReply::handled();
				return Silica::EventReply::unhandled();
			},
			.onDrop = [this](const Silica::DragDropPayload& payload) mutable {
				if (payload.type == "Entity") {
					Entity draggedEntity = std::any_cast<Entity>(payload.data);
					EditorActionQueue::push([this, draggedEntity]() mutable {
						detachEntityFromParent(draggedEntity);
						rebuildUI();
					});
					return Silica::EventReply::handled();
				}
				return Silica::EventReply::unhandled();
			},
			.child = Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = SPACING_MEDIUM,
				.slots = { { {0,0}, addEntityButton } } 
			})
		});

		// -- Tree Content --
		auto treeContainer = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 0.0f });
		auto view = m_scene->getRegistry().view<TagComponent>(entt::exclude<PendingDestroyComponent>);

		for (auto e : view) {
			Entity entity{ e, m_scene.get() };

			bool isRoot = !entity.hasComponent<RelationshipComponent>() || entity.getComponent<RelationshipComponent>().parent == entt::null;
			if (isRoot) {
				treeContainer->addSlot({ .child = buildEntityNode(entity) });
			}
		}

		m_contentBox->addSlot({
			.child = Silica::MakeWidget<Silica::SBorderLayout>({
				.topBar = topBarBox,
				.contentArea = Silica::MakeWidget<Silica::SScrollBox>({.child = treeContainer })
			})
		});
	}

}
