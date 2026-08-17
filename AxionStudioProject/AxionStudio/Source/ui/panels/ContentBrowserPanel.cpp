#include "studiopch.h"
#include "ContentBrowserPanel.h"

#include <Silica/include/Theme.h>
#include <Silica/include/SBox.h>
#include <Silica/include/SBorderLayout.h>
#include <Silica/include/SHorizontalBox.h>
#include <Silica/include/SVerticalBox.h>
#include <Silica/include/SButton.h>
#include <Silica/include/STextBlock.h>
#include <Silica/include/SImage.h>
#include <Silica/include/SMenuAnchor.h>
#include <Silica/include/SScrollBox.h>
#include <Silica/include/SEditableText.h>
#include <Silica/include/SCheckbox.h>
#include <Silica/include/SOverlay.h>
#include <Silica/include/SAlign.h>
#include <Silica/include/SWrapBox.h>
#include <Silica/include/SScrollCatcher.h>
#include <Silica/include/STreeNode.h>
#include <Silica/include/SHorizontalSplitBox.h>
#include <Silica/include/SVerticalSplitBox.h>

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/Application.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionEngine/Source/input/Input.h"

#include "AxionStudio/Source/core/EditorResourceManager.h"
#include "AxionStudio/Source/core/EditorActionQueue.h"
#include "AxionStudio/Source/core/EditorUtils.h"
#include "AxionStudio/Source/core/SilicaContext.h"
#include "AxionStudio/Source/scripting/VisualScriptGraph.h"
#include "AxionStudio/Source/scripting/VisualScriptSerializer.h"
#include "AxionStudio/Source/ui/modals/AudioImportModal.h"
#include "AxionStudio/Source/ui/modals/MaterialImportModal.h"
#include "AxionStudio/Source/ui/modals/MeshImportModal.h"
#include "AxionStudio/Source/ui/modals/PhysicsMaterialImportModal.h"
#include "AxionStudio/Source/ui/modals/PipelineImportModal.h"
#include "AxionStudio/Source/ui/modals/ShaderImportModal.h"
#include "AxionStudio/Source/ui/modals/SkyboxImportModal.h"
#include "AxionStudio/Source/ui/modals/Texture2DImportModal.h"
#include "AxionStudio/Source/ui/modals/TextureCubeImportModal.h"

namespace Axion {

	// ----- HELPER UI WIDGETS -----
	class SAssetClickBox : public Silica::SWidget {
	public:

		struct Args {
			std::optional<Silica::Color> hoverColor;
			std::function<void()> onDragStart;
			std::function<Silica::EventReply(const Silica::DragDropPayload&)> onDragOver;
			std::function<Silica::EventReply(const Silica::DragDropPayload&)> onDrop;
			std::function<Silica::EventReply()> onClick;
			Silica::WidgetPtr child;
		};

		void construct(const Args& args) {
			m_hoverColor = args.hoverColor.value_or(Silica::Color(255, 255, 255, 20));
			m_child = args.child;
			m_onDragStart = args.onDragStart;
			m_onDragOver = args.onDragOver;
			m_onDrop = args.onDrop;
			m_onClick = args.onClick;
		}

		void computeDesiredSize() override {
			if (m_child) {
				m_child->computeDesiredSize();
				m_desiredSize = m_child->getDesiredSize();
			}
		}

		void arrangeChildren(const Silica::Geometry& geom) override {
			SWidget::arrangeChildren(geom);
			if (m_child) m_child->arrangeChildren(geom);
		}

		void onDraw(Silica::DrawList& dl, const Silica::Geometry& geom) const override {
			Silica::Color color = Silica::Color::transparent();
			if (m_isMouseDown) { color = Silica::GetTheme().Accent_Primary; color.setAlpha(150); }
			else if (m_isHovered) color = m_hoverColor;

			// -- Drop Zone Highlighting --
			if (Silica::SWidget::getDragHoveredWidget() == this) {
				color = Silica::GetTheme().Accent_Primary;
				color.setAlpha(80);
			}

			if (color.a() > 0) {
				uint32_t startIndex = (uint32_t)dl.vertices.size();
				dl.vertices.push_back({ {geom.position.x, geom.position.y}, {0, 0}, color });
				dl.vertices.push_back({ {geom.position.x + geom.size.x, geom.position.y}, {0, 0}, color });
				dl.vertices.push_back({ {geom.position.x + geom.size.x, geom.position.y + geom.size.y}, {0, 0}, color });
				dl.vertices.push_back({ {geom.position.x, geom.position.y + geom.size.y}, {0, 0}, color });
				dl.indices.push_back(startIndex + 0);
				dl.indices.push_back(startIndex + 1);
				dl.indices.push_back(startIndex + 2);
				dl.indices.push_back(startIndex + 0);
				dl.indices.push_back(startIndex + 2);
				dl.indices.push_back(startIndex + 3);
				if (dl.commands.empty()) dl.commands.push_back({ 0, 0, 0 });
				dl.commands.back().indexCount += 6;
			}
			if (m_child) m_child->onDraw(dl, m_child->getAllocatedGeometry());
		}

		Silica::EventReply onDragOver(const Silica::Geometry& geom, const Silica::Vec2& pos, const Silica::DragDropPayload& payload) override {
			if (m_child) {
				Silica::EventReply reply = m_child->onDragOver(m_child->getAllocatedGeometry(), pos, payload);
				if (reply.isHandled) return reply;
			}
			if (geom.contains(pos) && m_onDragOver) {
				auto reply = m_onDragOver(payload);
				if (reply.isHandled) {
					Silica::SWidget::setDragHoveredWidget(this);
					return reply;
				}
			}
			return Silica::EventReply::unhandled();
		}

		Silica::EventReply onDrop(const Silica::Geometry& geom, const Silica::Vec2& pos, const Silica::DragDropPayload& payload) override {
			if (m_child) {
				Silica::EventReply reply = m_child->onDrop(m_child->getAllocatedGeometry(), pos, payload);
				if (reply.isHandled) return reply;
			}
			if (geom.contains(pos) && m_onDrop) return m_onDrop(payload);
			return Silica::EventReply::unhandled();
		}

		Silica::EventReply onMouseWheel(const Silica::Geometry& geom, const Silica::Vec2& pos, float delta) override {
			if (m_child) return m_child->onMouseWheel(m_child->getAllocatedGeometry(), pos, delta);
			return Silica::EventReply::unhandled();
		}

		Silica::EventReply onMouseButtonDown(const Silica::Geometry& geom, const Silica::Vec2& pos, Silica::MouseButton btn) override {
			Silica::EventReply reply = Silica::EventReply::unhandled();
			if (m_child) reply = m_child->onMouseButtonDown(m_child->getAllocatedGeometry(), pos, btn);
			if (reply.isHandled) return reply;

			if (btn == Silica::MouseButton::Left && geom.contains(pos)) {
				m_isMouseDown = true;
				Silica::SWidget::setCapturedWidget(this);
				return Silica::EventReply::handled();
			}
			return reply;
		}

		Silica::EventReply onMouseMove(const Silica::Geometry& geom, const Silica::Vec2& pos) override {
			m_isHovered = geom.contains(pos);

			if (m_isMouseDown && m_onDragStart) {
				m_onDragStart();
				m_isMouseDown = false;
				Silica::SWidget::setCapturedWidget(nullptr);
			}

			if (m_child) return m_child->onMouseMove(m_child->getAllocatedGeometry(), pos);
			return Silica::EventReply::unhandled();
		}

		Silica::EventReply onMouseButtonUp(const Silica::Geometry& geom, const Silica::Vec2& pos, Silica::MouseButton btn) override {
			bool wasClicked = m_isMouseDown;

			if (m_isMouseDown) {
				m_isMouseDown = false;
				Silica::SWidget::setCapturedWidget(nullptr);
			}

			Silica::EventReply reply = Silica::EventReply::unhandled();
			if (m_child) reply = m_child->onMouseButtonUp(m_child->getAllocatedGeometry(), pos, btn);

			if (!reply.isHandled && btn == Silica::MouseButton::Left && wasClicked && geom.contains(pos) && m_onClick) {
				return m_onClick();
			}
			return reply;
		}

	private:

		Silica::WidgetPtr m_child;
		Silica::Color m_hoverColor;
		std::function<void()> m_onDragStart;
		std::function<Silica::EventReply(const Silica::DragDropPayload&)> m_onDragOver;
		std::function<Silica::EventReply(const Silica::DragDropPayload&)> m_onDrop;
		std::function<Silica::EventReply()> m_onClick;
		bool m_isMouseDown = false;
		bool m_isHovered = false;

	};

	class SMouseNavCatcher : public Silica::SWidget {
	public:

		struct Args {
			std::function<void()> onBack;
			std::function<void()> onForward;
			Silica::WidgetPtr child;
		};

