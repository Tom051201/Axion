#include "studiopch.h"
#include "HierarchyPanel.h"

#include <Silica/include/Theme.h>
#include <Silica/include/SScrollBox.h>
#include <Silica/include/SVerticalBox.h>
#include <Silica/include/SHorizontalBox.h>
#include <Silica/include/SButton.h>
#include <Silica/include/STextBlock.h>
#include <Silica/include/STreeNode.h>
#include <Silica/include/SMenuAnchor.h>
#include <Silica/include/SBox.h>
#include <Silica/include/SBorderLayout.h>

#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/Application.h"
#include "AxionEngine/Source/scene/Components.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionAssetPipeline/Source/parser/PrefabParser.h"

#include "AxionStudio/Source/core/EditorActionQueue.h"

namespace Axion {

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
							if (draggedEntity.hasComponent<RelationshipComponent>()) {
								auto& rel = draggedEntity.getComponent<RelationshipComponent>();
								if (rel.parent != entt::null) {
									Entity parent = { rel.parent, m_scene.get() };
									auto& parentRel = parent.getComponent<RelationshipComponent>();
									auto it = std::find(parentRel.children.begin(), parentRel.children.end(), (entt::entity)draggedEntity);
									if (it != parentRel.children.end()) parentRel.children.erase(it);

									rel.parent = entt::null;
								}
							}
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

	void HierarchyPanel::setSelectionCallback(std::function<void(Entity)> callback) {
		m_onEntitySelected = callback;
	}

