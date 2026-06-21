#include "HierarchyPanel.h"

#include <algorithm>
#include <filesystem>

#include "AxionStudio/Vendor/Silica/include/SScrollBox.h"
#include "AxionStudio/Vendor/Silica/include/SVerticalBox.h"
#include "AxionStudio/Vendor/Silica/include/SHorizontalBox.h"
#include "AxionStudio/Vendor/Silica/include/SButton.h"
#include "AxionStudio/Vendor/Silica/include/STextBlock.h"
#include "AxionStudio/Vendor/Silica/include/STreeNode.h"
#include "AxionStudio/Vendor/Silica/include/SMenuAnchor.h"
#include "AxionStudio/Vendor/Silica/include/SBox.h"
#include "AxionStudio/Vendor/Silica/include/SBorderLayout.h"

#include "AxionEngine/Source/scene/Components.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/Application.h"

#include "AxionStudio/Source/core/EditorActionQueue.h"

#include "AxionAssetPipeline/Source/parser/PrefabParser.h"

namespace Axion {

	static Entity s_draggedEntity = {};



	// ----- CUSTOM WIDGETS -----
	class SHierarchyDropZone : public Silica::SWidget {
	public:

		struct Args {
			Silica::FontAtlas* font;
			std::function<Silica::EventReply()> onDrop;
			Silica::WidgetPtr child;
		};


		void construct(const Args& args) {
			m_font = args.font;
			m_onDrop = args.onDrop;
			m_child = args.child;
		}

		void computeDesiredSize() override {
			m_desiredSize = Silica::Vec2::zero();
			if (m_child) {
				m_child->computeDesiredSize();
				m_desiredSize = m_child->getDesiredSize();
			}
		}

		void arrangeChildren(const Silica::Geometry& allocatedGeometry) override {
			SWidget::arrangeChildren(allocatedGeometry);
			if (m_child) m_child->arrangeChildren(allocatedGeometry);
		}

		void onDraw(Silica::DrawList& outDrawList, const Silica::Geometry& allocatedGeometry) const override {
			// -- Draw Scroll Box And Tree Nodes --
			if (m_child) m_child->onDraw(outDrawList, allocatedGeometry);

			// -- Draw Floating Drag Payload Tooltip --
			if (s_draggedEntity && m_font) {
				std::string tag = "Unnamed Entity";
				if (s_draggedEntity.hasComponent<TagComponent>()) {
					tag = s_draggedEntity.getComponent<TagComponent>().tag;
				}

				Silica::Vec2 mousePos = Silica::Renderer::s_mousePosition;
				mousePos.x += 15.0f;
				mousePos.y += 15.0f;

				float textWidth = 0.0f;
				for (char c : tag) textWidth += m_font->getGlyph(c).advanceX;

				Silica::Geometry bgGeo = { mousePos, {textWidth + 16.0f, 26.0f} };
				addRectToDrawList(outDrawList, bgGeo, Silica::Color(40, 40, 40, 230));
				drawText(outDrawList, m_font, tag, { mousePos.x + 8.0f, mousePos.y }, Silica::Color::white(), 18.0f);
			}
		}

		Silica::EventReply onMouseMove(const Silica::Geometry& allocatedGeometry, const Silica::Vec2& mousePos) override {
			if (s_draggedEntity) {
				Application::get().getCursor().setCursor(CursorType::Hand);
			}

			if (m_child) return m_child->onMouseMove(m_child->getAllocatedGeometry(), mousePos);
			return Silica::EventReply::unhandled();
		}

		Silica::EventReply onMouseButtonDown(const Silica::Geometry& allocatedGeometry, const Silica::Vec2& mousePos, Silica::MouseButton button) override {
			if (m_child) return m_child->onMouseButtonDown(m_child->getAllocatedGeometry(), mousePos, button);
			return Silica::EventReply::unhandled();
		}

		Silica::EventReply onMouseButtonUp(const Silica::Geometry& allocatedGeometry, const Silica::Vec2& mousePos, Silica::MouseButton button) override {
			Silica::EventReply reply = Silica::EventReply::unhandled();

			if (m_child) {
				reply = m_child->onMouseButtonUp(m_child->getAllocatedGeometry(), mousePos, button);
			}

			if (s_draggedEntity) {
				Application::get().getCursor().setCursor(CursorType::Arrow);

				if (!reply.isHandled && m_onDrop) {
					return m_onDrop();
				}
			}

			return reply;
		}

		Silica::EventReply onMouseWheel(const Silica::Geometry& allocatedGeometry, const Silica::Vec2& mousePos, float scrollDelta) override {
			if (m_child) return m_child->onMouseWheel(m_child->getAllocatedGeometry(), mousePos, scrollDelta);
			return Silica::EventReply::unhandled();
		}

	private:

