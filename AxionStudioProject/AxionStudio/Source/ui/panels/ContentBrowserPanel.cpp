#include "ContentBrowserPanel.h"

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionEngine/Source/scene/SceneManager.h"
#include "AxionEngine/Source/core/Application.h"

#include "AxionStudio/Source/core/EditorResourceManager.h"
#include "AxionStudio/Source/core/EditorActionQueue.h"
#include "AxionStudio/Source/core/SilicaContext.h"
#include "AxionStudio/Source/scripting/VisualScriptGraph.h"
#include "AxionStudio/Source/scripting/VisualScriptSerializer.h"

#include "AxionStudio/Vendor/Silica/include/SBorderLayout.h"
#include "AxionStudio/Vendor/Silica/include/SHorizontalBox.h"
#include "AxionStudio/Vendor/Silica/include/SVerticalBox.h"
#include "AxionStudio/Vendor/Silica/include/SButton.h"
#include "AxionStudio/Vendor/Silica/include/STextBlock.h"
#include "AxionStudio/Vendor/Silica/include/SImage.h"
#include "AxionStudio/Vendor/Silica/include/SMenuAnchor.h"
#include "AxionStudio/Vendor/Silica/include/SScrollBox.h"
#include "AxionStudio/Vendor/Silica/include/SEditableText.h"
#include "AxionStudio/Vendor/Silica/include/SCheckbox.h"
#include "AxionStudio/Vendor/Silica/include/SOverlay.h"
#include "AxionStudio/Vendor/Silica/include/SAlign.h"
#include "AxionStudio/Vendor/Silica/include/SWrapBox.h"
#include "AxionStudio/Vendor/Silica/include/SScrollCatcher.h"

#include <chrono>

namespace Axion {

	// ----- CUSTOM WIDGETS -----
	class SAssetDropZone : public Silica::SWidget {
	public:

		struct Args {
			Silica::FontAtlas* font;
			Silica::WidgetPtr child;
		};

