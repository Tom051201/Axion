#include "studiopch.h"
#include "EditorLayer.h"

#include <Quartz/include/SQuartzEditor.h>
#include <Silica/include/Theme.h>
#include <Silica/include/SBox.h>
#include <Silica/include/SOverlay.h>
#include <Silica/include/SAlign.h>
#include <Silica/include/SWorkspace.h>
#include <Silica/include/SBorderLayout.h>
#include <Silica/include/SLoadingToast.h>
#include <Silica/include/SVerticalBox.h>

#include "AxionEngine/Source/core/Application.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/graphics/Renderer2D.h"
#include "AxionEngine/Source/scene/Prefab.h"
#include "AxionEngine/Source/scene/SceneManager.h"
#include "AxionEngine/Source/scene/SceneSerializer.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionEngine/Source/input/Input.h"
#include "AxionEngine/Source/scripting/ScriptEngine.h"

#include "AxionAssetPipeline/Source/core/AssetMigrator.h"

#include "AxionNetwork/Source/NetworkManager.h"

#include "AxionStudio/Source/core/EditorResourceManager.h"
#include "AxionStudio/Source/core/EditorCommand.h"
#include "AxionStudio/Source/core/EditorActionQueue.h"
#include "AxionStudio/Source/core/EditorModalManager.h"
#include "AxionStudio/Source/core/SilicaContext.h"
#include "AxionStudio/Source/core/WireframeRenderer.h"
#include "AxionStudio/Source/core/EditorUtils.h"
#include "AxionStudio/Source/core/EditorSettings.h"
#include "AxionStudio/Source/core/DiscordManager.h"
#include "AxionStudio/Source/ui/EditorMenuBar.h"
#include "AxionStudio/Source/ui/panels/ViewportPanel.h"
#include "AxionStudio/Source/ui/panels/ContentBrowserPanel.h"
#include "AxionStudio/Source/ui/panels/VisualScriptPanel.h"
#include "AxionStudio/Source/ui/panels/SceneSettingsPanel.h"
#include "AxionStudio/Source/ui/panels/ProjectSettingsPanel.h"
#include "AxionStudio/Source/ui/panels/AssetManagerPanel.h"
#include "AxionStudio/Source/ui/panels/HierarchyPanel.h"
#include "AxionStudio/Source/ui/panels/EntityPropertiesPanel.h"
#include "AxionStudio/Source/ui/panels/AssetLibraryPanel.h"
#include "AxionStudio/Source/ui/panels/MaterialPanel.h"
#include "AxionStudio/Source/ui/panels/HistoryPanel.h"
#include "AxionStudio/Source/ui/panels/NetworkPanel.h"
#include "AxionStudio/Source/ui/modals/SettingsModal.h"
#include "AxionStudio/Source/ui/modals/CreateProjectModal.h"
#include "AxionStudio/Source/ui/modals/ExportProjectModal.h"
#include "AxionStudio/Source/ui/modals/SystemInfoModal.h"

namespace Axion {

	EditorLayer::EditorLayer() : Layer("AxionEditorLayer"), m_editorCamera(1280, 720) {}