		void construct(const Args& args) {
			m_onBack = args.onBack;
			m_onForward = args.onForward;
			m_child = args.child;
		}

		void computeDesiredSize() override {
			if (m_child) {
				m_child->computeDesiredSize();
				m_desiredSize = m_child->getDesiredSize();
			}
		}

		void arrangeChildren(const Silica::Geometry& geo) override { 
			SWidget::arrangeChildren(geo);
			if (m_child) m_child->arrangeChildren(geo);
		}

		void onDraw(Silica::DrawList& dl, const Silica::Geometry& geo) const override {
			if (m_child) m_child->onDraw(dl, geo);
		}

		Silica::EventReply onMouseMove(const Silica::Geometry& geo, const Silica::Vec2& pos) override {
			if (m_child) return m_child->onMouseMove(m_child->getAllocatedGeometry(), pos);
			return Silica::EventReply::unhandled();
		}

		Silica::EventReply onMouseWheel(const Silica::Geometry& geo, const Silica::Vec2& pos, float delta) override {
			if (m_child) return m_child->onMouseWheel(m_child->getAllocatedGeometry(), pos, delta);
			return Silica::EventReply::unhandled();
		}

		Silica::EventReply onMouseButtonUp(const Silica::Geometry& geo, const Silica::Vec2& pos, Silica::MouseButton btn) override {
			if (m_child) return m_child->onMouseButtonUp(m_child->getAllocatedGeometry(), pos, btn);
			return Silica::EventReply::unhandled();
		}

		Silica::EventReply onMouseButtonDown(const Silica::Geometry& geo, const Silica::Vec2& pos, Silica::MouseButton btn) override {
			Silica::EventReply reply = Silica::EventReply::unhandled();
			if (m_child) reply = m_child->onMouseButtonDown(m_child->getAllocatedGeometry(), pos, btn);
			if (reply.isHandled) return reply;

			if (btn == Silica::MouseButton::Side1 && m_onBack) {
				m_onBack();
				return Silica::EventReply::handled();
			}
			if (btn == Silica::MouseButton::Side2 && m_onForward) {
				m_onForward();
				return Silica::EventReply::handled();
			}

			return Silica::EventReply::unhandled();
		}

		Silica::EventReply onDragOver(const Silica::Geometry& geo, const Silica::Vec2& pos, const Silica::DragDropPayload& payload) override {
			if (m_child) return m_child->onDragOver(m_child->getAllocatedGeometry(), pos, payload);
			return Silica::EventReply::unhandled();
		}

		Silica::EventReply onDrop(const Silica::Geometry& geo, const Silica::Vec2& pos, const Silica::DragDropPayload& payload) override {
			if (m_child) return m_child->onDrop(m_child->getAllocatedGeometry(), pos, payload);
			return Silica::EventReply::unhandled();
		}

	private:

		std::function<void()> m_onBack;
		std::function<void()> m_onForward;
		Silica::WidgetPtr m_child;

	};





	// ----- CONTENT BROWSER PANEL IMPLEMENTION -----
	void ContentBrowser::setup() {
		if (ProjectManager::hasProject()) {
			m_rootDirectory = ProjectManager::getProject()->getProjectPath();
			m_currentDirectory = m_rootDirectory;
			refreshDirectory();
		}
	}

	Silica::WidgetPtr ContentBrowser::getWidget() {
		if (!m_uiRoot) {
			m_uiRoot = Silica::MakeWidget<Silica::SBox>({
				.borderThickness = Silica::GetTheme().Border_Thickness
			});
			rebuildUI_Internal();
		}
		return m_uiRoot;
	}

	void ContentBrowser::rebuildUI() {
		if (m_rebuildQueued) return;
		m_rebuildQueued = true;

		EditorActionQueue::push([this]() {
			m_rebuildQueued = false;
			rebuildUI_Internal();
		});
	}