		void construct(const Args& args) {
			m_child = args.child;
			m_font = args.font;
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

		void onDraw(Silica::DrawList& outDrawList, const Silica::Geometry& allocatedGeometry) const override {
			if (m_child) m_child->onDraw(outDrawList, m_child->getAllocatedGeometry());

			// -- Draw Tooltip --
			if (!s_draggedAssetPath.empty() && m_font) {
				std::string tag = s_draggedAssetPath.filename().string();
				Silica::Vec2 mousePos = Silica::Renderer::s_mousePosition;
				mousePos.x += 15.0f;
				mousePos.y += 15.0f;

				float textWidth = 0.0f;
				for (char c : tag) textWidth += m_font->getGlyph(c).advanceX;

				Silica::Geometry bgGeo = { mousePos, {textWidth + 16.0f, 26.0f} };

				uint32_t startIndex = (uint32_t)outDrawList.vertices.size();
				Silica::Color color(40, 40, 40, 230);
				outDrawList.vertices.push_back({ {bgGeo.position.x, bgGeo.position.y}, {0, 0}, color });
				outDrawList.vertices.push_back({ {bgGeo.position.x + bgGeo.size.x, bgGeo.position.y}, {0, 0}, color });
				outDrawList.vertices.push_back({ {bgGeo.position.x + bgGeo.size.x, bgGeo.position.y + bgGeo.size.y}, {0, 0}, color });
				outDrawList.vertices.push_back({ {bgGeo.position.x, bgGeo.position.y + bgGeo.size.y}, {0, 0}, color });
				outDrawList.indices.push_back(startIndex + 0); outDrawList.indices.push_back(startIndex + 1); outDrawList.indices.push_back(startIndex + 2);
				outDrawList.indices.push_back(startIndex + 0); outDrawList.indices.push_back(startIndex + 2); outDrawList.indices.push_back(startIndex + 3);
				if (outDrawList.commands.empty()) outDrawList.commands.push_back({ 0, 0, 0 });
				outDrawList.commands.back().indexCount += 6;

				// -- Draw Text --
				float cursorX = mousePos.x + 8.0f;
				float baselineY = mousePos.y + 18.0f;
				for (char c : tag) {
					const Silica::Glyph& g = m_font->getGlyph(c);
					if (g.size.x > 0 && g.size.y > 0) {
						float x0 = cursorX + g.offset.x; float y0 = baselineY + g.offset.y;
						float x1 = x0 + g.size.x;        float y1 = y0 + g.size.y;
						uint32_t iIdx = (uint32_t)outDrawList.vertices.size();
						outDrawList.vertices.push_back({ {x0, y0}, {g.uvMin.x, g.uvMin.y}, Silica::Color::white() });
						outDrawList.vertices.push_back({ {x1, y0}, {g.uvMax.x, g.uvMin.y}, Silica::Color::white() });
						outDrawList.vertices.push_back({ {x1, y1}, {g.uvMax.x, g.uvMax.y}, Silica::Color::white() });
						outDrawList.vertices.push_back({ {x0, y1}, {g.uvMin.x, g.uvMax.y}, Silica::Color::white() });
						outDrawList.indices.push_back(iIdx + 0); outDrawList.indices.push_back(iIdx + 1); outDrawList.indices.push_back(iIdx + 2);
						outDrawList.indices.push_back(iIdx + 0); outDrawList.indices.push_back(iIdx + 2); outDrawList.indices.push_back(iIdx + 3);
						if (outDrawList.commands.empty()) outDrawList.commands.push_back({ 0, 0, 0 });
						outDrawList.commands.back().indexCount += 6;
					}
					cursorX += g.advanceX;
				}
			}
		}

		Silica::EventReply onMouseWheel(const Silica::Geometry& geom, const Silica::Vec2& pos, float delta) override {
			if (m_child) return m_child->onMouseWheel(m_child->getAllocatedGeometry(), pos, delta);
			return Silica::EventReply::unhandled();
		}

		Silica::EventReply onMouseButtonDown(const Silica::Geometry& geom, const Silica::Vec2& pos, Silica::MouseButton btn) override {
			Silica::EventReply reply = Silica::EventReply::unhandled();
			if (m_child) reply = m_child->onMouseButtonDown(m_child->getAllocatedGeometry(), pos, btn);
			return reply;
		}

		Silica::EventReply onMouseMove(const Silica::Geometry& geom, const Silica::Vec2& pos) override {
			if (!s_draggedAssetPath.empty()) {
				Application::get().getCursor().setCursor(CursorType::Hand);
			}
			if (m_child) return m_child->onMouseMove(m_child->getAllocatedGeometry(), pos);
			return Silica::EventReply::unhandled();
		}

		Silica::EventReply onMouseButtonUp(const Silica::Geometry& geom, const Silica::Vec2& pos, Silica::MouseButton btn) override {
			if (!s_draggedAssetPath.empty()) {
				s_draggedAssetPath = "";
				Application::get().getCursor().setCursor(CursorType::Arrow);
			}
			Silica::EventReply reply = Silica::EventReply::unhandled();
			if (m_child) reply = m_child->onMouseButtonUp(m_child->getAllocatedGeometry(), pos, btn);
			return reply;
		}

	private:

		Silica::FontAtlas* m_font = nullptr;
		Silica::WidgetPtr m_child;

	};

	class SAssetClickBox : public Silica::SWidget {
	public:

		struct Args {
			std::function<void()> onDragStart;
			std::function<Silica::EventReply()> onClick;
			Silica::WidgetPtr child;
		};

		void construct(const Args& args) {
			m_child = args.child;
			m_onDragStart = args.onDragStart;
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
			// -- Draw Background --
			Silica::Color color = Silica::Color::transparent();
			if (m_isMouseDown) color = Silica::Color(70, 130, 200, 150);
			else if (m_isHovered) color = Silica::Color(255, 255, 255, 20);

			if (color.a() > 0) {
				uint32_t startIndex = (uint32_t)dl.vertices.size();
				dl.vertices.push_back({ {geom.position.x, geom.position.y}, {0, 0}, color });
				dl.vertices.push_back({ {geom.position.x + geom.size.x, geom.position.y}, {0, 0}, color });
				dl.vertices.push_back({ {geom.position.x + geom.size.x, geom.position.y + geom.size.y}, {0, 0}, color });
				dl.vertices.push_back({ {geom.position.x, geom.position.y + geom.size.y}, {0, 0}, color });
				dl.indices.push_back(startIndex + 0); dl.indices.push_back(startIndex + 1); dl.indices.push_back(startIndex + 2);
				dl.indices.push_back(startIndex + 0); dl.indices.push_back(startIndex + 2); dl.indices.push_back(startIndex + 3);
				if (dl.commands.empty()) dl.commands.push_back({ 0, 0, 0 });
				dl.commands.back().indexCount += 6;
			}

			if (m_child) m_child->onDraw(dl, m_child->getAllocatedGeometry());
		}