	void EditorLayer::onAttach() {

		//PlatformUtils::registerProjectFileExtension();

		// ----- Initialize Axion Network -----
		AXNetwork::NetworkManager::initialize();

		ScriptEngine::s_networkSendEventCallback = [this](UUID id, uint32_t eventID, uint8_t* payload, uint16_t size) {
			m_testClient.sendNetworkEvent(id, eventID, payload, size);
		};

		// ----- Load Editor Resources -----
		EditorResourceManager::initialize();
		EditorResourceManager::loadIcon("AddFolderIcon", "AxionStudio/Resources/Editor/UI/AddFolderIcon.png");
		EditorResourceManager::loadIcon("BackIcon", "AxionStudio/Resources/Editor/UI/BackIcon.png");
		EditorResourceManager::loadIcon("FileIcon", "AxionStudio/Resources/Editor/UI/FileIcon.png");
		EditorResourceManager::loadIcon("FolderIcon", "AxionStudio/Resources/Editor/UI/FolderIcon.png");
		EditorResourceManager::loadIcon("ForwardIcon", "AxionStudio/Resources/Editor/UI/ForwardIcon.png");
		EditorResourceManager::loadIcon("RefreshIcon", "AxionStudio/Resources/Editor/UI/RefreshIcon.png");
		EditorResourceManager::loadIcon("2DCamIcon", "AxionStudio/Resources/Editor/UI/2dIcon.png");
		EditorResourceManager::loadIcon("3DCamIcon", "AxionStudio/Resources/Editor/UI/3dIcon.png");
		EditorResourceManager::loadIcon("CameraIcon", "AxionStudio/Resources/Editor/UI/CameraIcon.png");
		EditorResourceManager::loadIcon("LightIcon", "AxionStudio/Resources/Editor/UI/LightIcon.png");
		EditorResourceManager::loadIcon("PauseButton", "AxionStudio/Resources/Editor/UI/PauseIcon.png");
		EditorResourceManager::loadIcon("PlayButton", "AxionStudio/Resources/Editor/UI/PlayIcon.png");
		EditorResourceManager::loadIcon("SimulateButton", "AxionStudio/Resources/Editor/UI/SimulateIcon.png");
		EditorResourceManager::loadIcon("StepButton", "AxionStudio/Resources/Editor/UI/StepIcon.png");
		EditorResourceManager::loadIcon("StopButton", "AxionStudio/Resources/Editor/UI/StopIcon.png");
		EditorResourceManager::loadIcon("GearIcon", "AxionStudio/Resources/Editor/UI/GearIcon.png");

		// ----- Setup Framebuffer -----
		FrameBufferSpecification fbs;
		fbs.width = 1280;
		fbs.height = 720;
		fbs.textureFormat = ColorFormat::RGBA8;
		fbs.depthStencilFormat = DepthStencilFormat::DEPTH32F;
		fbs.useEntityIDAttachment = true;
		m_frameBuffer = FrameBuffer::create(fbs);
		m_viewportSize = { (float)fbs.width, (float)fbs.height };

		// ----- Init Silica Backend -----
		SilicaContext::initialize();

		// ----- Load Font -----
		if (m_font.loadFromFile("AxionStudio/Resources/Editor/Fonts/openSans/OpenSans-Bold.ttf", 18.0f)) {
			SilicaContext::uploadFontAtlas(m_font);
			Silica::GetTheme().Font_Default = &m_font;
		}
		else {
			AX_CORE_LOG_WARN("Silica: Failed to load OpenSans font!");
		}

		EditorCommandManager::setHistoryChangedCallback([this]() {
			EditorHistoryChangedEvent ev;
			onEvent(ev);
		});


		// ----- Build UI -----
		m_hierarchyPanel = MakeShared<HierarchyPanel>();
		m_hierarchyPanel->setEventCallback(AX_BIND_EVENT_FN(EditorLayer::onEvent));
		m_hierarchyPanel->setScene(m_activeScene);
		auto hierarchyWidget = m_hierarchyPanel->getWidget();

		m_propertiesPanel = MakeShared<EntityPropertiesPanel>();
		auto propertiesWidget = m_propertiesPanel->getWidget();

		m_contentBrowserPanel = MakeShared<ContentBrowser>();
		m_contentBrowserPanel->setup();
		m_contentBrowserPanel->setEventCallback(AX_BIND_EVENT_FN(EditorLayer::onEvent));
		m_contentBrowserPanel->setOpenVisualScriptPanelCallback(AX_BIND_FN(EditorLayer::openVisualScriptPanel));
		m_contentBrowserPanel->setOpenTextEditorPanelCallback(AX_BIND_FN(EditorLayer::openTextEditorTab));
		m_contentBrowserPanel->setOpenMaterialEditorPanelCallback(AX_BIND_FN(EditorLayer::openMaterialEditor));
		m_contentBrowserPanel->setOpenSceneInViewportCallback(AX_BIND_FN(EditorLayer::openSceneInViewport));
		auto contentBrowserWidget = m_contentBrowserPanel->getWidget();

		m_projectSettingsPanel = MakeShared<ProjectSettingsPanel>();
		m_projectSettingsPanel->setProject(ProjectManager::getProject());
		auto projectSettingsWidget = m_projectSettingsPanel->getWidget();

		m_sceneSettingsPanel = MakeShared<SceneSettingsPanel>();
		m_sceneSettingsPanel->setEventCallback(AX_BIND_EVENT_FN(EditorLayer::onEvent));
		m_sceneSettingsPanel->setScene(m_activeScene);
		auto sceneSettingsWidget = m_sceneSettingsPanel->getWidget();

		m_viewportTextureID = SilicaContext::getFrameBufferTextureID(m_frameBuffer);
		m_viewportPanel = MakeShared<ViewportPanel>();
		m_viewportPanel->setup(&m_sceneState, &m_prePauseState, &m_stepFrames, &m_editorCamera, &m_transformGizmo);
		m_viewportPanel->setEventCallback(AX_BIND_EVENT_FN(EditorLayer::onEvent));
		m_viewportPanel->setPrefabDropCallback([this](const std::filesystem::path& path, Silica::Vec2 localMouse) {
			if (!m_activeScene) return;

			UUID assetUUID = AssetManager::getAssetUUID(path);
			if (assetUUID.isValid()) {
				Ref<Prefab> prefab = AssetManager::get(AssetManager::load<Prefab>(assetUUID));
				if (prefab) {
					SceneSerializer serializer(m_activeScene);
					Entity spawnedEntity;

					if (!prefab->isBinary()) {
						YAML::Node entityNode = prefab->getEntityNode();
						spawnedEntity = serializer.deserializeEntityNode(m_activeScene.get(), entityNode, true);
					}
					else {
						std::string dataStr(prefab->getBinaryData().begin(), prefab->getBinaryData().end());
						std::istringstream in(dataStr, std::ios::binary);
						std::vector<std::pair<Entity, UUID>> relationshipsToBuild;
						spawnedEntity = serializer.deserializeEntityBinary(m_activeScene.get(), in, true, relationshipsToBuild, 2);
					}

					if (spawnedEntity) {
						if (spawnedEntity.hasComponent<TransformComponent>()) {
							float ndcX = (localMouse.x / m_viewportSize.x) * 2.0f - 1.0f;
							float ndcY = 1.0f - (localMouse.y / m_viewportSize.y) * 2.0f;
							Mat4 invVP = m_editorCamera.getViewProjectionMatrix().inverse();

							Vec4 rayStart = invVP * Vec4(ndcX, ndcY, 0.0f, 1.0f);
							Vec4 rayEnd = invVP * Vec4(ndcX, ndcY, 1.0f, 1.0f);

							Vec3 rayOrigin = Vec3(rayStart.x, rayStart.y, rayStart.z) / rayStart.w;
							Vec3 rayTarget = Vec3(rayEnd.x, rayEnd.y, rayEnd.z) / rayEnd.w;
							Vec3 rayDir = (rayTarget - rayOrigin).normalized();

							Vec3 spawnPos;
							if (m_editorCamera.is2D()) {
								float t = -rayOrigin.z / rayDir.z;
								spawnPos = rayOrigin + (rayDir * t);
							}
							else {
								if (std::abs(rayDir.y) > 0.001f) {
									float t = -rayOrigin.y / rayDir.y;
									if (t <= 0.1f) spawnPos = rayOrigin + (rayDir * 10.0f);
									else spawnPos = rayOrigin + (rayDir * t);
								}
								else {
									spawnPos = rayOrigin + (rayDir * 10.0f);
								}
							}
							spawnedEntity.getComponent<TransformComponent>().position = spawnPos;
						}

						AX_CORE_LOG_INFO("Successfully spawned Prefab: {0}", path.filename().string());
						EntitySelectedEvent e(spawnedEntity);
						onEvent(e);
						if (m_hierarchyPanel) m_hierarchyPanel->rebuildUI();
					}
				}
			}
		});
		m_viewportPanel->setVisualScriptDropCallback(AX_BIND_FN(EditorLayer::openVisualScriptPanel));
		m_viewportPanel->setRequestViewportTextureCallback([this]() -> Silica::TextureID { return m_viewportTextureID; });
		auto fullViewportPanel = m_viewportPanel->getWidget();

		m_visualScriptPanel = MakeShared<VisualScriptPanel>();
		auto visualScriptWidget = m_visualScriptPanel->getWidget();

		m_assetManagerPanel = MakeShared<AssetManagerPanel>();
		auto assetManagerWidget = m_assetManagerPanel->getWidget();

		m_assetLibraryPanel = MakeShared<AssetLibraryPanel>();
		auto assetLibraryWidget = m_assetLibraryPanel->getWidget();

		m_materialPanel = MakeShared<MaterialPanel>();
		m_materialPanel->setEventCallback(AX_BIND_EVENT_FN(EditorLayer::onEvent));
		auto materialWidget = m_materialPanel->getWidget();

		m_historyPanel = MakeShared<HistoryPanel>();
		auto historyWidget = m_historyPanel->getWidget();

		m_networkPanel = MakeShared<NetworkPanel>(&m_testServer, &m_testClient);
		auto networkWidget = m_networkPanel->getWidget();

		// ----- Setup Workspace And DockSpace -----
		auto workspace = Silica::MakeWidget<Silica::SWorkspace>({
			.initialTitle = "Hierarchy",
			.initialContent = hierarchyWidget
		});

		m_dock = workspace->getDockSpace();

		m_dock->registerTab("Hierarchy", hierarchyWidget);
		m_dock->registerTab("Properties", propertiesWidget);
		m_dock->registerTab("Viewport", fullViewportPanel);
		m_dock->registerTab("Visual Script", visualScriptWidget);
		m_dock->registerTab("Content Browser", contentBrowserWidget);
		m_dock->registerTab("Project Settings", projectSettingsWidget);
		m_dock->registerTab("Scene Settings", sceneSettingsWidget);
		m_dock->registerTab("Asset Inspector", assetManagerWidget);
		m_dock->registerTab("Asset Library", assetLibraryWidget);
		m_dock->registerTab("Material Editor", materialWidget);
		m_dock->registerTab("Editor History", historyWidget);
		m_dock->registerTab("Network Test", networkWidget);

		if (!m_dock->getRootNode() || (m_dock->getRootNode()->tabs.size() <= 1 && m_dock->getRootNode()->splitDirection == Silica::SplitDirection::None)) {
			auto root = m_dock->getRootNode();
			m_dock->splitNode(root, Silica::SplitDirection::Vertical, 0.75f, "Content Browser", contentBrowserWidget, false);

			root->child[1]->tabs.push_back({
				.title = "Asset Library",
				.content = assetLibraryWidget,
				.hitRect = {}
			});

			auto topHalf = root->child[0];
			m_dock->splitNode(topHalf, Silica::SplitDirection::Horizontal, 0.2f, "Viewport", fullViewportPanel, false);
			auto viewportNode = topHalf->child[1];
			viewportNode->tabs.push_back({
				.title = "Visual Script",
				.content = visualScriptWidget,
				.hitRect = {}
			});
			m_dock->splitNode(topHalf->child[0], Silica::SplitDirection::Vertical, 0.5f, "Properties", propertiesWidget, false);
			m_dock->splitNode(viewportNode, Silica::SplitDirection::Horizontal, 0.75f, "Project Settings", projectSettingsWidget, false);
			viewportNode->child[1]->tabs.push_back({
				.title = "Scene Settings",
				.content = sceneSettingsWidget,
				.hitRect = {}
			});
			topHalf->child[0]->child[1]->tabs.push_back({
				.title = "Asset Inspector",
				.content = assetManagerWidget,
				.hitRect = {}
			});
		}

		// ----- Menu Bar -----
		EditorMenuBar::MenuBarCallbacks menuCallbacks;
		menuCallbacks.newScene = AX_BIND_FN(EditorLayer::newScene);
		menuCallbacks.openScene = AX_BIND_FN(EditorLayer::openScene);
		menuCallbacks.saveScene = AX_BIND_FN(EditorLayer::saveScene);
		menuCallbacks.saveSceneAs = AX_BIND_FN(EditorLayer::saveSceneAs);
		menuCallbacks.exitEditor = []() { /* Application::get().close(); */ };
		menuCallbacks.openPreferences = AX_BIND_FN(EditorLayer::openPreferences);
		menuCallbacks.openCreateProjectModal = AX_BIND_FN(openCreateProjectModal);
		menuCallbacks.openExportProjectModal = AX_BIND_FN(openExportProjectModal);
		menuCallbacks.openSystemInfoModal = AX_BIND_FN(openSystemInfoModal);
		auto menuBar = EditorMenuBar::construct(m_dock, menuCallbacks);

		// ----- Assemble UI Root -----
		m_mainLayout = Silica::MakeWidget<Silica::SBorderLayout>({
			.topBar = menuBar,
			.contentArea = workspace
		});

		auto compilationToast = Silica::MakeWidget<Silica::SLoadingToast>({
			.text = "Compiling C# Scripts...",
			.isVisible = []() { return ProjectManager::isCompilingScripts(); }
		});

		auto sceneLoadingToast = Silica::MakeWidget<Silica::SLoadingToast>({
			.text = "Loading Scene & Assets...",
			.isVisible = []() { return SceneManager::isLoadingScene() || AssetManager::isLoadingAssets(); }
		});

		auto toastContainer = Silica::MakeWidget<Silica::SVerticalBox>({
			.spacing = 10.0f,
			.slots = {
				{ {0,0}, sceneLoadingToast },
				{ {0,0}, compilationToast }
			}
		});

		auto toastOverlayContainer = Silica::MakeWidget<Silica::SBox>({
			.padding = { 30.0f, 30.0f },
			.backgroundColor = Silica::Color::transparent(),
			.child = Silica::MakeWidget<Silica::SAlign>({
				.horizontalAlign = Silica::HorizontalAlign::Right,
				.verticalAlign = Silica::VerticalAlign::Bottom,
				.child = toastContainer
			})
		});

		m_silicaRoot = Silica::MakeWidget<Silica::SBox>({
			.backgroundColor = Silica::Color::transparent(),
			.child = Silica::MakeWidget<Silica::SOverlay>({
				.children = {
					m_mainLayout,
					toastOverlayContainer
				}
			})
		});

		SilicaContext::bindWndProcCallback(m_silicaRoot);



		// ----- Load Editor Settings -----
		EditorSettings::load(m_editorSettingsPath, [this](const YAML::Node& settings) {
			if (m_contentBrowserPanel) {
				m_contentBrowserPanel->loadSettings(settings);
				m_materialPanel->loadSettings(settings);
				m_viewportPanel->loadSettings(settings);
				m_assetLibraryPanel->loadSettings(settings);
			}
		});

		// ----- Setup Startup Project -----
		if (!EditorSettings::startupProjectPath.empty() && EditorSettings::startupProjectPath != "None") {
			if (std::filesystem::exists(EditorSettings::startupProjectPath)) {
				ProjectManager::loadProject(EditorSettings::startupProjectPath);
				m_activeScene = SceneManager::getScene();
			}
			else {
				SceneManager::newScene();
				m_activeScene = SceneManager::getScene();
				AX_CORE_LOG_WARN("Startup Project From Settings File Does Not Exist!");
			}
		}
		else {
			SceneManager::newScene();
			m_activeScene = SceneManager::getScene();
			AX_CORE_LOG_WARN("No Startup Project Set In Settings File!");
		}

		if (m_hierarchyPanel) m_hierarchyPanel->setScene(m_activeScene);
		if (m_sceneSettingsPanel) m_sceneSettingsPanel->setScene(m_activeScene);
		if (m_projectSettingsPanel) m_projectSettingsPanel->setProject(ProjectManager::getProject());

		// -- Restore Asset Library Paths --
		if (!EditorSettings::assetLibraryPaths.empty() && m_assetLibraryPanel) {
			std::vector<std::filesystem::path> savedPaths;
			for (const auto& pathStr : EditorSettings::assetLibraryPaths) {
				savedPaths.push_back(pathStr);
			}
			m_assetLibraryPanel->setLibraryDirectories(savedPaths);
		}

		// -- Restore Open Text Editors --
		for (const auto& pathStr : EditorSettings::openTextEditors) {
			if (std::filesystem::exists(pathStr)) {
				openTextEditorTab(pathStr);
			}
		}

		// ----- Initialize Discord -----
		if (EditorSettings::enableDiscordRPC) {
			DiscordManager::initialize("1543660033210851368");

			if (ProjectManager::hasProject()) {
				std::string projName = ProjectManager::getProject()->getName();
				std::string sceneName = m_activeScene ? m_activeScene->getTitle() : "Untitled";
				DiscordManager::setPresence("Editing: " + sceneName, "Project: " + projName);
			}
			else {
				DiscordManager::setPresence("In Hub / No Project", "Idle");
			}
		}

		m_dock->loadLayout("AxionStudio/Config/EditorLayout.ini");

		// -- Command Line Arguments --
		auto commandLineArgs = Application::get().getCommandLineArgs();
		if (commandLineArgs.count > 1) {
			std::filesystem::path projectPath = commandLineArgs[1];
			if (projectPath.extension() == ".axproj") {
				AX_CORE_LOG_INFO("Loading project from command line: {0}", projectPath.string());
				ProjectManager::loadProject(projectPath);
			}
		}

		EditorSettingsChangedEvent initSyncEvent(EditorSettingType::All);
		onEvent(initSyncEvent);
	}