		Silica::WidgetPtr m_child;
		Silica::FontAtlas* m_font;
		std::function<Silica::EventReply()> m_onDrop;

		void drawText(Silica::DrawList& drawList, Silica::FontAtlas* font, const std::string& text, Silica::Vec2 pos, Silica::Color color, float yOffset) const {
			float cursorX = pos.x;
			float baselineY = pos.y + yOffset;

			for (char c : text) {
				const Silica::Glyph& g = font->getGlyph(c);
				if (g.size.x > 0 && g.size.y > 0) {
					float x0 = cursorX + g.offset.x; float y0 = baselineY + g.offset.y;
					float x1 = x0 + g.size.x;        float y1 = y0 + g.size.y;
					uint32_t startIndex = (uint32_t)drawList.vertices.size();
					drawList.vertices.push_back({ {x0, y0}, {g.uvMin.x, g.uvMin.y}, color });
					drawList.vertices.push_back({ {x1, y0}, {g.uvMax.x, g.uvMin.y}, color });
					drawList.vertices.push_back({ {x1, y1}, {g.uvMax.x, g.uvMax.y}, color });
					drawList.vertices.push_back({ {x0, y1}, {g.uvMin.x, g.uvMax.y}, color });
					drawList.indices.push_back(startIndex + 0); drawList.indices.push_back(startIndex + 1); drawList.indices.push_back(startIndex + 2);
					drawList.indices.push_back(startIndex + 0); drawList.indices.push_back(startIndex + 2); drawList.indices.push_back(startIndex + 3);
					if (drawList.commands.empty()) drawList.commands.push_back({ 0, 0, 0 });
					drawList.commands.back().indexCount += 6;
				}
				cursorX += g.advanceX;
			}
		}

		void addRectToDrawList(Silica::DrawList& drawList, const Silica::Geometry& geo, Silica::Color color) const {
			uint32_t startIndex = (uint32_t)drawList.vertices.size();
			drawList.vertices.push_back({ {geo.position.x, geo.position.y}, {0.0f, 0.0f}, color });
			drawList.vertices.push_back({ {geo.position.x + geo.size.x, geo.position.y}, {0.0f, 0.0f}, color });
			drawList.vertices.push_back({ {geo.position.x + geo.size.x, geo.position.y + geo.size.y}, {0.0f, 0.0f}, color });
			drawList.vertices.push_back({ {geo.position.x, geo.position.y + geo.size.y}, {0.0f, 0.0f}, color });
			drawList.indices.push_back(startIndex + 0); drawList.indices.push_back(startIndex + 1); drawList.indices.push_back(startIndex + 2);
			drawList.indices.push_back(startIndex + 0); drawList.indices.push_back(startIndex + 2); drawList.indices.push_back(startIndex + 3);
			if (drawList.commands.empty()) drawList.commands.push_back({ 0, 0, 0 });
			drawList.commands.back().indexCount += 6;
		}
	};