	void ContentBrowser::rebuildUI_Internal() {
		if (!m_uiRoot) return;

		if (m_splitBox) m_treeViewWidth = m_splitBox->getLeftWidth();
		if (m_vSplitBox) m_treeViewTopHeight = m_vSplitBox->getTopHeight();

		if (!ProjectManager::hasProject()) {
			m_uiRoot->setChild(Silica::MakeWidget<Silica::SAlign>({
				.horizontalAlign = Silica::HorizontalAlign::Center,
				.verticalAlign = Silica::VerticalAlign::Center,
				.child = Silica::MakeWidget<Silica::STextBlock>({
					.text = "No Project Loaded.\nPlease load or create a project first."
				})
			}));
			return;
		}

		// -- Collections --
		auto collectionsTreeContainer = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 0.0f});
		if (m_vfs.getRoot()) {
			for (const auto& child : m_vfs.getRoot()->children) {
				if (child->isDirectory) {
					collectionsTreeContainer->addSlot({ {0,0}, buildCollectionTree(child) });
				}
			}
		}

		auto collectionsBackgroundClicker = Silica::MakeWidget<SAssetClickBox>({
			.hoverColor = Silica::Color::transparent(),
			.onDragOver = [](const Silica::DragDropPayload&) { return Silica::EventReply::unhandled(); },
			.onDrop = [](const Silica::DragDropPayload&) { return Silica::EventReply::unhandled(); },
			.onClick = [this]() {
				m_viewingCollection = true;
				m_currentCollection = m_vfs.getRoot();
				refresh();
				return Silica::EventReply::handled();
			},
			.child = collectionsTreeContainer
		});

		auto collectionsTreeScroll = Silica::MakeWidget<Silica::SScrollBox>({
			.child = Silica::MakeWidget<Silica::SBox>({
				.padding = { 5.0f, 5.0f },
				.child = collectionsBackgroundClicker
			})
		});

		auto collectionsTreeBgMenu = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 2.0f });
		collectionsTreeBgMenu->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
			.padding = {8, 4},
			.color = Silica::Color::transparent(),
			.hoverColor = Silica::GetTheme().Accent_Primary,
			.onClick = [this]() {
				EditorActionQueue::push([this]() {
					std::string baseName = "New Collection";
					std::string finalName = baseName;
					int counter = 1;
					auto root = m_vfs.getRoot();
					auto nameExists = [&](const std::string& n) { for (auto& c : root->children) if (c->name == n) return true; return false; };
					while (nameExists(finalName)) finalName = baseName + " " + std::to_string(counter++);

					m_vfs.createFolder(root, finalName);
					m_vfs.save(ProjectManager::getProject()->getProjectPath() / "Config" / "Collections.yaml");

					m_viewingCollection = true;
					m_currentCollection = root;
					m_itemBeingRenamed = finalName;
					m_itemRenameString = finalName;
					m_startRenaming = true;
					refresh();
				});
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Create Collection" })
		}) });

		auto collectionsTreeScrollContext = Silica::MakeWidget<Silica::SMenuAnchor>({
			.openOnHover = false,
			.openOnRightClick = true,
			.openAtMousePos = true,
			.anchorContent = collectionsTreeScroll,
			.menuContent = Silica::MakeWidget<Silica::SBox>({
				.padding = {5, 5},
				.borderThickness = Silica::GetTheme().Border_Thickness,
				.backgroundColor = Silica::GetTheme().Background_Popup,
				.child = collectionsTreeBgMenu
			})
		});

		auto collectionsDropZone = Silica::MakeWidget<Silica::SBox>({
			.onDragOver = [](const Silica::DragDropPayload& payload) {
				if (payload.type == "AssetPath" || payload.type == "VFSNode") return Silica::EventReply::handled();
				return Silica::EventReply::unhandled();
			},
			.onDrop = [this](const Silica::DragDropPayload& payload) mutable {
				auto targetNode = m_vfs.getRoot();

				EditorActionQueue::push([this, payload, targetNode]() mutable {
					if (payload.type == "AssetPath") {
						std::filesystem::path droppedPath = std::any_cast<std::filesystem::path>(payload.data);
						if (!std::filesystem::is_directory(droppedPath)) m_vfs.addFile(targetNode, droppedPath);
					}
					else if (payload.type == "VFSNode") {
						auto droppedNode = std::any_cast<std::shared_ptr<VFSNode>>(payload.data);
						if (droppedNode != targetNode) m_vfs.moveNode(droppedNode, targetNode);
					}

					m_vfs.save(ProjectManager::getProject()->getProjectPath() / "Config" / "Collections.yaml");
					m_viewingCollection = true;
					m_currentCollection = targetNode;
					refresh();
				});
				return Silica::EventReply::handled();
			},
			.child = collectionsTreeScrollContext
		});

		// -- Physical Folder Layout --
		auto physicalBackgroundClicker = Silica::MakeWidget<SAssetClickBox>({
			.hoverColor = Silica::Color::transparent(),
			.onDragOver = [](const Silica::DragDropPayload&) { return Silica::EventReply::unhandled(); },
			.onDrop = [](const Silica::DragDropPayload&) { return Silica::EventReply::unhandled(); },
			.onClick = [this]() {
				m_viewingCollection = false;
				m_currentDirectory = m_rootDirectory;
				refresh();
				return Silica::EventReply::handled();
			},
			.child = buildDirectoryTree(m_rootDirectory)
		});

		auto physicalTreeScroll = Silica::MakeWidget<Silica::SScrollBox>({
			.child = Silica::MakeWidget<Silica::SBox>({
				.padding = { 5.0f, 5.0f },
				.child = physicalBackgroundClicker
			})
		});

		auto physicalTreeBgMenu = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 2.0f });
		physicalTreeBgMenu->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
			.padding = {8, 4}, .color = Silica::Color::transparent(), .hoverColor = Silica::GetTheme().Accent_Primary,
			.onClick = [this]() {
				std::string baseName = "New Folder";
				std::filesystem::path newFolderPath = m_rootDirectory / baseName;
				int counter = 1;
				while (std::filesystem::exists(newFolderPath)) {
					newFolderPath = m_rootDirectory / (baseName + " " + std::to_string(counter));
					++counter;
				}
				std::error_code ec;
				std::filesystem::create_directory(newFolderPath, ec);
				if (!ec) {
					m_viewingCollection = false;
					m_currentDirectory = m_rootDirectory;
					m_itemBeingRenamed = newFolderPath;
					m_itemRenameString = newFolderPath.filename().string();
					m_startRenaming = true;
					refresh();
				}
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Create Folder in Root" })
		}) });

		auto physicalTreeScrollContext = Silica::MakeWidget<Silica::SMenuAnchor>({
			.openOnHover = false, .openOnRightClick = true, .openAtMousePos = true,
			.anchorContent = physicalTreeScroll,
			.menuContent = Silica::MakeWidget<Silica::SBox>({
				.padding = {5, 5}, .borderThickness = Silica::GetTheme().Border_Thickness,
				.backgroundColor = Silica::GetTheme().Background_Popup, .child = physicalTreeBgMenu
			})
		});

		// -- Vertically Split Collections And Physical Folder Layout --
		m_vSplitBox = Silica::MakeWidget<Silica::SVerticalSplitBox>({
			.topHeight = m_treeViewTopHeight,
			.topContent = collectionsTreeScrollContext,
			.bottomContent = physicalTreeScrollContext
		});

		// -- Grid View For Content --
		auto gridView = buildContentArea();

		// -- Horizontally Split Trees And Grid --
		m_splitBox = Silica::MakeWidget<Silica::SHorizontalSplitBox>({
			.leftWidth = m_treeViewWidth,
			.leftContent = Silica::MakeWidget<Silica::SBox>({
				.backgroundColor = Silica::GetTheme().Surface_Primary,
				.child = m_vSplitBox
			}),
			.rightContent = Silica::MakeWidget<Silica::SBox>({
				.backgroundColor = Silica::GetTheme().Surface_Primary,
				.child = gridView
			})
		});

		// -- Assemble Toolbar and Split Boxes --
		auto mainLayout = Silica::MakeWidget<Silica::SBorderLayout>({
			.topBar = buildToolbar(),
			.contentArea = m_splitBox
		});

		// -- Mouse Navigation Catcher--
		auto navCatcher = Silica::MakeWidget<SMouseNavCatcher>({
			.onBack = [this]() {
				if (!m_backHistory.empty()) {
					m_forwardHistory.push_back(m_currentDirectory);
					m_currentDirectory = m_backHistory.back();
					m_backHistory.pop_back();
					refresh();
				}
			},
			.onForward = [this]() {
				if (!m_forwardHistory.empty()) {
					m_backHistory.push_back(m_currentDirectory);
					m_currentDirectory = m_forwardHistory.back();
					m_forwardHistory.pop_back();
					refresh();
				}
			},
			.child = mainLayout
		});

		Silica::WidgetPtr finalContent = navCatcher;

		// -- Delete Popup Modal --
		if (m_openDeletePopup) {
			finalContent = Silica::MakeWidget<Silica::SOverlay>({
				.children = { finalContent, buildDeleteModal() }
			});
		}

		// -- Assemble All --
		m_uiRoot->setChild(Silica::MakeWidget<Silica::SAlign>({
			.horizontalAlign = Silica::HorizontalAlign::Fill,
			.verticalAlign = Silica::VerticalAlign::Fill,
			.child = finalContent
		}));
	}

	Silica::WidgetPtr ContentBrowser::buildToolbar() {
		float iconSize = 24.0f;

		// -- Helper Functions --
		auto makeIconBtn = [=](const std::string& iconName, bool isDisabled, std::function<void()> onClick) {
			return Silica::MakeWidget<Silica::SButton>({
				.padding = { 4.0f, 4.0f },
				.enabled = !isDisabled,
				.color = Silica::Color::transparent(),
				.hoverColor = Silica::Color(100, 100, 100, 150),
				.disabledColor = Silica::Color::transparent(),
				.onClick = [onClick]() {
					onClick();
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::SImage>({
					.textureID = SilicaContext::getIcon(iconName),
					.tint = isDisabled ? Silica::Color(100, 100, 100, 150) : Silica::Color::white(),
					.desiredSize = { iconSize, iconSize }
				})
			});
		};

		auto makeSpacer = []() { return Silica::MakeWidget<Silica::SBox>({
			.backgroundColor = Silica::Color::transparent()
		}); };



		// -- Back Button --
		auto backBtn = makeIconBtn("BackIcon", m_backHistory.empty(), [this]() {
			m_forwardHistory.push_back(m_currentDirectory);
			m_currentDirectory = m_backHistory.back();
			m_backHistory.pop_back();
			refresh();
		});

		// -- Forward Button --
		auto fwdBtn = makeIconBtn("ForwardIcon", m_forwardHistory.empty(), [this]() {
			m_backHistory.push_back(m_currentDirectory);
			m_currentDirectory = m_forwardHistory.back();
			m_forwardHistory.pop_back();
			refresh();
		});

		// -- Refresh Button --
		auto refreshBtn = makeIconBtn("RefreshIcon", false, [this]() {
			refresh();
		});

		// -- Add Folder Button --
		auto addBtn = makeIconBtn("AddFolderIcon", false, [this]() {
			std::string baseName = "New Folder";
			std::filesystem::path newFolderPath = m_currentDirectory / baseName;
			int counter = 1;
			while (std::filesystem::exists(newFolderPath)) {
				newFolderPath = m_currentDirectory / (baseName + " " + std::to_string(counter));
				++counter;
			}
			std::error_code ec;
			std::filesystem::create_directory(newFolderPath, ec);
			if (!ec) {
				m_itemBeingRenamed = newFolderPath;
				m_itemRenameString = newFolderPath.filename().string();
				m_startRenaming = true;
				refresh();
			}
		});

		// -- Search Box --
		auto searchBox = Silica::MakeWidget<Silica::SBox>({
			.explicitSize = Silica::Vec2(200.0f, 0.0f),
			.child = Silica::MakeWidget<Silica::SEditableText>({
				.initialText = m_searchString,
				.hintText = "Search...",
				.onTextCommitted = [this](const std::string& val) {
					m_searchString = val;
					rebuildUI();
				}
			})
		});

		// -- Clear Search Button --
		auto clearSearchBtn = Silica::MakeWidget<Silica::SButton>({
			.padding = { 8.0f, 4.0f },
			.color = m_searchString.empty() ? Silica::Color::transparent() : Silica::Color(50, 50, 50, 255),
			.onClick = [this]() {
				m_searchString.clear();
				rebuildUI();
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "X" })
		});

		// -- Only Engine Assets Checkbox --
		auto onlyEngineAssetsCheckbox = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 5.0f,
			.slots = {
				{ {0,0}, Silica::MakeWidget<Silica::SAlign>({
					.verticalAlign = Silica::VerticalAlign::Center,
					.child = Silica::MakeWidget<Silica::SCheckBox>({
						.initialCheck = m_onlyEngineAssets,
						.onCheckChanged = [this](bool val) { m_onlyEngineAssets = val; rebuildUI(); }
					})
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SAlign>({
					.verticalAlign = Silica::VerticalAlign::Center,
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Assets Only" })
				})}
			}
		});

		// -- Show Extensions Checkbox --
		auto showExtensionsCheckbox = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 5.0f,
			.slots = {
				{ {0,0}, Silica::MakeWidget<Silica::SAlign>({
					.verticalAlign = Silica::VerticalAlign::Center,
					.child = Silica::MakeWidget<Silica::SCheckBox>({
						.initialCheck = m_showFileExtensions,
						.onCheckChanged = [this](bool val) { m_showFileExtensions = val; rebuildUI(); }
					})
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SAlign>({
					.verticalAlign = Silica::VerticalAlign::Center,
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Show Ext" })
				})}
			}
		});


		// -- Assemble --
		return Silica::MakeWidget<Silica::SBox>({
			.padding = { 5.0f, 5.0f },
			.backgroundColor = Silica::GetTheme().Surface_Tertiary,
			.child = Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = 8.0f,
				.slots = {
					{ {0, 0}, backBtn },
					{ {0, 0}, fwdBtn },
					{ {0, 0}, refreshBtn },
					{ {0, 0}, addBtn },
					{ {0, 0}, searchBox },
					{ {0, 0}, clearSearchBtn },
					{ {1, 0}, makeSpacer() },
					{ {0, 0}, onlyEngineAssetsCheckbox },
					{ {0, 0}, showExtensionsCheckbox }
				}
			})
		});
	}

	Silica::WidgetPtr ContentBrowser::buildContentArea() {
		static auto lastClickTime = std::chrono::steady_clock::now();
		static std::filesystem::path lastClickedPath = "";

		auto grid = Silica::MakeWidget<Silica::SWrapBox>({ .spacing = 16.0f });

		for (const auto& item : m_directoryEntries) {
			const auto& path = item.path;
			if (!matchesSearch(item.displayName)) continue;
			if (m_onlyEngineAssets && !item.isDir && !EditorUtils::isEngineAssetExtension(path)) continue;

			Silica::TextureID iconTex = item.isDir ? SilicaContext::getIcon("FolderIcon") : SilicaContext::getIcon("FileIcon");

			// -- Context Menu Builder --
			auto ctxMenu = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 2.0f });

			if (path.extension() == ".axscene") {
				ctxMenu->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = {8, 4},
					.color = Silica::Color::transparent(),
					.hoverColor = Silica::GetTheme().Accent_Primary,
					.onClick = [path]() {
						ProjectManager::getProject()->setDefaultScene(path);
						ProjectManager::saveProject(ProjectManager::getProjectFilePath());
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({ .text = "Set as Default Scene" })
				}) });
			}

			if (path.extension() == ".axmat") {
				ctxMenu->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = {8, 4},
					.color = Silica::Color::transparent(),
					.hoverColor = Silica::GetTheme().Accent_Primary,
					.onClick = [path]() {
						AssetManager::reload<Material>(AssetManager::getAssetUUID(path));
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({ .text = "Reload Material" })
				}) });
			}

			if (!item.isDir) {
				std::string ext = path.extension().string();
				auto openInListContent = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 2.0f });

				// -- Open In Material Editor --
				if (ext == ".axmat") {
					openInListContent->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
						.padding = {8, 4},
						.color = Silica::Color::transparent(),
						.hoverColor = Silica::GetTheme().Accent_Primary,
						.onClick = [this, path]() {
							if (m_openMaterialPanel) m_openMaterialPanel(path);
							return Silica::EventReply::handled();
						},
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Material Editor" })
					}) });
				}

				// -- Open In Viewport --
				if (ext == ".axscene") {
					openInListContent->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
						.padding = {8, 4},
						.color = Silica::Color::transparent(),
						.hoverColor = Silica::GetTheme().Accent_Primary,
						.onClick = [this, path]() {
							m_openInViewportCallback(path);
							return Silica::EventReply::handled();
						},
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Viewport" })
					}) });
				}

				// -- Open In Visual Script Editor --
				if (ext == ".axvs") {
					openInListContent->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
						.padding = {8, 4},
						.color = Silica::Color::transparent(),
						.hoverColor = Silica::GetTheme().Accent_Primary,
						.onClick = [this, path]() {
							if (m_openVisualScriptPanel) m_openVisualScriptPanel(path);
							return Silica::EventReply::handled();
						},
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Visual Script Editor" })
					}) });
				}

				// -- Open In Quartz Editor --
				if (EditorUtils::isTextEditorFile(ext)) {
					openInListContent->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
						.padding = {8, 4},
						.color = Silica::Color::transparent(),
						.hoverColor = Silica::GetTheme().Accent_Primary,
						.onClick = [this, path]() {
							if (m_openTextFile) m_openTextFile(path);
							return Silica::EventReply::handled();
						},
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Quartz Editor" })
					}) });
				}

				// -- Open In External Program --
				std::string progName = PlatformUtils::getDefaultProgramName(path);
				std::string btnText = progName.empty() ? "System Default (External)" : progName + " (External)";

				openInListContent->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = {8, 4},
					.color = Silica::Color::transparent(),
					.hoverColor = Silica::GetTheme().Accent_Primary,
					.onClick = [path]() {
						PlatformUtils::openExternally(path);
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = btnText })
				}) });

				// -- Create Submenu Anchor --
				auto openInSubMenu = Silica::MakeWidget<Silica::SMenuAnchor>({
					.openOnHover = true,
					.openToRight = true,
					.showArrow = true,
					.anchorContent = Silica::MakeWidget<Silica::SButton>({
						.padding = {8, 4},
						.color = Silica::Color::transparent(),
						.hoverColor = Silica::GetTheme().Accent_Primary,
						.onClick = []() { return Silica::EventReply::unhandled(); },
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Open in..." })
					}),
					.menuContent = Silica::MakeWidget<Silica::SBox>({
						.padding = {5, 5},
						.borderThickness = Silica::GetTheme().Border_Thickness,
						.backgroundColor = Silica::GetTheme().Background_Popup,
						.child = openInListContent
					})
				});

				ctxMenu->addSlot({ {0,0}, openInSubMenu });
			}

			ctxMenu->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
				.padding = {8, 4},
				.color = Silica::Color::transparent(),
				.hoverColor = Silica::GetTheme().Accent_Primary,
				.onClick = [path]() {
					PlatformUtils::showInFileExplorer(path);
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::STextBlock>({ .text = "Show in Explorer" })
			}) });

			ctxMenu->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
				.padding = {8, 4},
				.color = Silica::Color::transparent(),
				.hoverColor = Silica::GetTheme().Accent_Primary,
				.onClick = [this, path, item]() {
					m_itemBeingRenamed = path;
					m_itemRenameString = path.filename().string();
					m_startRenaming = true;
					rebuildUI();
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::STextBlock>({ .text = "Rename" })
			}) });

			ctxMenu->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
				.padding = {8, 4},
				.color = Silica::Color::transparent(),
				.hoverColor = Silica::GetTheme().Accent_Danger,
				.onClick = [this, path, item]() {
					if (m_viewingCollection && item.vfsNode) {
						// -- VFS Delete --
						EditorActionQueue::push([this, item]() {
							auto& siblings = item.vfsNode->parent->children;
							siblings.erase(std::remove(siblings.begin(), siblings.end(), item.vfsNode), siblings.end());
							std::filesystem::path vfsPath = ProjectManager::getProject()->getProjectPath() / "Config" / "Collections.yaml";
							m_vfs.save(vfsPath);
							refresh();
						});
					}
					else {
						// -- Physical Delete --
						m_pendingDelete = path;
						m_relatedFilesToDelete = findRelatedFiles(path);
						m_deleteRelatedFiles = true;
						m_openDeletePopup = true;
						rebuildUI();
					}
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Delete" })
			}) });


			// -- Item Label Or Rename Box --
			Silica::WidgetPtr labelWidget = nullptr;
			if (m_itemBeingRenamed == path) {
				labelWidget = Silica::MakeWidget<Silica::SBox>({
					.explicitSize = Silica::Vec2(m_thumbnailSize + 30.0f, 0.0f),
					.child = Silica::MakeWidget<Silica::SEditableText>({
						.initialText = m_itemRenameString,
						.onTextCommitted = [this, path, item](const std::string& newText) {
							if (m_viewingCollection && item.vfsNode) {
								// -- Rename Virtual Node --
								item.vfsNode->name = newText;
								std::filesystem::path vfsPath = ProjectManager::getProject()->getProjectPath() / "Config" / "Collections.yaml";
								m_vfs.save(vfsPath);
								resetRenaming();
								refresh();
								return;
							}

							// -- Rename Physical --
							std::filesystem::path newPath = path.parent_path() / newText;
							if (!item.isDir && path.has_extension()) {
								std::string ext = path.extension().string();
								if (newPath.extension().string() != ext) newPath += ext;
							}

							std::error_code ec;
							if (!std::filesystem::exists(newPath, ec)) {

								std::filesystem::path oldLayoutPath, newLayoutPath;
								std::filesystem::path oldCSPath, newCSPath;
								if (path.extension() == ".axvs") {
									oldLayoutPath = path.parent_path() / (path.stem().string() + "_layout.axvslayout");
									newLayoutPath = newPath.parent_path() / (newPath.stem().string() + "_layout.axvslayout");

									std::filesystem::path scriptsDir = ProjectManager::getProject()->getProjectPath() / "Scripts";
									oldCSPath = scriptsDir / (path.stem().string() + ".cs");
									newCSPath = scriptsDir / (newPath.stem().string() + ".cs");
								}

								// -- Rename Main Asset --
								std::filesystem::rename(path, newPath, ec);

								if (!ec) {
									// -- Rename Layout File --
									if (!oldLayoutPath.empty() && std::filesystem::exists(oldLayoutPath)) {
										std::error_code layoutEc;
										std::filesystem::rename(oldLayoutPath, newLayoutPath, layoutEc);
									}

									// -- Rename C# Script --
									if (!oldCSPath.empty() && std::filesystem::exists(oldCSPath)) {
										std::error_code csEc;
										std::filesystem::rename(oldCSPath, newCSPath, csEc);
									}

									// -- Notify Editor That Rename Occurred --
									if (m_onAssetRenamed) {
										m_onAssetRenamed(path, newPath);
									}
								}
							}
							resetRenaming();
							refresh();
						}
					})
				});
			}
			else {
				std::string displayName = m_showFileExtensions ? path.filename().string() : path.stem().string();
				labelWidget = Silica::MakeWidget<Silica::SBox>({
					.explicitSize = Silica::Vec2(m_thumbnailSize + 30.0f, 0.0f),
					.child = Silica::MakeWidget<Silica::SAlign>({
						.horizontalAlign = Silica::HorizontalAlign::Center,
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = displayName })
					})
				});
			}


			// -- Clickable Folder / File --
			auto assetClickBox = Silica::MakeWidget<SAssetClickBox>({
				.onDragStart = [this, path, item]() {
					if (m_viewingCollection && item.vfsNode) {
						Silica::DragDrop::beginDrag("VFSNode", item.vfsNode, item.displayName, Silica::GetTheme().Font_Default);
					}
					else {
						Silica::DragDrop::beginDrag("AssetPath", path, path.filename().string(), Silica::GetTheme().Font_Default);
					}
				},
				.onDragOver = [this, item](const Silica::DragDropPayload& payload) {
					if (item.isDir && m_viewingCollection && item.vfsNode) {
						if (payload.type == "AssetPath" || payload.type == "VFSNode") return Silica::EventReply::handled();
					}
					return Silica::EventReply::unhandled();
				},
				.onDrop = [this, item](const Silica::DragDropPayload& payload) mutable {
					if (item.isDir && m_viewingCollection && item.vfsNode) {
						auto targetNode = item.vfsNode;
			
						EditorActionQueue::push([this, payload, targetNode]() mutable {
							if (payload.type == "AssetPath") {
								m_vfs.addFile(targetNode, std::any_cast<std::filesystem::path>(payload.data));
							}
							else if (payload.type == "VFSNode") {
								auto droppedNode = std::any_cast<std::shared_ptr<VFSNode>>(payload.data);
								if (droppedNode != targetNode) m_vfs.moveNode(droppedNode, targetNode);
							}
							m_vfs.save(ProjectManager::getProject()->getProjectPath() / "Config" / "Collections.yaml");
							refresh();
						});
						return Silica::EventReply::handled();
					}
					return Silica::EventReply::unhandled();
				},
				.onClick = [this, path, item]() {
					auto now = std::chrono::steady_clock::now();
					bool isDouble = (path == lastClickedPath) && (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastClickTime).count() < 300);
					lastClickTime = now;
					lastClickedPath = path;

					if (isDouble) {
						if (item.isDir) {
							if (m_viewingCollection && item.vfsNode) {
								m_currentCollection = item.vfsNode;
							}
							else {
								m_backHistory.push_back(m_currentDirectory);
								m_forwardHistory.clear();
								m_currentDirectory = path;
								m_expandedDirectories.insert(path.string());
							}
							refresh();
						}
						else {
							std::string ext = path.extension().string();

							if (ext == ".axscene") {
								m_openInViewportCallback(path);
							}
							else if (ext == ".axvs" && m_openVisualScriptPanel) {
								m_openVisualScriptPanel(path);
							}
							else if (ext == ".axmat" && m_openMaterialPanel) {
								m_openMaterialPanel(path);
							}
							else if (EditorUtils::isTextEditorFile(ext) && m_openTextFile) {
								m_openTextFile(path);
							}
							else if (EditorUtils::isEngineAssetExtension(ext) && m_openTextFile) {
								m_openTextFile(path);
							}
							else {
								PlatformUtils::openExternally(path);
							}
						}
					}
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::SVerticalBox>({
					.spacing = 4.0f,
					.slots = {
						{ {0,0}, Silica::MakeWidget<Silica::SAlign>({
							.horizontalAlign = Silica::HorizontalAlign::Center,
							.child = Silica::MakeWidget<Silica::SImage>({
								.textureID = iconTex,
								.desiredSize = {m_thumbnailSize, m_thumbnailSize} 
							})
						})},
						{ {0,0}, labelWidget }
					}
				}),
			});

			auto itemBox = Silica::MakeWidget<Silica::SMenuAnchor>({
				.openOnHover = false,
				.openOnRightClick = true,
				.openAtMousePos = true,
				.anchorContent = assetClickBox,
				.menuContent = Silica::MakeWidget<Silica::SBox>({
					.padding = {5,5},
					.borderThickness = Silica::GetTheme().Border_Thickness,
					.backgroundColor = Silica::GetTheme().Background_Popup,
					.child = ctxMenu
				})
			});

			grid->addChild(itemBox);
		}

		// -- Background Area Context Menu --
		auto bgMenuContent = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 2.0f });

		bgMenuContent->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
			.padding = {8, 4},
			.color = Silica::Color::transparent(),
			.hoverColor = Silica::GetTheme().Accent_Primary,
			.onClick = [this]() {
				std::string baseName = "NewVisualScript";
				std::filesystem::path newScriptPath = m_currentDirectory / (baseName + ".axvs");
				int counter = 1;
				while (std::filesystem::exists(newScriptPath)) {
					newScriptPath = m_currentDirectory / (baseName + "_" + std::to_string(counter) + ".axvs");
					counter++;
				}
				VisualGraph newGraph;
				newGraph.className = newScriptPath.stem().string();
				VisualScriptSerializer::serialize(newGraph, newScriptPath);

				m_itemBeingRenamed = newScriptPath;
				m_itemRenameString = newScriptPath.filename().string();
				m_startRenaming = true;
				refresh();
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({ .text = "Create Visual Script" })
		}) });


		// -- Audio Import --
		bgMenuContent->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
			.padding = {8, 4},
			.color = Silica::Color::transparent(),
			.hoverColor = Silica::GetTheme().Accent_Primary,
			.onClick = [this]() {
				std::filesystem::path audioDir = ProjectManager::getProject()->getAssetsPath() / "audio";
				std::filesystem::path absPath = std::filesystem::exists(audioDir) ?
					FileDialogs::openFile({ {"Audio Files", "*.mp3;*.wav;*.ogg"} }, audioDir) :
					FileDialogs::openFile({ {"Audio Files", "*.mp3;*.wav;*.ogg"} }, ProjectManager::getProject()->getAssetsPath());

				if (!absPath.empty() && m_openGlobalModal) {
					m_audioImportModal = std::make_shared<AudioImportModal>();
					m_audioImportModal->presetFromFile(absPath);

					auto modalWidget = m_audioImportModal->getWidget([this]() {
						EditorActionQueue::push([this]() {
							if (m_closeGlobalModal) m_closeGlobalModal();
							m_audioImportModal = nullptr;
							refresh();
						});
					});

					m_openGlobalModal(modalWidget);
				}
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({ .text = "Import Audio" })
		}) });


		// -- Material Import --
		bgMenuContent->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
			.padding = {8, 4},
			.color = Silica::Color::transparent(),
			.hoverColor = Silica::GetTheme().Accent_Primary,
			.onClick = [this]() {
				if (m_openGlobalModal) {
					m_materialImportModal = std::make_shared<MaterialImportModal>();

					auto modalWidget = m_materialImportModal->getWidget([this]() {
						EditorActionQueue::push([this]() {
							if (m_closeGlobalModal) m_closeGlobalModal();
							m_materialImportModal = nullptr;
							refresh();
						});
					});

					m_openGlobalModal(modalWidget);
				}
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({ .text = "Create Material" })
		}) });


		// -- Mesh Import --
		bgMenuContent->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
			.padding = {8, 4},
			.color = Silica::Color::transparent(),
			.hoverColor = Silica::GetTheme().Accent_Primary,
			.onClick = [this]() {
				std::filesystem::path meshDir = ProjectManager::getProject()->getAssetsPath() / "meshes";
				std::filesystem::path absPath = std::filesystem::exists(meshDir) ?
					FileDialogs::openFile({ {"3D Models", "*.obj;*.gltf;*.glb"} }, meshDir) :
					FileDialogs::openFile({ {"3D Models", "*.obj;*.gltf;*.glb"} }, ProjectManager::getProject()->getAssetsPath());

				if (!absPath.empty() && m_openGlobalModal) {
					m_meshImportModal = std::make_shared<MeshImportModal>();
					m_meshImportModal->presetFromFile(absPath);

					auto modalWidget = m_meshImportModal->getWidget([this]() {
						EditorActionQueue::push([this]() {
							if (m_closeGlobalModal) m_closeGlobalModal();
							m_meshImportModal = nullptr;
							refresh();
						});
					});

					m_openGlobalModal(modalWidget);
				}

				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Import Mesh" })
		}) });


		// -- Create Physics Material --
		bgMenuContent->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
			.padding = {8, 4},
			.color = Silica::Color::transparent(),
			.hoverColor = Silica::GetTheme().Accent_Primary,
			.onClick = [this]() {
				if (m_openGlobalModal) {
					m_physicsMaterialModal = std::make_shared<PhysicsMaterialImportModal>();

					auto modalWidget = m_physicsMaterialModal->getWidget([this]() {
						EditorActionQueue::push([this]() {
							if (m_closeGlobalModal) m_closeGlobalModal();
							m_physicsMaterialModal = nullptr;
							refresh();
						});
					});

					m_openGlobalModal(modalWidget);
				}
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Create Physics Material" })
		}) });


		// -- Create Pipeline --
		bgMenuContent->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
			.padding = {8, 4},
			.color = Silica::Color::transparent(),
			.hoverColor = Silica::GetTheme().Accent_Primary,
			.onClick = [this]() {
				if (m_openGlobalModal) {
					m_pipelineImportModal = std::make_shared<PipelineImportModal>();

					auto modalWidget = m_pipelineImportModal->getWidget([this]() {
						EditorActionQueue::push([this]() {
							if (m_closeGlobalModal) m_closeGlobalModal();
							m_pipelineImportModal = nullptr;
							refresh();
						});
					});

					m_openGlobalModal(modalWidget);
				}
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Create Pipeline" })
		}) });


		// -- Shader Import --
		bgMenuContent->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
			.padding = {8, 4},
			.color = Silica::Color::transparent(),
			.hoverColor = Silica::GetTheme().Accent_Primary,
			.onClick = [this]() {
				if (m_openGlobalModal) {
					m_shaderImportModal = std::make_shared<ShaderImportModal>();

					auto modalWidget = m_shaderImportModal->getWidget([this]() {
						EditorActionQueue::push([this]() {
							if (m_closeGlobalModal) m_closeGlobalModal();
							m_shaderImportModal = nullptr;
							refresh();
						});
					});

					m_openGlobalModal(modalWidget);
				}
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Import Shader" })
		}) });


		// -- Create Skybox --
		bgMenuContent->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
			.padding = {8, 4},
			.color = Silica::Color::transparent(),
			.hoverColor = Silica::GetTheme().Accent_Primary,
			.onClick = [this]() {
				if (m_openGlobalModal) {
					m_skyboxImportModal = std::make_shared<SkyboxImportModal>();

					auto modalWidget = m_skyboxImportModal->getWidget([this]() {
						EditorActionQueue::push([this]() {
							if (m_closeGlobalModal) m_closeGlobalModal();
							m_skyboxImportModal = nullptr;
							refresh();
						});
					});

					m_openGlobalModal(modalWidget);
				}
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Create Skybox" })
		}) });


		// -- Texture2D Import --
		bgMenuContent->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
			.padding = {8, 4},
			.color = Silica::Color::transparent(),
			.hoverColor = Silica::GetTheme().Accent_Primary,
			.onClick = [this]() {
				if (m_openGlobalModal) {
					m_texture2DImportModal = std::make_shared<Texture2DImportModal>();

					auto modalWidget = m_texture2DImportModal->getWidget([this]() {
						EditorActionQueue::push([this]() {
							if (m_closeGlobalModal) m_closeGlobalModal();
							m_texture2DImportModal = nullptr;
							refresh();
						});
					});

					m_openGlobalModal(modalWidget);
				}
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Import Texture 2D" })
		}) });


		// -- Texture Cube Import --
		bgMenuContent->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
			.padding = {8, 4},
			.color = Silica::Color::transparent(),
			.hoverColor = Silica::GetTheme().Accent_Primary,
			.onClick = [this]() {
				if (m_openGlobalModal) {
					m_textureCubeImportModal = std::make_shared<TextureCubeImportModal>();

					auto modalWidget = m_textureCubeImportModal->getWidget([this]() {
						EditorActionQueue::push([this]() {
							if (m_closeGlobalModal) m_closeGlobalModal();
							m_textureCubeImportModal = nullptr;
							refresh();
						});
					});

					m_openGlobalModal(modalWidget);
				}
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Import Texture Cube" })
		}) });


		// -- Assemble --
		auto backgroundMenu = Silica::MakeWidget<Silica::SMenuAnchor>({
			.openOnHover = false,
			.openOnRightClick = true,
			.openAtMousePos = true,
			.anchorContent = Silica::MakeWidget<Silica::SScrollBox>({
				.child = grid
			}),
			.menuContent = Silica::MakeWidget<Silica::SBox>({
				.padding = {5,5},
				.borderThickness = Silica::GetTheme().Border_Thickness,
				.backgroundColor = Silica::GetTheme().Background_Popup,
				.child = bgMenuContent
			})
		});

		auto zoomBox = Silica::MakeWidget<Silica::SScrollCatcher>({
			.onMouseWheel = [this](float delta) {
				bool isCtrlDown = Input::isKeyPressed(KeyCode::LeftControl) || Input::isKeyPressed(KeyCode::RightControl);
				if (isCtrlDown) {
					m_thumbnailSize += delta * 5.0f;
					m_thumbnailSize = std::clamp(m_thumbnailSize, 32.0f, 256.0f);
					m_showNames = m_thumbnailSize >= 50.0f;
					rebuildUI();
					return Silica::EventReply::handled();
				}
				return Silica::EventReply::unhandled();
			},
			.child = backgroundMenu
		});

		auto gridBackgroundDropZone = Silica::MakeWidget<Silica::SBox>({
			.onDragOver = [this](const Silica::DragDropPayload& payload) {
				if (m_viewingCollection && m_currentCollection) {
					if (payload.type == "AssetPath" || payload.type == "VFSNode") return Silica::EventReply::handled();
				}
				return Silica::EventReply::unhandled();
			},
			.onDrop = [this](const Silica::DragDropPayload& payload) mutable {
				if (m_viewingCollection && m_currentCollection) {
					EditorActionQueue::push([this, payload]() mutable {
						if (payload.type == "AssetPath") {
							m_vfs.addFile(m_currentCollection, std::any_cast<std::filesystem::path>(payload.data));
						}
						else if (payload.type == "VFSNode") {
							auto droppedNode = std::any_cast<std::shared_ptr<VFSNode>>(payload.data);
							if (droppedNode != m_currentCollection) m_vfs.moveNode(droppedNode, m_currentCollection);
						}
						m_vfs.save(ProjectManager::getProject()->getProjectPath() / "Config" / "Collections.yaml");
						refresh();
					});
					return Silica::EventReply::handled();
				}
				return Silica::EventReply::unhandled();
			},
			.child = zoomBox
		});

		return gridBackgroundDropZone;
	}

	Silica::WidgetPtr ContentBrowser::buildDeleteModal() {
		auto modalContent = Silica::MakeWidget<Silica::SVerticalBox>({
			.spacing = 10.0f,
			.slots = {
				{ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "Are you sure you want to delete\n" + m_pendingDelete->filename().string() + "?" }) }
			}
		});

		if (!m_relatedFilesToDelete.empty()) {
			modalContent->addSlot({ {0,0}, Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = 5.0f,
				.slots = {
					{ {0,0}, Silica::MakeWidget<Silica::SCheckBox>({
						.initialCheck = m_deleteRelatedFiles,
						.onCheckChanged = [this](bool val) { m_deleteRelatedFiles = val; rebuildUI(); }
					})},
					{ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "Delete related files" })}
				}
			}) });
		}

		auto buttons = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 10.0f,
			.slots = {
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 20.0f, 8.0f },
					.color = Silica::GetTheme().Accent_Danger,
					.onClick = [this]() {
						if (m_onAssetDeleted) m_onAssetDeleted(*m_pendingDelete);
						if (EditorUtils::isEngineAssetExtension(*m_pendingDelete)) {
							UUID assetUUID = AssetManager::getAssetUUID(*m_pendingDelete);
							if (assetUUID.isValid()) {
								AssetManager::removeAsset(assetUUID);
								ProjectManager::saveProject(ProjectManager::getProjectFilePath());
							}
						}
						deletePath(*m_pendingDelete);
						if (m_deleteRelatedFiles) {
							for (const auto& rel : m_relatedFilesToDelete) deletePath(rel);
						}
						if (m_pendingDelete->extension() == ".axvs" && m_deleteRelatedFiles) {
							ProjectManager::triggerScriptAssemblyLoad();
						}
						m_pendingDelete.reset();
						m_relatedFilesToDelete.clear();
						m_openDeletePopup = false;
						refresh();
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Delete" })
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 20.0f, 8.0f },
					.onClick = [this]() {
						m_pendingDelete.reset(); m_openDeletePopup = false; rebuildUI();
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Cancel" })
				})}
			}
		});

		modalContent->addSlot({ {0,0}, buttons });

		auto modalBox = Silica::MakeWidget<Silica::SBox>({
			.padding = { 20.0f, 20.0f },
			.child = modalContent
		});

		return Silica::MakeWidget<Silica::SAlign>({
			.horizontalAlign = Silica::HorizontalAlign::Center,
			.verticalAlign = Silica::VerticalAlign::Center,
			.child = modalBox
		});
	}

	void ContentBrowser::onEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<ProjectChangedEvent>(AX_BIND_EVENT_FN(ContentBrowser::onProjectChanged));
	}

	void ContentBrowser::refreshDirectory() {
		std::vector<DirItem> tmp;

		if (m_viewingCollection && m_currentCollection) {
			// -- Collections Grid --
			for (const auto& child : m_currentCollection->children) {
				DirItem di;
				di.path = child->isDirectory ? child->name : child->physicalPath;
				di.isDir = child->isDirectory;
				di.displayName = child->name;
				di.vfsNode = child;
				tmp.push_back(std::move(di));
			}
		}
		else {
			// -- Physical Grid --
			std::error_code ec;
			std::filesystem::directory_iterator it(m_currentDirectory, std::filesystem::directory_options::skip_permission_denied, ec);
			if (ec) { m_directoryEntries.clear(); return; }

			for (auto end = std::filesystem::directory_iterator(); it != end; it.increment(ec)) {
				if (ec) { ec.clear(); continue; }
				const auto p = it->path();

				// -- Hide Certain Files And Folders --
				std::string filename = p.filename().string();
				std::string ext = p.extension().string();
				if (it->is_directory(ec)) {
					// -- Compilation Folders And Hidden System Folders --
					if (filename == "bin" || filename == "obj" || filename[0] == '.' || filename == "Export") {
						continue;
					}
				}
				else {
					// -- Compilation Files And Project Files --
					if (ext == ".dll" || ext == ".pdb" || ext == ".csproj" || ext == ".sln" || ext == ".cache" || ext == ".axproj") {
						continue;
					}
				}

				DirItem di;
				di.path = p;
				di.isDir = it->is_directory(ec);
				auto rel = p.lexically_relative(m_rootDirectory);
				di.displayName = rel.empty() ? p.filename().string() : rel.filename().string();
				tmp.push_back(std::move(di));
			}
		}

		m_directoryEntries.swap(tmp);
	}

	void ContentBrowser::resetRenaming() {
		m_itemBeingRenamed.clear();
		m_itemRenameString.clear();
		m_startRenaming = false;
	}

	bool ContentBrowser::matchesSearch(const std::string& name) {
		if (m_searchString.empty()) return true;
		auto lower = [](std::string s) { std::transform(s.begin(), s.end(), s.begin(), ::tolower); return s; };
		return lower(name).find(lower(m_searchString)) != std::string::npos;
	}

	void ContentBrowser::deletePath(const std::filesystem::path& path) {
		std::error_code ec;
		if (std::filesystem::is_directory(path, ec) && !ec) std::filesystem::remove_all(path, ec);
		else std::filesystem::remove(path, ec);
	}

	bool ContentBrowser::onProjectChanged(ProjectChangedEvent& e) {
		if (ProjectManager::hasProject()) {
			std::error_code ec;
			m_rootDirectory = std::filesystem::weakly_canonical(ProjectManager::getProject()->getProjectPath(), ec);
			if (ec) m_rootDirectory = std::filesystem::absolute(ProjectManager::getProject()->getProjectPath(), ec);
			m_currentDirectory = m_rootDirectory;

			// -- Load Collections --
			std::filesystem::path vfsPath = ProjectManager::getProject()->getProjectPath() / "Config" / "Collections.yaml";
			if (std::filesystem::exists(vfsPath)) {
				m_vfs.load(vfsPath);
			}
			else {
				m_vfs.getRoot()->name = "Collections";
				std::filesystem::create_directories(vfsPath.parent_path());
				m_vfs.save(vfsPath);
			}

			m_viewingCollection = false;
			m_currentCollection = m_vfs.getRoot();

			m_expandedDirectories.clear();
			m_expandedDirectories.insert(m_rootDirectory.string());

			refresh();
		}
		else {
			m_directoryEntries.clear();
			m_rootDirectory.clear();
			m_currentDirectory.clear();
		}

		m_backHistory.clear();
		m_forwardHistory.clear();
		return false;
	}

	void ContentBrowser::refresh() {
		refreshDirectory();
		rebuildUI();
	}

	std::vector<std::filesystem::path> ContentBrowser::findRelatedFiles(const std::filesystem::path& path) {
		std::vector<std::filesystem::path> related;

		if (path.extension() == ".axvs") {
			// -- Delete Layout File --
			std::filesystem::path layoutPath = path.parent_path() / (path.stem().string() + "_layout.axvslayout");
			if (std::filesystem::exists(layoutPath)) {
				related.push_back(layoutPath);
			}

			// -- Delete C# Script --
			if (ProjectManager::hasProject()) {
				std::filesystem::path scriptsDir = ProjectManager::getProject()->getProjectPath() / "Scripts";
				std::filesystem::path csPath = scriptsDir / (path.stem().string() + ".cs");
				if (std::filesystem::exists(csPath)) {
					related.push_back(csPath);
				}
			}
		}

		return related;
	}

	void ContentBrowser::loadSettings(const YAML::Node& editorConfig) {
		if (auto cbConfig = editorConfig["ContentBrowser"]) {
			if (cbConfig["ShowFileExtensions"]) m_showFileExtensions = cbConfig["ShowFileExtensions"].as<bool>();
			if (cbConfig["OnlyEngineAssets"]) m_onlyEngineAssets = cbConfig["OnlyEngineAssets"].as<bool>();
			if (cbConfig["ThumbnailSize"]) m_thumbnailSize = cbConfig["ThumbnailSize"].as<float>();
			if (cbConfig["TreeViewWidth"]) m_treeViewWidth = cbConfig["TreeViewWidth"].as<float>();
			if (cbConfig["TreeViewTopHeight"]) m_treeViewTopHeight = cbConfig["TreeViewTopHeight"].as<float>();
		}
	}

	void ContentBrowser::saveSettings(YAML::Emitter& out) const {
		float currentWidth = m_splitBox ? m_splitBox->getLeftWidth() : m_treeViewWidth;
		float currentTopHeight = m_vSplitBox ? m_vSplitBox->getTopHeight() : m_treeViewTopHeight;
		out << YAML::Key << "ContentBrowser" << YAML::Value << YAML::BeginMap;
		out << YAML::Key << "ShowFileExtensions" << YAML::Value << m_showFileExtensions;
		out << YAML::Key << "OnlyEngineAssets" << YAML::Value << m_onlyEngineAssets;
		out << YAML::Key << "ThumbnailSize" << YAML::Value << m_thumbnailSize;
		out << YAML::Key << "TreeViewWidth" << YAML::Value << currentWidth;
		out << YAML::Key << "TreeViewTopHeight" << YAML::Value << currentTopHeight;
		out << YAML::EndMap;
	}

	Silica::WidgetPtr ContentBrowser::buildDirectoryTree(const std::filesystem::path& dirPath) {
		std::string pathStr = dirPath.string();

		bool isOpen = m_expandedDirectories.find(pathStr) != m_expandedDirectories.end();
		bool isSelected = (!m_viewingCollection && dirPath == m_currentDirectory);

		std::vector<std::filesystem::path> subDirectories;
		std::vector<std::filesystem::path> subFiles;
		std::error_code ec;
		std::filesystem::directory_iterator it(dirPath, std::filesystem::directory_options::skip_permission_denied, ec);

		if (!ec) {
			for (const auto& entry : it) {
				std::string name = entry.path().filename().string();
				std::string ext = entry.path().extension().string();

				// -- Skip Hidden Items And Build Artifacts --
				if (name[0] == '.' || name == "bin" || name == "obj") continue;
				if (!entry.is_directory() && (ext == ".dll" || ext == ".pdb" || ext == ".csproj" || ext == ".sln" || ext == ".cache")) continue;

				if (entry.is_directory()) {
					subDirectories.push_back(entry.path());
				}
				else {
					if (m_onlyEngineAssets && !EditorUtils::isEngineAssetExtension(ext)) continue;
					if (matchesSearch(name)) {
						subFiles.push_back(entry.path());
					}
				}
			}
		}

		// -- Sort Alphabetically --
		std::sort(subDirectories.begin(), subDirectories.end());
		std::sort(subFiles.begin(), subFiles.end());

		std::vector<Silica::WidgetPtr> childWidgets;
		if (isOpen) {
			// -- Add Subdirectories --
			for (const auto& subDir : subDirectories) {
				childWidgets.push_back(buildDirectoryTree(subDir));
			}

			// -- Add Files --
			for (const auto& filePath : subFiles) {
				auto fileIcon = Silica::MakeWidget<Silica::SImage>({
					.textureID = SilicaContext::getIcon("FileIcon"),
					.tint = Silica::GetTheme().Text_Main,
					.desiredSize = { 16.0f, 16.0f },
				});

				std::string fileName = m_showFileExtensions ? filePath.filename().string() : filePath.stem().string();

				auto fileNode = Silica::MakeWidget<Silica::STreeNode>({
					.label = fileName,
					.initiallyOpen = false,
					.isSelected = false,
					.isLeaf = true,
					.isEmpty = true,
					.isDragged = [filePath]() {
						return Silica::DragDrop::isDraggingType("AssetPath") && std::any_cast<std::filesystem::path>(Silica::DragDrop::getPayload().data) == filePath;
					},
					.onDragStart = [filePath, fileName]() {
						Silica::DragDrop::beginDrag("AssetPath", filePath, fileName, Silica::GetTheme().Font_Default);
					},
					.leadingWidget = fileIcon
				});

				childWidgets.push_back(fileNode);
			}
		}

		// -- Icon For Folder --
		auto folderIcon = Silica::MakeWidget<Silica::SImage>({
			.textureID = SilicaContext::getIcon("FolderIcon"),
			.tint = isSelected ? Silica::GetTheme().Text_Main : Silica::GetTheme().Text_Dim,
			.desiredSize = { 16.0f, 16.0f },
		});

		std::string label = (dirPath == m_rootDirectory) ? ProjectManager::getProject()->getName() : dirPath.filename().string();
		bool hasChildren = !subDirectories.empty() || !subFiles.empty();

		return Silica::MakeWidget<Silica::STreeNode>({
			.label = label,
			.initiallyOpen = isOpen,
			.isSelected = isSelected,
			.isLeaf = !hasChildren,
			.isEmpty = !hasChildren,
			.onClicked = [this, dirPath]() {
				m_viewingCollection = false;

				if (m_currentDirectory != dirPath) {
					m_backHistory.push_back(m_currentDirectory);
					m_forwardHistory.clear();
					m_currentDirectory = dirPath;
				}
				refresh();
			},
			.onToggleOpen = [this, pathStr](bool open) {
				if (open) m_expandedDirectories.insert(pathStr);
				else m_expandedDirectories.erase(pathStr);
				rebuildUI();
			},
			.leadingWidget = folderIcon,
			.children = childWidgets
		});
	}

	Silica::WidgetPtr ContentBrowser::buildCollectionTree(std::shared_ptr<VFSNode> node) {
		if (!node) return nullptr;

		std::vector<Silica::WidgetPtr> childWidgets;
		for (const auto& child : node->children) {
			if (child->isDirectory) childWidgets.push_back(buildCollectionTree(child));
		}

		auto folderIcon = Silica::MakeWidget<Silica::SImage>({
			.textureID = SilicaContext::getIcon("FolderIcon"),
			.tint = (m_viewingCollection && m_currentCollection == node) ? Silica::GetTheme().Text_Main : Silica::GetTheme().Text_Dim,
			.desiredSize = { 16.0f, 16.0f },
		});

		auto treeNode = Silica::MakeWidget<Silica::STreeNode>({
			.label = node->name,
			.initiallyOpen = true,
			.isSelected = (m_viewingCollection && m_currentCollection == node),
			.isLeaf = childWidgets.empty(),
			.isEmpty = childWidgets.empty(),
			.isDragged = [node]() {
				return Silica::DragDrop::isDraggingType("VFSNode") && std::any_cast<std::shared_ptr<VFSNode>>(Silica::DragDrop::getPayload().data) == node;
			},
			.onClicked = [this, node]() {
				m_viewingCollection = true;
				m_currentCollection = node;
				refresh();
			},
			.onDragStart = [node]() {
				Silica::DragDrop::beginDrag("VFSNode", node, node->name, Silica::GetTheme().Font_Default);
			},
			.onDragOver = [](const Silica::DragDropPayload& payload) {
				if (payload.type == "AssetPath" || payload.type == "VFSNode") return Silica::EventReply::handled();
				return Silica::EventReply::unhandled();
			},
			.onDrop = [this, node](const Silica::DragDropPayload& payload) mutable {
				if (payload.type == "AssetPath") {
					std::filesystem::path droppedPath = std::any_cast<std::filesystem::path>(payload.data);
					EditorActionQueue::push([this, node, droppedPath]() mutable {
						if (!std::filesystem::is_directory(droppedPath)) {
							bool exists = false;
							for (const auto& child : node->children) {
								if (!child->isDirectory && child->physicalPath == droppedPath) { exists = true; break; }
							}
							if (!exists) {
								m_vfs.addFile(node, droppedPath);
								m_vfs.save(ProjectManager::getProject()->getProjectPath() / "Config" / "Collections.yaml");
								m_viewingCollection = true;
								m_currentCollection = node;
								refresh();
							}
						}
					});
					return Silica::EventReply::handled();
				}
				else if (payload.type == "VFSNode") {
					auto droppedNode = std::any_cast<std::shared_ptr<VFSNode>>(payload.data);
					EditorActionQueue::push([this, node, droppedNode]() mutable {
						if (droppedNode != node) {
							bool isDescendant = false;
							auto checkParent = node->parent;
							while (checkParent) {
								if (checkParent == droppedNode.get()) { isDescendant = true; break; }
								checkParent = checkParent->parent;
							}
							if (!isDescendant) {
								m_vfs.moveNode(droppedNode, node);
								m_vfs.save(ProjectManager::getProject()->getProjectPath() / "Config" / "Collections.yaml");
								refresh();
							}
						}
					});
					return Silica::EventReply::handled();
				}
				return Silica::EventReply::unhandled();
			},
			.leadingWidget = folderIcon,
			.children = childWidgets
		});

		auto ctxMenuBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 2.0f });

		ctxMenuBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
			.padding = {8, 4}, .color = Silica::Color::transparent(), .hoverColor = Silica::GetTheme().Accent_Primary,
			.onClick = [this, node]() {
				EditorActionQueue::push([this, node]() {
					std::string baseName = "New Collection";
					std::string finalName = baseName;
					int counter = 1;
					auto nameExists = [&](const std::string& n) { for (auto& c : node->children) if (c->name == n) return true; return false; };
					while (nameExists(finalName)) finalName = baseName + " " + std::to_string(counter++);

					m_vfs.createFolder(node, finalName);
					m_vfs.save(ProjectManager::getProject()->getProjectPath() / "Config" / "Collections.yaml");

					m_viewingCollection = true;
					m_currentCollection = node;
					m_itemBeingRenamed = finalName;
					m_itemRenameString = finalName;
					m_startRenaming = true;
					rebuildUI();
				});
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Create Sub-Collection" })
		}) });

		// -- Delete Collection Option --
		if (node != m_vfs.getRoot()) {
			ctxMenuBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
				.padding = {8, 4}, .color = Silica::Color::transparent(), .hoverColor = Silica::GetTheme().Accent_Danger,
				.onClick = [this, node]() {
					EditorActionQueue::push([this, node]() {
						if (node->parent) {
							bool isCurrentDescendant = false;
							VFSNode* current = m_currentCollection.get();
							while (current) {
								if (current == node.get()) { isCurrentDescendant = true; break; }
								current = current->parent;
							}

							auto& siblings = node->parent->children;
							siblings.erase(std::remove(siblings.begin(), siblings.end(), node), siblings.end());
							m_vfs.save(ProjectManager::getProject()->getProjectPath() / "Config" / "Collections.yaml");

							if (isCurrentDescendant) {
								m_currentCollection = m_vfs.getRoot();
								m_viewingCollection = true;
							}
							rebuildUI();
						}
					});
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Delete Collection" })
			}) });
		}

		return Silica::MakeWidget<Silica::SMenuAnchor>({
			.openOnHover = false, .openOnRightClick = true, .openAtMousePos = true,
			.anchorContent = treeNode,
			.menuContent = Silica::MakeWidget<Silica::SBox>({
				.padding = {5, 5}, .borderThickness = Silica::GetTheme().Border_Thickness,
				.backgroundColor = Silica::GetTheme().Background_Popup, .child = ctxMenuBox
			})
		});
	}

}