	void EditorLayer::onDetach() {
		// -- Save open Text Editors --
		EditorSettings::openTextEditors.clear();
		for (const auto& [pathStr, tabName] : m_openTextEditors) {
			if (m_dock->isTabOpen(tabName)) {
				EditorSettings::openTextEditors.push_back(pathStr);
			}
		}

		// -- Save Asset Library paths --
		EditorSettings::assetLibraryPaths.clear();
		if (m_assetLibraryPanel) {
			for (const auto& path : m_assetLibraryPanel->getLibraryDirectories()) {
				EditorSettings::assetLibraryPaths.push_back(path.string());
			}
		}

		// -- Save all settings --
		EditorSettings::save(m_editorSettingsPath, [this](YAML::Emitter& out) {
			if (m_contentBrowserPanel) {
				m_contentBrowserPanel->saveSettings(out);
				m_materialPanel->saveSettings(out);
				m_viewportPanel->saveSettings(out);
				m_assetLibraryPanel->saveSettings(out);
			}
		});

		m_selectedEntity = {};
		if (m_propertiesPanel) m_propertiesPanel->setEntity({});
		if (m_dock) m_dock->saveLayout("AxionStudio/Config/EditorLayout.ini");

		SilicaContext::unbindWndProcCallback();

		m_silicaRoot = nullptr;
		m_dock = nullptr;
		m_hierarchyPanel = nullptr;
		m_propertiesPanel = nullptr;
		m_contentBrowserPanel = nullptr;
		m_projectSettingsPanel = nullptr;
		m_sceneSettingsPanel = nullptr;
		m_visualScriptPanel = nullptr;
		m_assetManagerPanel = nullptr;

		m_activeScene = nullptr;
		m_editorScene = nullptr;
		m_frameBuffer->release();

		EditorActionQueue::shutdown();
		EditorModalManager::shutdown();
		EditorResourceManager::shutdown();
		SilicaContext::shutdown();

		DiscordManager::shutdown();

		m_testClient.disconnect();
		m_testServer.stop();

		AXNetwork::NetworkManager::shutdown();
	}