	Silica::WidgetPtr HierarchyPanel::buildEntityNode(Entity entity) {
		std::string tag = "Unnamed Entity";
		if (entity.hasComponent<TagComponent>()) {
			tag = entity.getComponent<TagComponent>().tag;
		}

		bool hasChildren = false;
		if (entity.hasComponent<RelationshipComponent>()) {
			hasChildren = !entity.getComponent<RelationshipComponent>().children.empty();
		}

		// -- Right Click Context Menu --
		auto contextMenu = Silica::MakeWidget<Silica::SBox>({
			.padding = { 5.0f, 5.0f },
			.borderThickness = Silica::GetTheme().Border_Thickness,
			.backgroundColor = Silica::GetTheme().Background_Popup,
			.child = Silica::MakeWidget<Silica::SVerticalBox>({
				.spacing = 2.0f,
				.slots = {
					// -- Delete Entity --
					{ {0,0}, Silica::MakeWidget<Silica::SButton>({
						.padding = { 8.0f, 4.0f },
						.color = Silica::Color::transparent(),
						.hoverColor = Silica::GetTheme().Accent_Danger,
						.onClick = [this, entity]() mutable {
							EditorActionQueue::push([this, entity]() mutable {
								// -- Remove From Parent --
								if (entity.hasComponent<RelationshipComponent>()) {
									auto& rel = entity.getComponent<RelationshipComponent>();
									if (rel.parent != entt::null) {
										Entity parent = { rel.parent, m_scene.get() };
										auto& parentRel = parent.getComponent<RelationshipComponent>();
										auto it = std::find(parentRel.children.begin(), parentRel.children.end(), (entt::entity)entity);
										if (it != parentRel.children.end()) parentRel.children.erase(it);
									}
								}

								// -- Destroy Entity And All Descendants --
								auto destroyHierarchy = [this](Entity e, auto& self) -> void {
									if (e.hasComponent<RelationshipComponent>()) {
										auto childrenCopy = e.getComponent<RelationshipComponent>().children;
										for (auto childHandle : childrenCopy) {
											self(Entity{ childHandle, m_scene.get() }, self);
										}
									}
									m_scene->destroyEntity(e);
								};

								destroyHierarchy(entity, destroyHierarchy);

								if (m_onEntitySelected) m_onEntitySelected({});

								rebuildUI();
							});
							return Silica::EventReply::handled();
						},
						.child = Silica::MakeWidget<Silica::STextBlock>({ .text = "Delete Entity" })
					})},
					// -- Add Child --
					{ {0,0}, Silica::MakeWidget<Silica::SButton>({
						.padding = { 8.0f, 4.0f },
						.color = Silica::Color::transparent(),
						.hoverColor = Silica::GetTheme().Accent_Primary,
						.onClick = [this, entity]() mutable {
							EditorActionQueue::push([this, entity]() mutable {
								Entity child = m_scene->createEntity("Child Entity");
								child.setParent(entity);
								rebuildUI();
							});
							return Silica::EventReply::handled();
						},
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Add Child" })
					})},
					// -- Create Prefab --
					{ {0,0}, Silica::MakeWidget<Silica::SButton>({
						.padding = { 8.0f, 4.0f },
						.color = Silica::Color::transparent(),
						.hoverColor = Silica::GetTheme().Accent_Primary,
						.onClick = [this, entity]() mutable {
							std::filesystem::path prefabDir = ProjectManager::getProject()->getAssetsPath() / "prefabs";
							std::filesystem::create_directories(prefabDir);
							std::filesystem::path savePath = FileDialogs::saveFile({ {"Axion Prefab Asset", "*.axprefab"} }, prefabDir);
							if (!savePath.empty()) {
								UUID newAssetUUID = UUID::generate();
								AAP::PrefabAssetData data;
								data.uuid = newAssetUUID;
								data.name = entity.getComponent<TagComponent>().tag;
								data.scene = m_scene;
								data.entity = entity;
								AAP::PrefabParser::createTextFile(data, savePath);

								AssetMetadata metadata;
								metadata.handle = newAssetUUID;
								metadata.type = AssetType::Prefab;
								metadata.filePath = AssetManager::getRelativeToAssets(savePath);
								auto registry = ProjectManager::getProject()->getAssetRegistry();
								registry->add(metadata);
								registry->serialize(ProjectManager::getProject()->getProjectPath() / "AssetRegistry.yaml");
							}
							return Silica::EventReply::handled();
						},
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Create Prefab" })
					})}
				}
			})
		});

		// -- Create TreeNode And Drag / Drop Logic --
		auto treeNode = Silica::MakeWidget<Silica::STreeNode>({
			.label = tag,
			.yTextOffset = 16.0f,
			.initiallyOpen = m_openNodes.find((entt::entity)entity) != m_openNodes.end(),
			.isSelected = m_selectedEntity == entity,
			.isLeaf = !hasChildren,
			.isDragged = [entity]() {
				return Silica::DragDrop::isDraggingType("Entity") && std::any_cast<Entity>(Silica::DragDrop::getPayload().data) == entity;
			},

			.onClicked = [this, entity]() {
				m_selectedEntity = entity;
				if (m_onEntitySelected) m_onEntitySelected(entity);
				rebuildUI();
			},

			// -- Initiate Native Drag --
			.onDragStart = [entity, tag]() {
				Silica::DragDrop::beginDrag("Entity", entity, tag, Silica::GetTheme().Font_Default);
			},

			// -- Smart Drag Over Validation --
			.onDragOver = [this, entity](const Silica::DragDropPayload& payload) {
				if (payload.type == "Entity") {
					Entity draggedEntity = std::any_cast<Entity>(payload.data);

					entt::entity currentParent = entt::null;
					if (draggedEntity.hasComponent<RelationshipComponent>()) {
						currentParent = draggedEntity.getComponent<RelationshipComponent>().parent;
					}

					if (draggedEntity == entity || currentParent == (entt::entity)entity) {
						return Silica::EventReply::unhandled();
					}

					Entity current = entity;
					while (current) {
						if (current == draggedEntity) return Silica::EventReply::unhandled();
						current = current.getParent();
					}

					return Silica::EventReply::handled();
				}
				return Silica::EventReply::unhandled();
			},

			// -- Catch Native Drop --
			.onDrop = [this, entity](const Silica::DragDropPayload& payload) mutable {
				if (payload.type == "Entity") {
					Entity draggedEntity = std::any_cast<Entity>(payload.data);

					EditorActionQueue::push([this, entity, draggedEntity]() mutable {
						entt::entity currentParent = entt::null;
						if (draggedEntity.hasComponent<RelationshipComponent>()) {
							currentParent = draggedEntity.getComponent<RelationshipComponent>().parent;
						}

						if (draggedEntity != entity && currentParent != (entt::entity)entity) {
							bool isDescendant = false;
							Entity current = entity;
							while (current) {
								if (current == draggedEntity) {
									isDescendant = true;
									break;
								}
								current = current.getParent();
							}

							if (!isDescendant) {
								// -- Remove From Old Parent --
								if (currentParent != entt::null) {
									Entity parent = { currentParent, m_scene.get() };
									auto& parentRel = parent.getComponent<RelationshipComponent>();
									auto it = std::find(parentRel.children.begin(), parentRel.children.end(), (entt::entity)draggedEntity);
									if (it != parentRel.children.end()) parentRel.children.erase(it);
								}

								// -- Attach To New Parent --
								draggedEntity.setParent(entity);
							}
						}
						rebuildUI();
					});
					return Silica::EventReply::handled();
				}
				return Silica::EventReply::unhandled();
			},
			.onToggleOpen = [this, entity](bool isOpen) {
				if (isOpen) m_openNodes.insert((entt::entity)entity);
				else m_openNodes.erase((entt::entity)entity);
			},
		});

		auto nodeWithMenu = Silica::MakeWidget<Silica::SMenuAnchor>({
			.openOnHover = false,
			.openOnRightClick = true,
			.openAtMousePos = true,
			.anchorContent = treeNode,
			.menuContent = contextMenu
		});

		// -- Attach Children --
		if (entity.hasComponent<RelationshipComponent>()) {
			auto& rel = entity.getComponent<RelationshipComponent>();

			for (entt::entity childHandle : rel.children) {
				Entity childEntity{ childHandle, m_scene.get() };
				Silica::WidgetPtr childWidget = buildEntityNode(childEntity);
				treeNode->addChild(childWidget);
			}
		}

		return nodeWithMenu;
	}