		Silica::EventReply onMouseWheel(const Silica::Geometry& geom, const Silica::Vec2& pos, float delta) override {
			if (m_child) return m_child->onMouseWheel(m_child->getAllocatedGeometry(), pos, delta);
			return Silica::EventReply::unhandled();
		}

		Silica::EventReply onMouseButtonDown(const Silica::Geometry& geom, const Silica::Vec2& pos, Silica::MouseButton btn) override {
			Silica::EventReply reply = Silica::EventReply::unhandled();
			if (m_child) reply = m_child->onMouseButtonDown(m_child->getAllocatedGeometry(), pos, btn);
			if (btn == Silica::MouseButton::Left && geom.contains(pos)) m_isMouseDown = true;
			return reply;
		}

		Silica::EventReply onMouseMove(const Silica::Geometry& geom, const Silica::Vec2& pos) override {
			m_isHovered = geom.contains(pos);
			if (m_isMouseDown && m_onDragStart) m_onDragStart();
			if (m_child) return m_child->onMouseMove(m_child->getAllocatedGeometry(), pos);
			return Silica::EventReply::unhandled();
		}

		Silica::EventReply onMouseButtonUp(const Silica::Geometry& geom, const Silica::Vec2& pos, Silica::MouseButton btn) override {
			m_isMouseDown = false;
			Silica::EventReply reply = Silica::EventReply::unhandled();
			if (m_child) reply = m_child->onMouseButtonUp(m_child->getAllocatedGeometry(), pos, btn);

			if (!reply.isHandled && btn == Silica::MouseButton::Left && geom.contains(pos) && m_onClick) {
				return m_onClick();
			}
			return reply;
		}

	private:

		Silica::WidgetPtr m_child;
		std::function<void()> m_onDragStart;
		std::function<Silica::EventReply()> m_onClick;
		bool m_isMouseDown = false;
		bool m_isHovered = false;

	};





	// ----- CONTENT BROWSER PANEL IMPLEMENTION -----
	void ContentBrowser::setup() {
		if (ProjectManager::hasProject()) {
			m_rootDirectory = ProjectManager::getProject()->getAssetsPath();
			m_currentDirectory = m_rootDirectory;
			refreshDirectory();
		}
	}