	void EditorLayer::onUpdate(Timestep ts) {

		// TODO: TEMP
		m_testClient.onUpdate(m_activeScene.get());
		m_testServer.onUpdate(m_activeScene.get());


		// ----- Update Discord Rich Presence -----
		DiscordManager::onUpdate();


		// ----- Process Action Queue -----
		EditorActionQueue::execute();


		// ----- Process Logic -----
		bool processLogic = true;
		if (m_sceneState == EditorState::Pause) {
			if (m_stepFrames > 0) m_stepFrames--;
			else processLogic = false;
		}

		EditorState activeState = (m_sceneState == EditorState::Pause) ? m_prePauseState : m_sceneState;
		bool isViewportVisible = m_dock->isTabVisible("Viewport");

		Silica::Vec2 currentViewSize = { 0.0f, 0.0f };
		bool isHovering = false;
		if (m_viewportPanel) {
			currentViewSize = m_viewportPanel->getViewportSize();
			isHovering = m_viewportPanel->isHovered(Silica::Renderer::getMousePosition());
			m_editorCamera.setHoveringSceneViewport(isHovering);
		}

		if (isViewportVisible && currentViewSize.x > 0.0f && currentViewSize.y > 0.0f) {

			// -- Resizing --
			if (m_viewportSize.x != currentViewSize.x || m_viewportSize.y != currentViewSize.y) {
				m_viewportSize = { currentViewSize.x, currentViewSize.y };

				m_frameBuffer->resize((uint32_t)m_viewportSize.x, (uint32_t)m_viewportSize.y);
				m_editorCamera.resize((uint32_t)m_viewportSize.x, (uint32_t)m_viewportSize.y);

				m_viewportTextureID = SilicaContext::getFrameBufferTextureID(m_frameBuffer, m_viewportTextureID);

				if (m_viewportPanel) {
					m_viewportPanel->setViewportTexture(m_viewportTextureID, currentViewSize);
				}
			}

			// -- Update Hover Picking --
			bool isCameraActive = Input::isMouseButtonPressed(MouseButton::Right) || Input::isMouseButtonPressed(MouseButton::Middle);
			if (m_viewportPanel && isHovering && activeState == EditorState::Edit && !isCameraActive) {
				Silica::Vec2 localMouse = m_viewportPanel->getRelativeMousePos();
				m_hoveredEntityID = m_frameBuffer->readPixel(1, (int)localMouse.x, (int)localMouse.y);
			}
			else {
				m_hoveredEntityID = -1;
			}


			// -- Render Scene Into The Framebuffer --
			Renderer::setRenderTarget(m_frameBuffer.get());
			m_frameBuffer->bind();
			m_frameBuffer->clear();

			switch (activeState) {
				case EditorState::Edit: {
					m_editorCamera.onUpdate(ts);
					m_activeScene->onUpdate(ts, m_editorCamera);
					break;
				}
				case EditorState::Play: {
					if (processLogic) {
						m_activeScene->onUpdate(ts);
					}
					else {
						m_activeScene->onUpdate(Timestep(0.0f));
					}
					break;
				}
				case EditorState::Simulate: {
					if (processLogic) {
						m_editorCamera.onUpdate(ts);
						m_activeScene->onUpdateSimulation(ts, m_editorCamera);
					}
					else {
						m_editorCamera.onUpdate(ts);
						m_activeScene->onUpdateSimulation(Timestep(0.0f), m_editorCamera);
					}
					break;
				}
				case EditorState::Pause: { break; }
				default: { break; }
			}

			if (activeState == EditorState::Edit) {
				Mat4 worldM;
				if (m_selectedEntity.isValid()) {
					worldM = m_activeScene->getWorldTransform(m_selectedEntity);

					// -- Update Gizmo Math --
					bool isMouseDown = Input::isMouseButtonPressed(MouseButton::Left);
					bool snap = Input::isKeyPressed(KeyCode::LeftControl);
					float snapValue = (m_transformGizmo.getMode() == GizmoMode::Rotate) ? 15.0f : 1.0f;
					Silica::Vec2 mousePos = m_viewportPanel->getRelativeMousePos();
					bool isCameraActive = Input::isMouseButtonPressed(MouseButton::Right) || Input::isMouseButtonPressed(MouseButton::Middle);
					bool canInteractWithGizmo = !isCameraActive;

					m_transformGizmo.setGizmoScale(EditorSettings::viewportPanelGizmoScale);

					if (isMouseDown && m_transformGizmo.isHovered() && !m_isDraggingGizmo && canInteractWithGizmo) {
						m_isDraggingGizmo = true;
						m_dragStartTransform = m_selectedEntity.getComponent<TransformComponent>();
					}

					auto delta = m_transformGizmo.onUpdate(worldM, m_editorCamera, Vec2(mousePos.x, mousePos.y), Vec2(currentViewSize.x, currentViewSize.y), isMouseDown, snap, snapValue, canInteractWithGizmo);

					if (delta.has_value()) {
						auto& tc = m_selectedEntity.getComponent<TransformComponent>();
						Entity parent = m_selectedEntity.getParent();

						if (m_transformGizmo.getMode() == GizmoMode::Translate) {
							if (parent) {
								Mat4 parentWorldInv = m_activeScene->getWorldTransform(parent).inverse();
								Vec3 newWorldPos = worldM.getTranslation() + delta.value();
								tc.position = (parentWorldInv * Vec4(newWorldPos.x, newWorldPos.y, newWorldPos.z, 1.0f)).xyz();
							}
							else {
								tc.position += delta.value();
							}
						}
						else if (m_transformGizmo.getMode() == GizmoMode::Rotate) {
							float angleDegrees = delta.value().length();

							if (angleDegrees > 0.001f) {
								Vec3 rotationAxis = delta.value().normalized();
								Quat deltaQuat = Quat::fromAxisAngle(rotationAxis, Math::toRadians(angleDegrees));

								tc.rotation = deltaQuat * tc.rotation;
								tc.rotation = tc.rotation.normalized();
							}
						}
						else if (m_transformGizmo.getMode() == GizmoMode::Scale) {
							tc.scale += delta.value();
						}
					}

					if (!isMouseDown && m_isDraggingGizmo) {
						m_isDraggingGizmo = false;
						auto endTransform = m_selectedEntity.getComponent<TransformComponent>();

						if (m_dragStartTransform.position != endTransform.position ||
							m_dragStartTransform.rotation != endTransform.rotation ||
							m_dragStartTransform.scale != endTransform.scale) {

							EditorCommandManager::push(MakeShared<TransformCommand>(m_selectedEntity, m_dragStartTransform, endTransform));
						}
					}
				}

				// -- Draw Overlay --
				m_frameBuffer->clearDepth();
				drawOverlay();

				// -- Draw Gizmo --
				if (m_selectedEntity.isValid()) {
					m_transformGizmo.onRender(worldM, m_editorCamera);
				}
			}

			m_frameBuffer->unbind();
		}

		// ----- Draw Renderer Stats -----
		if (m_viewportPanel && EditorSettings::viewportPanelShowRendererStats) {
			auto& stats = Renderer::getStats();
			char buffer[256];
			snprintf(buffer, sizeof(buffer),
				"FPS: %.0f\nFrame: %.2f ms\nDraw Calls: %d\nMeshes: %d\nInstances: %d",
				1000.0 / Renderer::getFrameTimeMs(), Renderer::getFrameTimeMs(), stats.drawCalls, stats.meshCount3D, stats.instanceCount3D
			);

			m_viewportPanel->setStatsText(buffer);
		}

		// -- Update Material Panel --
		if (m_materialPanel && m_dock->isTabVisible("Material Editor")) {
			m_materialPanel->onUpdate(ts);
		}

		Renderer::renderToSwapChain();
	}