	void HierarchyPanel::rebuildUI() {
		if (!m_contentBox) return;
		m_contentBox->clearSlots();

		if (!m_scene) return;

		auto addEntityButton = Silica::MakeWidget<Silica::SButton>({
			.padding = { 8.0f, 4.0f },
			.hoverColor = Silica::GetTheme().Accent_Primary,
			.onClick = [this]() mutable {
				EditorActionQueue::push([this]() {
					m_scene->createEntity("Empty Entity");
					rebuildUI();
				});

				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "+ Add Entity" })
		});

		auto topBarBox = Silica::MakeWidget<Silica::SBox>({
			.padding = { 5.0f, 5.0f },
			.backgroundColor = Silica::GetTheme().Surface_Tertiary,
			.onDragOver = [](const Silica::DragDropPayload& payload) {
				if (payload.type == "Entity") return Silica::EventReply::handled();
				return Silica::EventReply::unhandled();
			},
			.onDrop = [this](const Silica::DragDropPayload& payload) mutable {
				if (payload.type == "Entity") {
					Entity draggedEntity = std::any_cast<Entity>(payload.data);

					EditorActionQueue::push([this, draggedEntity]() mutable {
						if (draggedEntity.hasComponent<RelationshipComponent>()) {
							auto& rel = draggedEntity.getComponent<RelationshipComponent>();
							if (rel.parent != entt::null) {
								Entity parent = { rel.parent, m_scene.get() };
								auto& parentRel = parent.getComponent<RelationshipComponent>();
								auto it = std::find(parentRel.children.begin(), parentRel.children.end(), (entt::entity)draggedEntity);
								if (it != parentRel.children.end()) parentRel.children.erase(it);

								rel.parent = entt::null;
							}
						}
						rebuildUI();
					});
					return Silica::EventReply::handled();
				}
				return Silica::EventReply::unhandled();
			},
			.child = Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = 5.0f,
				.slots = {
					{ {0,0}, addEntityButton }
				}
			})
		});

		// -- Tree Content --
		auto treeContainer = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 0.0f });

		auto view = m_scene->getRegistry().view<TagComponent>(entt::exclude<PendingDestroyComponent>);
		for (auto e : view) {
			Entity entity{ e, m_scene.get() };

			bool isRoot = true;
			if (entity.hasComponent<RelationshipComponent>()) {
				if (entity.getComponent<RelationshipComponent>().parent != entt::null) {
					isRoot = false;
				}
			}

			if (isRoot) {
				treeContainer->addSlot({
					.padding = {0.0f, 0.0f},
					.child = buildEntityNode(entity)
				});
			}
		}

		auto scrollBox = Silica::MakeWidget<Silica::SScrollBox>({ .child = treeContainer });


		// -- Assemble Border Layout --
		auto borderLayout = Silica::MakeWidget<Silica::SBorderLayout>({
			.topBar = topBarBox,
			.contentArea = scrollBox
		});

		m_contentBox->addSlot({
			.padding = {0.0f, 0.0f},
			.child = borderLayout
		});
	}

}