	Silica::WidgetPtr ContentBrowser::getWidget(Silica::FontAtlas* font) {
		m_font = font;

		if (!m_uiRoot) {
			m_uiRoot = Silica::MakeWidget<Silica::SBox>({
				.backgroundColor = Silica::Color(30, 30, 30, 255)
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

		if (!ProjectManager::hasProject()) {
			m_uiRoot->setChild(Silica::MakeWidget<Silica::SAlign>({
				.horizontalAlign = Silica::HorizontalAlign::Center,
				.verticalAlign = Silica::VerticalAlign::Center,
				.child = Silica::MakeWidget<Silica::STextBlock>({
					.text = "No Project Loaded.\nPlease load or create a project first.",
					.font = m_font
				})
			}));
			return;
		}

		auto mainLayout = Silica::MakeWidget<Silica::SBorderLayout>({
			.topBar = buildToolbar(),
			.contentArea = buildContentArea()
		});

		Silica::WidgetPtr finalContent = mainLayout;

		// -- Delete Popup Modal --
		if (m_openDeletePopup) {
			finalContent = Silica::MakeWidget<Silica::SOverlay>({
				.children = { finalContent, buildDeleteModal() }
			});
		}

		// -- Assemble --
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
				.color = Silica::Color(45, 45, 45, 0),
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

		auto makeSpacer = []() { return Silica::MakeWidget<Silica::SBox>({ .backgroundColor = Silica::Color::transparent() }); };



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
				.font = m_font,
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
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "X", .font = m_font})
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
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Assets Only", .font = m_font})
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
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Show Ext", .font = m_font})
				})}
			}
		});


		// -- Assemble --
		return Silica::MakeWidget<Silica::SBox>({
			.padding = { 5.0f, 5.0f },
			.backgroundColor = Silica::Color(25, 25, 25, 255),
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
			if (m_onlyEngineAssets && !item.isDir && !isEngineAssetExtension(path)) continue;

			Silica::TextureID iconTex = item.isDir ? SilicaContext::getIcon("FolderIcon") : SilicaContext::getIcon("FileIcon");

			// -- Context Menu Builder --
			auto ctxMenu = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 2.0f });

			if (path.extension() == ".axscene") {
				ctxMenu->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = {8, 4}, .color = Silica::Color::transparent(), .hoverColor = Silica::Color(70, 130, 200, 255),
					.onClick = [path]() { ProjectManager::getProject()->setDefaultScene(path); ProjectManager::saveProject(ProjectManager::getProjectFilePath()); return Silica::EventReply::handled(); },
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Set as Default Scene", .font = m_font})
				}) });
			}

			if (path.extension() == ".axmat") {
				ctxMenu->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = {8, 4}, .color = Silica::Color::transparent(), .hoverColor = Silica::Color(70, 130, 200, 255),
					.onClick = [path]() { AssetManager::reload<Material>(AssetManager::getAssetUUID(path)); return Silica::EventReply::handled(); },
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Reload Material", .font = m_font})
				}) });
			}

			ctxMenu->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
				.padding = {8, 4}, .color = Silica::Color::transparent(), .hoverColor = Silica::Color(70, 130, 200, 255),
				.onClick = [path]() { PlatformUtils::showInFileExplorer(path); return Silica::EventReply::handled(); },
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Show in Explorer", .font = m_font})
			}) });

			ctxMenu->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
				.padding = {8, 4}, .color = Silica::Color::transparent(), .hoverColor = Silica::Color(70, 130, 200, 255),
				.onClick = [this, path, item]() {
					m_itemBeingRenamed = path; m_itemRenameString = path.filename().string(); m_startRenaming = true; rebuildUI(); return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Rename", .font = m_font})
			}) });

			ctxMenu->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
				.padding = {8, 4}, .color = Silica::Color::transparent(), .hoverColor = Silica::Color(200, 50, 50, 255),
				.onClick = [this, path]() {
					m_pendingDelete = path; m_relatedFilesToDelete = findRelatedFiles(path); m_deleteRelatedFiles = true; m_openDeletePopup = true; rebuildUI(); return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Delete", .font = m_font})
			}) });


			// -- Item Label Or Rename Box --
			Silica::WidgetPtr labelWidget = nullptr;
			if (m_itemBeingRenamed == path) {
				labelWidget = Silica::MakeWidget<Silica::SBox>({
					.explicitSize = Silica::Vec2(m_thumbnailSize + 30.0f, 0.0f),
					.child = Silica::MakeWidget<Silica::SEditableText>({
						.initialText = m_itemRenameString,
						.font = m_font,
						.onTextCommitted = [this, path, item](const std::string& newText) {
							std::filesystem::path newPath = path.parent_path() / newText;
							if (!item.isDir && path.has_extension()) {
								std::string ext = path.extension().string();
								if (newPath.extension().string() != ext) newPath += ext;
							}
							std::error_code ec;
							if (!std::filesystem::exists(newPath, ec)) {
								std::filesystem::rename(path, newPath, ec);
							}
							resetRenaming(); refresh();
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
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = displayName, .font = m_font})
					})
				});
			}


			// -- Clickable Folder / File --
			auto assetClickBox = Silica::MakeWidget<SAssetClickBox>({
				.onDragStart = [path]() {
					s_draggedAssetPath = path;
				},
				.onClick = [this, path, item]() {
					auto now = std::chrono::steady_clock::now();
					bool isDouble = (path == lastClickedPath) && (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastClickTime).count() < 300);
					lastClickTime = now; lastClickedPath = path;

					if (isDouble) {
						if (item.isDir) {
							m_backHistory.push_back(m_currentDirectory);
							m_forwardHistory.clear();
							m_currentDirectory = path;
							refresh();
						}
						else if (path.extension() == ".axscene") {
							SceneManager::loadScene(path);
						}
						else if (path.extension() == ".axvs" && m_openVisualScriptPanel) {
							m_openVisualScriptPanel(path);
						}
					}
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::SVerticalBox>({
					.spacing = 4.0f,
					.slots = {
						{ {0,0}, Silica::MakeWidget<Silica::SAlign>({
							.horizontalAlign = Silica::HorizontalAlign::Center,
							.child = Silica::MakeWidget<Silica::SImage>({.textureID = iconTex, .tint = Silica::Color::white(), .desiredSize = {m_thumbnailSize, m_thumbnailSize} })
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
				.menuContent = Silica::MakeWidget<Silica::SBox>({.padding = {5,5}, .backgroundColor = Silica::Color(45, 45, 45, 255), .child = ctxMenu })
			});

			grid->addChild(itemBox);
		}

		// -- Background Area Context Menu --
		auto bgMenuContent = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 2.0f });

		bgMenuContent->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
			.padding = {8, 4}, .color = Silica::Color::transparent(), .hoverColor = Silica::Color(70, 130, 200, 255),
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
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Create Visual Script", .font = m_font})
		}) });


		// -- Audio Import --
		bgMenuContent->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
			.padding = {8, 4}, .color = Silica::Color::transparent(), .hoverColor = Silica::Color(70, 130, 200, 255),
			.onClick = [this]() {
				std::filesystem::path audioDir = ProjectManager::getProject()->getAssetsPath() / "audio";
				std::filesystem::path absPath = std::filesystem::exists(audioDir) ?
					FileDialogs::openFile({ {"Audio Files", "*.mp3;*.wav;*.ogg"} }, audioDir) :
					FileDialogs::openFile({ {"Audio Files", "*.mp3;*.wav;*.ogg"} }, ProjectManager::getProject()->getAssetsPath());

				if (!absPath.empty() && m_openGlobalModal) {
					m_audioImportModal = std::make_shared<AudioImportModal>();
					m_audioImportModal->presetFromFile(absPath);

					auto modalWidget = m_audioImportModal->getWidget(m_font, [this]() {
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
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Import Audio", .font = m_font})
		}) });


		// -- Material Import --
		bgMenuContent->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
			.padding = {8, 4}, .color = Silica::Color::transparent(), .hoverColor = Silica::Color(70, 130, 200, 255),
			.onClick = [this]() {
				if (m_openGlobalModal) {
					m_materialImportModal = std::make_shared<MaterialImportModal>();

					auto modalWidget = m_materialImportModal->getWidget(m_font, [this]() {
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
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Create Material", .font = m_font})
		}) });


		// -- Mesh Import --
		bgMenuContent->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
			.padding = {8, 4}, .color = Silica::Color::transparent(), .hoverColor = Silica::Color(70, 130, 200, 255),
			.onClick = [this]() {
				std::filesystem::path meshDir = ProjectManager::getProject()->getAssetsPath() / "meshes";
				std::filesystem::path absPath = std::filesystem::exists(meshDir) ?
					FileDialogs::openFile({ {"3D Models", "*.obj;*.gltf;*.glb"} }, meshDir) :
					FileDialogs::openFile({ {"3D Models", "*.obj;*.gltf;*.glb"} }, ProjectManager::getProject()->getAssetsPath());

				if (!absPath.empty() && m_openGlobalModal) {
					m_meshImportModal = std::make_shared<MeshImportModal>();
					m_meshImportModal->presetFromFile(absPath);

					auto modalWidget = m_meshImportModal->getWidget(m_font, [this]() {
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
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Import Mesh", .font = m_font})
		}) });


		// -- Create Physics Material --
		bgMenuContent->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
			.padding = {8, 4}, .color = Silica::Color::transparent(), .hoverColor = Silica::Color(70, 130, 200, 255),
			.onClick = [this]() {
				if (m_openGlobalModal) {
					m_physicsMaterialModal = std::make_shared<PhysicsMaterialImportModal>();

					auto modalWidget = m_physicsMaterialModal->getWidget(m_font, [this]() {
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
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Create Physics Material", .font = m_font})
		}) });


		// -- Create Pipeline --
		bgMenuContent->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
			.padding = {8, 4}, .color = Silica::Color::transparent(), .hoverColor = Silica::Color(70, 130, 200, 255),
			.onClick = [this]() {
				if (m_openGlobalModal) {
					m_pipelineImportModal = std::make_shared<PipelineImportModal>();

					auto modalWidget = m_pipelineImportModal->getWidget(m_font, [this]() {
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
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Create Pipeline", .font = m_font})
		}) });


		// -- Shader Import --
		bgMenuContent->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
			.padding = {8, 4}, .color = Silica::Color::transparent(), .hoverColor = Silica::Color(70, 130, 200, 255),
			.onClick = [this]() {
				if (m_openGlobalModal) {
					m_shaderImportModal = std::make_shared<ShaderImportModal>();

					auto modalWidget = m_shaderImportModal->getWidget(m_font, [this]() {
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
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Import Shader", .font = m_font})
		}) });


		// -- Create Skybox --
		bgMenuContent->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
			.padding = {8, 4}, .color = Silica::Color::transparent(), .hoverColor = Silica::Color(70, 130, 200, 255),
			.onClick = [this]() {
				if (m_openGlobalModal) {
					m_skyboxImportModal = std::make_shared<SkyboxImportModal>();

					auto modalWidget = m_skyboxImportModal->getWidget(m_font, [this]() {
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
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Create Skybox", .font = m_font})
		}) });


		// -- Texture2D Import --
		bgMenuContent->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
			.padding = {8, 4}, .color = Silica::Color::transparent(), .hoverColor = Silica::Color(70, 130, 200, 255),
			.onClick = [this]() {
				if (m_openGlobalModal) {
					m_texture2DImportModal = std::make_shared<Texture2DImportModal>();

					auto modalWidget = m_texture2DImportModal->getWidget(m_font, [this]() {
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
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Import Texture 2D", .font = m_font})
		}) });


		// -- Texture Cube Import --
		bgMenuContent->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
			.padding = {8, 4}, .color = Silica::Color::transparent(), .hoverColor = Silica::Color(70, 130, 200, 255),
			.onClick = [this]() {
				if (m_openGlobalModal) {
					m_textureCubeImportModal = std::make_shared<TextureCubeImportModal>();

					auto modalWidget = m_textureCubeImportModal->getWidget(m_font, [this]() {
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
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Import Texture Cube", .font = m_font})
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
				.padding = {5,5}, .backgroundColor = Silica::Color(45, 45, 45, 255),
				.child = bgMenuContent
			})
		});

		auto dropZone = Silica::MakeWidget<SAssetDropZone>({
			.font = m_font,
			.child = backgroundMenu,
		});

		auto zoomBox = Silica::MakeWidget<Silica::SScrollCatcher>({
			.onMouseWheel = [this](float delta) {
				m_thumbnailSize += delta * 5.0f;
				m_thumbnailSize = std::clamp(m_thumbnailSize, 32.0f, 256.0f);
				m_showNames = m_thumbnailSize >= 50.0f;
				rebuildUI();
				return Silica::EventReply::handled();
			},
			.child = dropZone,
		});

		return zoomBox;
	}

	Silica::WidgetPtr ContentBrowser::buildDeleteModal() {
		auto overlayBg = Silica::MakeWidget<Silica::SBox>({
			.backgroundColor = Silica::Color(0, 0, 0, 150)
		});

		auto modalContent = Silica::MakeWidget<Silica::SVerticalBox>({
			.spacing = 10.0f,
			.slots = {
				{ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "Are you sure you want to delete\n" + m_pendingDelete->filename().string() + "?", .font = m_font}) }
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
					{ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "Delete related files", .font = m_font})}
				}
			}) });
		}

		auto buttons = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 10.0f,
			.slots = {
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 20.0f, 8.0f }, .color = Silica::Color(150, 50, 50, 255),
					.onClick = [this]() {
						if (m_onAssetDeleted) m_onAssetDeleted(*m_pendingDelete);
						if (isEngineAssetExtension(*m_pendingDelete)) {
							UUID assetUUID = AssetManager::getAssetUUID(*m_pendingDelete);
							if (assetUUID.isValid()) { AssetManager::removeAsset(assetUUID); ProjectManager::saveProject(ProjectManager::getProjectFilePath()); }
						}
						deletePath(*m_pendingDelete);
						if (m_deleteRelatedFiles) { for (const auto& rel : m_relatedFilesToDelete) deletePath(rel); }
						m_pendingDelete.reset(); m_relatedFilesToDelete.clear(); m_openDeletePopup = false; refresh();
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Delete", .font = m_font})
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 20.0f, 8.0f }, .color = Silica::Color(80, 80, 80, 255),
					.onClick = [this]() {
						m_pendingDelete.reset(); m_openDeletePopup = false; rebuildUI();
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Cancel", .font = m_font})
				})}
			}
		});

		modalContent->addSlot({ {0,0}, buttons });

		auto modalBox = Silica::MakeWidget<Silica::SBox>({
			.padding = { 20.0f, 20.0f },
			.backgroundColor = Silica::Color(40, 40, 40, 255),
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
		std::error_code ec;
		std::filesystem::directory_iterator it(m_currentDirectory, std::filesystem::directory_options::skip_permission_denied, ec);
		if (ec) { m_directoryEntries.clear(); return; }

		for (auto end = std::filesystem::directory_iterator(); it != end; it.increment(ec)) {
			if (ec) { ec.clear(); continue; }
			const auto p = it->path();
			DirItem di;
			di.path = p;
			di.isDir = it->is_directory(ec);
			auto rel = p.lexically_relative(m_rootDirectory);
			di.displayName = rel.empty() ? p.filename().string() : rel.filename().string();
			tmp.push_back(std::move(di));
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

	bool ContentBrowser::isEngineAssetExtension(const std::filesystem::path& path) {
		auto ext = path.extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
		return ext == ".axmesh" || ext == ".axsky" || ext == ".axshader" || ext == ".axmat" || ext == ".axaudio" || ext == ".axtex" || ext == ".axpmat" || ext == ".axprefab" || ext == ".axpso" || ext == ".axscene" || ext == ".axtcube" || ext == ".axanim" || ext == ".axskelmesh" || ext == ".axvs";
	}

	void ContentBrowser::deletePath(const std::filesystem::path& path) {
		std::error_code ec;
		if (std::filesystem::is_directory(path, ec) && !ec) std::filesystem::remove_all(path, ec);
		else std::filesystem::remove(path, ec);
	}

	bool ContentBrowser::onProjectChanged(ProjectChangedEvent& e) {
		if (ProjectManager::hasProject()) {
			std::error_code ec;
			m_rootDirectory = std::filesystem::weakly_canonical(ProjectManager::getProject()->getAssetsPath(), ec);
			if (ec) m_rootDirectory = std::filesystem::absolute(ProjectManager::getProject()->getAssetsPath(), ec);
			m_currentDirectory = m_rootDirectory;
			refresh();
		}
		else {
			m_directoryEntries.clear(); m_rootDirectory.clear(); m_currentDirectory.clear();
		}
		m_backHistory.clear(); m_forwardHistory.clear();
		return false;
	}

	void ContentBrowser::refresh() {
		refreshDirectory();
		rebuildUI();
	}

	std::vector<std::filesystem::path> ContentBrowser::findRelatedFiles(const std::filesystem::path& path) {
		std::vector<std::filesystem::path> related;
		if (path.extension() == ".axvs") {
			std::filesystem::path layoutPath = path.parent_path() / (path.stem().string() + "_layout.json");
			if (std::filesystem::exists(layoutPath)) related.push_back(layoutPath);
		}
		return related;
	}
}