	void EditorLayer::onEvent(Event& e) {

		// ----- Pass Events To Editor Camera -----
		if (m_sceneState == EditorState::Edit || m_sceneState == EditorState::Simulate) m_editorCamera.onEvent(e);

		// ----- Pass Events To Scene -----
		if (m_activeScene) m_activeScene->onEvent(e);

		// ----- Pass Events To Panels -----
		if (m_contentBrowserPanel) m_contentBrowserPanel->onEvent(e);
		if (m_sceneSettingsPanel) m_sceneSettingsPanel->onEvent(e);
		if (m_projectSettingsPanel) m_projectSettingsPanel->onEvent(e);
		if (m_assetLibraryPanel) m_assetLibraryPanel->onEvent(e);
		if (m_visualScriptPanel) m_visualScriptPanel->onEvent(e);
		if (m_propertiesPanel) m_propertiesPanel->onEvent(e);
		if (m_hierarchyPanel) m_hierarchyPanel->onEvent(e);
		if (m_assetManagerPanel) m_assetManagerPanel->onEvent(e);
		if (m_historyPanel) m_historyPanel->onEvent(e);
		if (m_viewportPanel) m_viewportPanel->onEvent(e);
		if (m_materialPanel) m_materialPanel->onEvent(e);

		// ----- Use Events In EditorLayer -----
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<SceneChangedEvent>(AX_BIND_EVENT_FN(EditorLayer::onSceneChanged));
		dispatcher.dispatch<EditorStateChangedEvent>(AX_BIND_EVENT_FN(EditorLayer::onEditorStateChanged));
		dispatcher.dispatch<KeyPressedEvent>(AX_BIND_EVENT_FN(EditorLayer::onKeyPressed));
		dispatcher.dispatch<KeyReleasedEvent>(AX_BIND_EVENT_FN(EditorLayer::onKeyReleased));
		dispatcher.dispatch<EntitySelectedEvent>(AX_BIND_EVENT_FN(EditorLayer::onEntitySelected));
		dispatcher.dispatch<MouseButtonPressedEvent>(AX_BIND_EVENT_FN(EditorLayer::onMouseButtonPressed));
		dispatcher.dispatch<ProjectChangedEvent>(AX_BIND_EVENT_FN(EditorLayer::onProjectChanged));
	}

	void EditorLayer::onGuiRender() {
		// ----- Render Silica -----
		SilicaContext::newFrame();
		float width = (float)Application::get().getWindow().getWidth();
		float height = (float)Application::get().getWindow().getHeight();
		Silica::Renderer::render(m_silicaRoot, width, height);
		SilicaContext::renderDrawData(width, height);
	}