	// ----- HIERARCHY PANEL IMPLEMENTATION -----
	Silica::WidgetPtr HierarchyPanel::getWidget(Silica::FontAtlas* font) {
		m_font = font;
		if (!m_uiRoot) {
			m_contentBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 0.0f });
			m_uiRoot = Silica::MakeWidget<Silica::SBox>({ .child = m_contentBox });
			rebuildUI();
		}
		return m_uiRoot;
	}

	void HierarchyPanel::setScene(Ref<Scene> scene) {
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

		// -- Right Click Context Menu --
		auto contextMenu = Silica::MakeWidget<Silica::SBox>({
			.padding = { 5.0f, 5.0f },
			.backgroundColor = Silica::Color(45, 45, 45, 255),
			.child = Silica::MakeWidget<Silica::SVerticalBox>({
				.spacing = 2.0f,
				.slots = {
					// -- Delete Entity --
					{ {0,0}, Silica::MakeWidget<Silica::SButton>({
						.padding = { 8.0f, 4.0f },
						.color = Silica::Color::transparent(),
						.hoverColor = Silica::Color(200, 50, 50, 255),
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

								if (s_draggedEntity == entity) s_draggedEntity = {};
								if (m_onEntitySelected) m_onEntitySelected({});

								rebuildUI();
							});
							return Silica::EventReply::handled();
						},
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Delete Entity", .font = m_font })
					})},
					// -- Add Child --
					{ {0,0}, Silica::MakeWidget<Silica::SButton>({
						.padding = { 8.0f, 4.0f },
						.color = Silica::Color::transparent(),
						.hoverColor = Silica::Color(70, 130, 200, 255),
						.onClick = [this, entity]() mutable {
							EditorActionQueue::push([this, entity]() mutable {
								Entity child = m_scene->createEntity("Child Entity");
								child.setParent(entity);
								rebuildUI();
							});
							return Silica::EventReply::handled();
						},
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Add Child", .font = m_font })
					})},
					// -- Create Prefab --
					{ {0,0}, Silica::MakeWidget<Silica::SButton>({
						.padding = { 8.0f, 4.0f },
						.color = Silica::Color::transparent(),
						.hoverColor = Silica::Color(70, 130, 200, 255),
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
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Create Prefab", .font = m_font })
					})}
				}
			})
		});

		// -- Create TreeNode And Drag / Drop Logic --
		auto treeNode = Silica::MakeWidget<Silica::STreeNode>({
			.label = tag,
			.font = m_font,
			.yTextOffset = 16.0f,
			.isDragged = [entity]() { return s_draggedEntity == entity; },
			.onClicked = [this, entity]() {
				s_draggedEntity = {};
				if (m_onEntitySelected) m_onEntitySelected(entity);
				return Silica::EventReply::handled();
			},

			// -- Pick Up Payload --
			.onDragStart = [entity]() {
				s_draggedEntity = entity;
			},

			// -- Drop Payload On This Node --
			.onDrop = [this, entity]() mutable {
				if (s_draggedEntity) {
					EditorActionQueue::push([this, entity]() mutable {
						entt::entity currentParent = entt::null;
						if (s_draggedEntity.hasComponent<RelationshipComponent>()) {
							currentParent = s_draggedEntity.getComponent<RelationshipComponent>().parent;
						}

						if (s_draggedEntity != entity && currentParent != (entt::entity)entity) {
							bool isDescendant = false;
							Entity current = entity;
							while (current) {
								if (current == s_draggedEntity) {
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
									auto it = std::find(parentRel.children.begin(), parentRel.children.end(), (entt::entity)s_draggedEntity);
									if (it != parentRel.children.end()) parentRel.children.erase(it);
								}

								// -- Attach To New Parent --
								s_draggedEntity.setParent(entity);
							}
						}

						s_draggedEntity = {};
						Application::get().getCursor().setCursor(CursorType::Arrow);
						rebuildUI();
					});
					return Silica::EventReply::handled();
				}
				return Silica::EventReply::unhandled();
			}
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
			.color = Silica::Color(50, 50, 50, 255),
			.hoverColor = Silica::Color(70, 130, 200, 255),
			.onClick = [this]() mutable {
				EditorActionQueue::push([this]() {
					s_draggedEntity = {};
					m_scene->createEntity("Empty Entity");
					rebuildUI();
				});

				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "+ Add Entity", .font = m_font })
		});

		auto topBarBox = Silica::MakeWidget<Silica::SBox>({
			.padding = { 5.0f, 5.0f },
			.backgroundColor = Silica::Color(30, 30, 30, 255),
			.onDrop = [this]() mutable {
				if (s_draggedEntity) {
					EditorActionQueue::push([this]() mutable {
						if (s_draggedEntity.hasComponent<RelationshipComponent>()) {
							auto& rel = s_draggedEntity.getComponent<RelationshipComponent>();
							if (rel.parent != entt::null) {
								Entity parent = { rel.parent, m_scene.get() };
								auto& parentRel = parent.getComponent<RelationshipComponent>();
								auto it = std::find(parentRel.children.begin(), parentRel.children.end(), (entt::entity)s_draggedEntity);
								if (it != parentRel.children.end()) parentRel.children.erase(it);

								rel.parent = entt::null;
							}
						}
						s_draggedEntity = {};
						Application::get().getCursor().setCursor(CursorType::Arrow);
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

		auto backgroundDropWrapper = Silica::MakeWidget<SHierarchyDropZone>({
			.font = m_font,
			.onDrop = [this]() mutable {
				if (s_draggedEntity) {
					EditorActionQueue::push([this]() mutable {
						if (s_draggedEntity.hasComponent<RelationshipComponent>()) {
							auto& rel = s_draggedEntity.getComponent<RelationshipComponent>();
							if (rel.parent != entt::null) {
								Entity parent = { rel.parent, m_scene.get() };
								auto& parentRel = parent.getComponent<RelationshipComponent>();
								auto it = std::find(parentRel.children.begin(), parentRel.children.end(), (entt::entity)s_draggedEntity);
								if (it != parentRel.children.end()) parentRel.children.erase(it);

								rel.parent = entt::null;
							}
						}
						s_draggedEntity = {};
						Application::get().getCursor().setCursor(CursorType::Arrow);
						rebuildUI();
						});
					return Silica::EventReply::handled();
				}
				return Silica::EventReply::unhandled();
			},
			.child = scrollBox
		});


		// -- Assemble Border Layout --
		auto borderLayout = Silica::MakeWidget<Silica::SBorderLayout>({
			.topBar = topBarBox,
			.contentArea = backgroundDropWrapper
		});

		m_contentBox->addSlot({
			.padding = {0.0f, 0.0f},
			.child = borderLayout
		});
	}

}