	void EditorLayer::drawOverlay() {
		Renderer2D::beginScene(m_editorCamera);

		// ----- Draw Collider Wireframes -----
		if (m_selectedEntity) {

			// -- Box Collider --
			if (m_selectedEntity.hasComponent<BoxColliderComponent>()) {
				auto& bc = m_selectedEntity.getComponent<BoxColliderComponent>();

				Mat4 worldTransform = m_activeScene->getWorldTransform(m_selectedEntity);
				Vec3 worldScale = worldTransform.getScale();

				worldScale.x = std::abs(worldScale.x);
				worldScale.y = std::abs(worldScale.y);
				worldScale.z = std::abs(worldScale.z);

				Mat4 cleanWorldTransform = Mat4::TRS(worldTransform.getTranslation(), worldTransform.getRotation(), worldScale);
				Mat4 colliderTransform = cleanWorldTransform * Mat4::translation(bc.offset) * Mat4::scale(bc.halfExtents * 2.0f);

				Vec4 color = bc.isTrigger ? Vec4(1.0f, 0.57f, 0.0f, 1.0f) : Vec4(0.0f, 1.0f, 0.0f, 1.0f);
				WireframeRenderer::drawBox(colliderTransform, color);
			}

			// -- Sphere Collider --
			if (m_selectedEntity.hasComponent<SphereColliderComponent>()) {
				auto& sc = m_selectedEntity.getComponent<SphereColliderComponent>();

				Mat4 worldTransform = m_activeScene->getWorldTransform(m_selectedEntity);
				Vec3 worldScale = worldTransform.getScale();

				float maxScale = (std::max)(std::abs(worldScale.x), (std::max)(std::abs(worldScale.y), std::abs(worldScale.z)));
				float radius = sc.radius * maxScale;

				Mat4 cleanWorldTransform = Mat4::TRS(worldTransform.getTranslation(), worldTransform.getRotation(), Vec3::one());
				Mat4 colliderTransform = cleanWorldTransform * Mat4::translation(sc.offset);

				Vec4 color = sc.isTrigger ? Vec4(1.0f, 0.57f, 0.0f, 1.0f) : Vec4(0.0f, 1.0f, 0.0f, 1.0f);
				WireframeRenderer::drawSphere(colliderTransform, radius, color);
			}

			// -- Capsule Collider --
			if (m_selectedEntity.hasComponent<CapsuleColliderComponent>()) {
				auto& cc = m_selectedEntity.getComponent<CapsuleColliderComponent>();

				Mat4 worldTransform = m_activeScene->getWorldTransform(m_selectedEntity);
				Vec3 worldScale = worldTransform.getScale();

				float scaleXZ = (std::max)(std::abs(worldScale.x), std::abs(worldScale.z));
				float radius = cc.radius * scaleXZ;
				float halfHeight = cc.halfHeight * std::abs(worldScale.y);

				Mat4 cleanWorldTransform = Mat4::TRS(worldTransform.getTranslation(), worldTransform.getRotation(), Vec3::one());
				Mat4 colliderTransform = cleanWorldTransform * Mat4::translation(cc.offset);

				Vec4 color = cc.isTrigger ? Vec4(1.0f, 0.57f, 0.0f, 1.0f) : Vec4(0.0f, 1.0f, 0.0f, 1.0f);
				WireframeRenderer::drawCapsule(colliderTransform, radius, halfHeight, color);
			}

			// -- Triangle Collider --
			if (m_selectedEntity.hasComponent<TriangleMeshColliderComponent>()) {
				auto& cc = m_selectedEntity.getComponent<TriangleMeshColliderComponent>();

				if (cc.collisionMesh.isValid()) {
					Ref<Mesh> mesh = AssetManager::get<Mesh>(cc.collisionMesh);
					if (mesh) {
						Mat4 worldTransform = m_activeScene->getWorldTransform(m_selectedEntity);
						Vec4 color = cc.isTrigger ? Vec4(1.0f, 0.57f, 0.0f, 1.0f) : Vec4(0.0f, 1.0f, 0.0f, 1.0f);

						WireframeRenderer::drawMesh(worldTransform, mesh, color);
					}
				}

			}

			// -- Convex Collider --
			if (m_selectedEntity.hasComponent<ConvexColliderComponent>()) {
				auto& cc = m_selectedEntity.getComponent<ConvexColliderComponent>();

				if (cc.collisionMesh.isValid()) {
					Ref<Mesh> mesh = AssetManager::get<Mesh>(cc.collisionMesh);
					if (mesh) {
						Mat4 worldTransform = m_activeScene->getWorldTransform(m_selectedEntity);
						Vec4 color = cc.isTrigger ? Vec4(1.0f, 0.57f, 0.0f, 1.0f) : Vec4(0.0f, 1.0f, 0.0f, 1.0f);

						WireframeRenderer::drawMesh(worldTransform, mesh, color);
					}
				}
			}

			// -- Character Controller --
			if (m_selectedEntity.hasComponent<CharacterControllerComponent>()) {
				auto& cct = m_selectedEntity.getComponent<CharacterControllerComponent>();

				Mat4 worldTransform = m_activeScene->getWorldTransform(m_selectedEntity);
				Mat4 cleanWorldTransform = Mat4::TRS(worldTransform.getTranslation(), worldTransform.getRotation(), Vec3::one());
				float halfHeight = cct.height * 0.5f;
				Vec4 color = Vec4(0.0f, 1.0f, 1.0f, 1.0f);

				WireframeRenderer::drawCapsule(cleanWorldTransform, cct.radius, halfHeight, color);
			}

		}


		// ----- Draw Edit Mode Icons -----
		Mat4 cameraViewMatrix = m_editorCamera.getViewMatrix();

		// -- Camera Icons --
		auto cameraView = m_activeScene->getRegistry().view<CameraComponent>();
		for (auto [entity, camera] : cameraView.each()) {
			Vec3 position = m_activeScene->getWorldTransform({ entity, m_activeScene.get() }).getTranslation();
			Renderer2D::drawBillboard(position, Vec2::one(), cameraViewMatrix, EditorResourceManager::getIcon("CameraIcon"));
		}

		// -- Directional Light Icons --
		auto dirLightView = m_activeScene->getRegistry().view<DirectionalLightComponent>();
		for (auto [entity, light] : dirLightView.each()) {
			Vec3 position = m_activeScene->getWorldTransform({ entity, m_activeScene.get() }).getTranslation();
			Renderer2D::drawBillboard(position, Vec2::one(), cameraViewMatrix, EditorResourceManager::getIcon("LightIcon"));
		}

		// -- Point Light Icons --
		auto pointLightView = m_activeScene->getRegistry().view<PointLightComponent>();
		for (auto [entity, light] : pointLightView.each()) {
			Vec3 position = m_activeScene->getWorldTransform({ entity, m_activeScene.get() }).getTranslation();
			Renderer2D::drawBillboard(position, Vec2::one(), cameraViewMatrix, EditorResourceManager::getIcon("LightIcon"));
		}

		// -- Spot Light Icons --
		auto spotLightView = m_activeScene->getRegistry().view<SpotLightComponent>();
		for (auto [entity, light] : spotLightView.each()) {
			Vec3 position = m_activeScene->getWorldTransform({ entity, m_activeScene.get() }).getTranslation();
			Renderer2D::drawBillboard(position, Vec2::one(), cameraViewMatrix, EditorResourceManager::getIcon("LightIcon"));
		}

		Renderer2D::endScene();
	}

	void EditorLayer::playScene() {
		m_sceneState = EditorState::Play;
		m_editorScene = m_activeScene;

		std::filesystem::path tempPath = "AxionStudio/Config/TempScene.axscene";
		SceneSerializer serializer(m_editorScene);
		serializer.serializeText(tempPath, false);

		m_activeScene = std::make_shared<Scene>();
		SceneSerializer deserializer(m_activeScene);
		deserializer.deserializeText(tempPath);

		m_activeScene->onPhysicsStart();
		m_activeScene->onViewportResized((uint32_t)m_viewportSize.x, (uint32_t)m_viewportSize.y);

		if (m_hierarchyPanel) m_hierarchyPanel->setScene(m_activeScene);
		if (m_assetManagerPanel) m_assetManagerPanel->refresh();
		if (m_viewportPanel) m_viewportPanel->refreshToolbar();
	}

	void EditorLayer::simScene() {
		m_sceneState = EditorState::Simulate;
		m_editorScene = m_activeScene;

		std::filesystem::path tempPath = "AxionStudio/Config/TempScene.axscene";
		SceneSerializer serializer(m_editorScene);
		serializer.serializeText(tempPath, false);

		m_activeScene = std::make_shared<Scene>();
		SceneSerializer deserializer(m_activeScene);
		deserializer.deserializeText(tempPath);

		m_activeScene->onPhysicsStart();
		m_activeScene->onViewportResized((uint32_t)m_viewportSize.x, (uint32_t)m_viewportSize.y);

		if (m_hierarchyPanel) m_hierarchyPanel->setScene(m_activeScene);
		if (m_assetManagerPanel) m_assetManagerPanel->refresh();
		if (m_viewportPanel) m_viewportPanel->refreshToolbar();
	}

	void EditorLayer::stopScene() {
		m_sceneState = EditorState::Edit;
		m_prePauseState = EditorState::Edit;
		m_activeScene->onPhysicsStop();
		m_activeScene = m_editorScene;

		EntitySelectedEvent ev({});
		onEvent(ev);

		if (m_hierarchyPanel) m_hierarchyPanel->setScene(m_activeScene);
		if (m_assetManagerPanel) m_assetManagerPanel->refresh();
		if (m_viewportPanel) m_viewportPanel->refreshToolbar();
	}

	void EditorLayer::newScene() {
		if (m_sceneState != EditorState::Edit) return;
		SceneManager::newScene();
	}

	void EditorLayer::openScene() {
		if (m_sceneState != EditorState::Edit) return;

		std::filesystem::path path;
		std::filesystem::path scenesPath = ProjectManager::getProject()->getAssetsPath() / "scenes";
		if (std::filesystem::exists(scenesPath)) {
			path = FileDialogs::openFile({ {"Axion Scene", "*.axscene"} }, scenesPath);
		}
		else {
			path = FileDialogs::openFile({ {"Axion Scene", "*.axscene"} }, ProjectManager::getProject()->getAssetsPath());
		}

		if (!path.empty()) {
			SceneManager::loadScene(path);
		}
	}

	void EditorLayer::saveScene() {
		if (m_sceneState != EditorState::Edit) return;

		if (!m_currentScenePath.empty()) {
			SceneSerializer serializer(m_activeScene);
			serializer.serializeText(m_currentScenePath, true);
			AX_CORE_LOG_INFO("Successfully saved Scene: {0}", m_currentScenePath.filename().string());
		}
		else {
			saveSceneAs();
		}
	}

	void EditorLayer::saveSceneAs() {
		if (m_sceneState != EditorState::Edit) return;

		std::filesystem::path path;
		std::filesystem::path scenesPath = ProjectManager::getProject()->getAssetsPath() / "scenes";
		if (std::filesystem::exists(scenesPath)) {
			path = FileDialogs::saveFile({ {"Axion Scene", "*.axscene"} }, scenesPath);
		}
		else {
			path = FileDialogs::saveFile({ {"Axion Scene", "*.axscene"} }, ProjectManager::getProject()->getAssetsPath());
		}
		if (!path.empty()) {
			SceneSerializer serializer(m_activeScene);
			serializer.serializeText(path, true);
			m_currentScenePath = path;

			AX_CORE_LOG_INFO("Successfully saved Scene As: {0}", path.filename().string());
		}
	}

	EventReply EditorLayer::onKeyPressed(KeyPressedEvent& e) {
		if (Silica::SWidget::getFocusedWidget()) return EventReply::unhandled();

		bool ctrl = Input::isKeyPressed(KeyCode::LeftControl) || Input::isKeyPressed(KeyCode::RightControl);
		bool shift = Input::isKeyPressed(KeyCode::LeftShift) || Input::isKeyPressed(KeyCode::RightShift);

		switch (e.getKeyCode()) {
			case KeyCode::N: {
				if (ctrl) newScene();
				break;
			}
			case KeyCode::O: {
				if (ctrl) openScene();
				break;
			}
			case KeyCode::S: {
				if (ctrl && shift) saveSceneAs();
				else if (ctrl) saveScene();
				break;
			}
			case KeyCode::Z: {
				if (ctrl && !shift) {
					EditorCommandManager::undo();
				}
				break;
			}
			case KeyCode::Y: {
				if (ctrl) {
					EditorCommandManager::redo();
				}
				break;
			}
			case KeyCode::F5: {
				if (m_sceneState == EditorState::Edit) playScene();
				else stopScene();
				break;
			}
			case KeyCode::F6: {
				if (m_sceneState == EditorState::Edit) simScene();
				else if (m_sceneState == EditorState::Simulate) stopScene();
				break;
			}
			case KeyCode::F8: {
				if (m_sceneState == EditorState::Pause) {
					m_sceneState = m_prePauseState;
				}
				else if (m_sceneState == EditorState::Play || m_sceneState == EditorState::Simulate) {
					m_prePauseState = m_sceneState;
					m_sceneState = EditorState::Pause;
				}

				if (m_viewportPanel) m_viewportPanel->refreshToolbar();
				break;
			}
			case KeyCode::F10: {
				if (m_sceneState == EditorState::Pause) {
					m_stepFrames = 1;
					if (m_viewportPanel) m_viewportPanel->refreshToolbar();
				}
				break;
			}
			case KeyCode::Escape: {
				if (m_sceneState != EditorState::Edit) {
					stopScene();
				}
				else if (m_selectedEntity) {
					EntitySelectedEvent ev({});
					onEvent(ev);
				}
				break;
			}
			case KeyCode::Delete: {
				if (m_sceneState == EditorState::Edit && m_selectedEntity) {
					EditorActionQueue::push([this]() {
						auto cmd = MakeShared<DeleteEntityCommand>(m_activeScene, m_selectedEntity);
						cmd->execute();
						EditorCommandManager::push(cmd);

						EntitySelectedEvent ev({});
						onEvent(ev);
						});
				}
				break;
			}
		}

		return EventReply::unhandled();
	}

	EventReply EditorLayer::onEditorStateChanged(EditorStateChangedEvent& ev) {
		EditorState newState = ev.getState();

		if (newState == EditorState::Play) playScene();
		else if (newState == EditorState::Simulate) simScene();
		else if (newState == EditorState::Edit) stopScene();
		else if (newState == EditorState::Pause) {
			m_prePauseState = m_sceneState;
			m_sceneState = EditorState::Pause;
		}

		return EventReply::unhandled();
	}

	EventReply EditorLayer::onKeyReleased(KeyReleasedEvent& ev) {

		// -- Handle Gizmo Mode Switching (Q, W, E, R, T) --
		if (!Input::isMouseButtonPressed(MouseButton::Right) && m_selectedEntity.isValid() && m_viewportPanel->isHovered(Silica::Renderer::getMousePosition())) {
			bool changed = false;

			if (ev.getKeyCode() == KeyCode::W && m_transformGizmo.getMode() != GizmoMode::Translate) {
				m_transformGizmo.setMode(GizmoMode::Translate);
				changed = true;
			}
			if (ev.getKeyCode() == KeyCode::E && m_transformGizmo.getMode() != GizmoMode::Rotate) {
				m_transformGizmo.setMode(GizmoMode::Rotate);
				changed = true;
			}
			if (ev.getKeyCode() == KeyCode::R && m_transformGizmo.getMode() != GizmoMode::Scale) {
				m_transformGizmo.setMode(GizmoMode::Scale);
				changed = true;
			}
			if (ev.getKeyCode() == KeyCode::T) {
				m_transformGizmo.setSpace(m_transformGizmo.getSpace() == GizmoSpace::Local ? GizmoSpace::Global : GizmoSpace::Local);
				changed = true;
			}

			if (changed && m_viewportPanel) m_viewportPanel->refreshToolbar();
		}

		return EventReply::unhandled();
	}

	void EditorLayer::openTextEditorTab(const std::filesystem::path& filepath) {
		std::string pathStr = filepath.generic_string();

		if (m_openTextEditors.find(pathStr) != m_openTextEditors.end()) {
			std::string existingTabName = m_openTextEditors[pathStr];
			if (m_dock) m_dock->openTab(existingTabName);
			return;
		}

		// -- Setup Language Profile --
		Quartz::LanguageProfile lang = Quartz::LanguageProfile::CPlusPlus();
		std::string ext = filepath.extension().string();

		if (ext == ".cs") {
			lang = Quartz::LanguageProfile::CSharp();
		}
		else if (ext == ".hlsl" || ext == ".glsl") {
			lang = Quartz::LanguageProfile::HLSL();
		}
		else if (ext == ".yaml" || ext == ".axscene" || ext == ".axproj" || EditorUtils::isEngineAssetExtension(filepath)) {
			lang = Quartz::LanguageProfile::YAML();
		}

		auto textEditorWidget = Silica::MakeWidget<Quartz::SQuartzEditor>({
			.initialText = "",
			.font = &m_font,
			.language = lang
		});

		Quartz::SQuartzEditor* editorRaw = textEditorWidget.get();

		textEditorWidget->setSaveFileCallback([editorRaw]() {
			if (editorRaw) {
				editorRaw->saveFile();
				AX_CORE_LOG_INFO("Saved file: {0}", editorRaw->getCurrentFilePath().filename().string());
			}
		});

		// -- Load Actual File --
		textEditorWidget->openFile(filepath);

		// -- Register Tab --
		std::string tabName = filepath.filename().string() + "##" + filepath.generic_string();
		m_openTextEditors[pathStr] = tabName;

		if (m_dock) {
			m_dock->registerTab(tabName, textEditorWidget);
			m_dock->openTab(tabName);
		}

		Silica::SWidget::setFocusedWidget(editorRaw);
	}

	void EditorLayer::openPreferences() {
		if (!m_settingsModal) {
			m_settingsModal = MakeShared<SettingsModal>();
			m_settingsModal->setEventCallback(AX_BIND_EVENT_FN(EditorLayer::onEvent));
		}

		auto widget = m_settingsModal->getWidget([]() {
			EditorModalManager::close();
		});

		EditorModalManager::open(widget);
	}

	void EditorLayer::openMaterialEditor(const std::filesystem::path& filepath) {
		m_materialPanel->setMaterial(filepath);
		m_dock->openTab("Material Editor");
		m_dock->focusTab("Material Editor");
	}

	void EditorLayer::openVisualScriptPanel(const std::filesystem::path& filepath) {
		m_visualScriptPanel->openScript(filepath);
		m_dock->openTab("Visual Script");
		m_dock->focusTab("Visual Script");
	}

	void EditorLayer::openSceneInViewport(const std::filesystem::path& filepath) {
		SceneManager::loadScene(filepath);
		m_dock->openTab("Viewport");
		m_dock->focusTab("Viewport");
	}

	EventReply EditorLayer::onSceneChanged(SceneChangedEvent& ev) {
		EditorCommandManager::clear();

		m_activeScene = SceneManager::getScene();
		m_currentScenePath = SceneManager::getScenePath();

		// -- Create EntitySelectedEvent --
		EntitySelectedEvent emptySelectionEv({});
		onEvent(emptySelectionEv);

		// -- Update Discord Rich Presence --
		if (ProjectManager::hasProject()) {
			std::string projName = ProjectManager::getProject()->getName();
			std::string sceneName = m_activeScene ? m_activeScene->getTitle() : "Untitled";
			DiscordManager::setPresence("Editing: " + sceneName, "Project: " + projName);
		}

		AX_CORE_LOG_INFO("EditorLayer successfully synced with new Scene!");
		return EventReply::unhandled();
	}

	EventReply EditorLayer::onProjectChanged(ProjectChangedEvent& ev) {
		EditorCommandManager::clear();

		// -- Call AAP Asset Migrator --
		if (ProjectManager::hasProject()) {
			AAP::AssetMigrator::upgradeLegacyAssets(ProjectManager::getProject());
		}

		return EventReply::unhandled();
	}

	EventReply EditorLayer::onEntitySelected(EntitySelectedEvent& ev) {
		m_selectedEntity = ev.getEntity();
		return EventReply::unhandled();
	}

	EventReply EditorLayer::onMouseButtonPressed(MouseButtonPressedEvent& ev) {
		if (ev.getMouseButton() == MouseButton::Left) {
			bool isCameraActive = Input::isMouseButtonPressed(MouseButton::Right) || Input::isMouseButtonPressed(MouseButton::Middle);
			if (isCameraActive) return EventReply::unhandled();

			if (m_viewportPanel && m_viewportPanel->isHovered(Silica::Renderer::getMousePosition())) {

				if (m_selectedEntity.isValid() && m_transformGizmo.isHovered()) {
					return EventReply::unhandled();
				}

				int pixelData = m_hoveredEntityID;
				Entity clickedEntity = {};

				if (pixelData != -1) {
					clickedEntity = { (entt::entity)pixelData, m_activeScene.get() };
				}

				EntitySelectedEvent selectedEv(clickedEntity);
				onEvent(selectedEv);

				if (m_hierarchyPanel) m_hierarchyPanel->rebuildUI();

			}
		}
		return EventReply::unhandled();
	}

	void EditorLayer::openCreateProjectModal() {
		if (!m_createProjectModal) {
			m_createProjectModal = MakeShared<CreateProjectModal>();
		}

		EditorModalManager::open(m_createProjectModal->getWidget());
	}

	void EditorLayer::openExportProjectModal() {
		if (!m_exportProjectModal) {
			m_exportProjectModal = MakeShared<ExportProjectModal>();
		}

		EditorModalManager::open(m_exportProjectModal->getWidget());
	}

	void EditorLayer::openSystemInfoModal() {
		if (!m_systemInfoModal) {
			m_systemInfoModal = MakeShared<SystemInfoModal>();
		}

		EditorModalManager::open(m_systemInfoModal->getWidget());
	}

}
