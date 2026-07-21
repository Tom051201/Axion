#include "EditorLayer.h"

#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/scene/Prefab.h"

#include "AxionStudio/Vendor/Silica/include/SOverlay.h"
#include "AxionStudio/Vendor/Silica/include/SAlign.h"
#include "AxionStudio/Vendor/Silica/include/SWorkspace.h"
#include "AxionStudio/Vendor/Silica/include/SBorderLayout.h"
#include "AxionStudio/Vendor/Silica/include/Theme.h"
#include "AxionStudio/Vendor/Silica/include/SLoadingToast.h"

#include "AxionEngine/Vendor/yaml-cpp/include/yaml-cpp/yaml.h"

#include "AxionStudio/Source/ui/EditorMenuBar.h"
#include "AxionStudio/Source/ui/panels/ViewportPanel.h"
#include "AxionStudio/Source/ui/panels/ContentBrowserPanel.h"
#include "AxionStudio/Source/ui/panels/VisualScriptPanel.h"
#include "AxionStudio/Source/ui/panels/SceneOverviewPanel.h"
#include "AxionStudio/Source/ui/panels/ProjectOverviewPanel.h"
#include "AxionStudio/Source/ui/panels/AssetManagerPanel.h"
#include "AxionStudio/Source/ui/panels/HierarchyPanel.h"
#include "AxionStudio/Source/ui/panels/EntityPropertiesPanel.h"
#include "AxionStudio/Source/core/EditorResourceManager.h"
#include "AxionStudio/Source/core/EditorActionQueue.h"
#include "AxionStudio/Source/core/EditorModalManager.h"
#include "AxionStudio/Source/core/SilicaContext.h"
#include "AxionStudio/Source/core/WireframeRenderer.h"

namespace Axion {

	EditorLayer::EditorLayer() : Layer("AxionEditorLayer"), m_editorCamera(1280, 720) {}

	void EditorLayer::onAttach() {

		// ----- Load Editor Resources -----
		EditorResourceManager::initialize();
		EditorResourceManager::loadIcon("PlayButton", "AxionStudio/Resources/toolbar/PlayIcon.png");
		EditorResourceManager::loadIcon("StopButton", "AxionStudio/Resources/toolbar/StopIcon.png");
		EditorResourceManager::loadIcon("PauseButton", "AxionStudio/Resources/toolbar/PauseIcon.png");
		EditorResourceManager::loadIcon("SimulateButton", "AxionStudio/Resources/toolbar/SimulateIcon.png");
		EditorResourceManager::loadIcon("StepButton", "AxionStudio/Resources/toolbar/StepIcon.png");
		EditorResourceManager::loadIcon("CameraIcon", "AxionStudio/Resources/CameraIcon.png");
		EditorResourceManager::loadIcon("LightIcon", "AxionStudio/Resources/LightIcon.png");
		EditorResourceManager::loadIcon("2DCamIcon", "AxionStudio/Resources/toolbar/2dIcon.png");
		EditorResourceManager::loadIcon("3DCamIcon", "AxionStudio/Resources/toolbar/3dIcon.png");
		EditorResourceManager::loadIcon("FolderIcon", "AxionStudio/Resources/contentbrowser/FolderIcon.png");
		EditorResourceManager::loadIcon("FileIcon", "AxionStudio/Resources/contentbrowser/FileIcon.png");
		EditorResourceManager::loadIcon("BackIcon", "AxionStudio/Resources/contentbrowser/BackIcon.png");
		EditorResourceManager::loadIcon("ForwardIcon", "AxionStudio/Resources/contentbrowser/ForwardIcon.png");
		EditorResourceManager::loadIcon("RefreshIcon", "AxionStudio/Resources/contentbrowser/RefreshIcon.png");
		EditorResourceManager::loadIcon("AddFolderIcon", "AxionStudio/Resources/contentbrowser/AddFolderIcon.png");


		// ----- Setup Project And Scene -----
		ProjectManager::loadProject("AxionStudio/Projects/TestProject/TestProject.axproj");
		m_activeScene = SceneManager::getScene();


		// ----- Setup Framebuffer -----
		FrameBufferSpecification fbs;
		fbs.width = 1280;
		fbs.height = 720;
		fbs.textureFormat = ColorFormat::RGBA8;
		fbs.depthStencilFormat = DepthStencilFormat::DEPTH32F;
		m_frameBuffer = FrameBuffer::create(fbs);
		m_viewportSize = { (float)fbs.width, (float)fbs.height };


		// ----- Init Silica Backend -----
		SilicaContext::initialize();


		// ----- Load Font -----
		if (m_font.loadFromFile("AxionStudio/Resources/fonts/openSans/OpenSans-Bold.ttf", 18.0f)) {
			SilicaContext::uploadFontAtlas(m_font);
			Silica::GetTheme().Font_Default = &m_font;
		}
		else {
			AX_CORE_LOG_WARN("Silica: Failed to load OpenSans font!");
		}


		// ----- Build UI -----
		m_hierarchyPanel = std::make_shared<HierarchyPanel>();
		m_hierarchyPanel->setSelectionCallback([this](Entity e) { selectEntity(e); });
		m_hierarchyPanel->setScene(m_activeScene);
		auto hierarchyWidget = m_hierarchyPanel->getWidget();

		m_propertiesPanel = std::make_shared<EntityPropertiesPanel>();
		m_propertiesPanel->setHierarchyRefreshCallback([this]() { if (m_hierarchyPanel) m_hierarchyPanel->refresh(); });
		auto propertiesWidget = m_propertiesPanel->getWidget();

		m_contentBrowserPanel = std::make_shared<ContentBrowser>();
		m_contentBrowserPanel->setup();
		m_contentBrowserPanel->setOpenVisualScriptPanelCallback([this](const std::filesystem::path& path) {
			m_visualScriptPanel->openScript(path);
			m_dock->focusTab("Visual Script");
		});
		m_contentBrowserPanel->setModalCallbacks(
			[this](Silica::WidgetPtr modalWidget) {
				EditorModalManager::open(modalWidget);
			},
			[this]() {
				EditorModalManager::close();
			}
		);
		m_contentBrowserPanel->setAssetRenamedCallback([this](const std::filesystem::path& oldPath, const std::filesystem::path& newPath) {
			m_visualScriptPanel->onAssetRenamed(oldPath, newPath);
			// Notify Other Panels Here As Well
		});
		m_contentBrowserPanel->setAssetDeletedCallback([this](const std::filesystem::path& path) {
			m_visualScriptPanel->onAssetDeleted(path);
			// Notify Other Panels Here As Well
		});
		auto contentBrowserWidget = m_contentBrowserPanel->getWidget();

		m_projectOverviewPanel = std::make_shared<ProjectPanel>();
		m_projectOverviewPanel->setProject(ProjectManager::getProject());
		auto projectSettings = m_projectOverviewPanel->getWidget();

		m_sceneOverviewPanel = std::make_shared<SceneOverviewPanel>();
		m_sceneOverviewPanel->setScene(m_activeScene);
		auto sceneSettings = m_sceneOverviewPanel->getWidget();

		m_viewportTextureID = SilicaContext::getFrameBufferTextureID(m_frameBuffer);
		m_viewportPanel = std::make_shared<ViewportPanel>();
		m_viewportPanel->setup(&m_sceneState, &m_prePauseState, &m_stepFrames, &m_editorCamera);
		m_viewportPanel->setCallbacks([this]() { playScene(); }, [this]() { simScene(); }, [this]() { stopScene(); });
		m_viewportPanel->setSkyboxDropCallback([this](const std::filesystem::path& path) { setSkybox(path); });
		m_viewportPanel->setSceneDropCallback([this](const std::filesystem::path& path) {
			SceneManager::loadScene(path);
			m_activeScene = SceneManager::getScene();

			if (m_hierarchyPanel) m_hierarchyPanel->setScene(m_activeScene);
			if (m_sceneOverviewPanel) m_sceneOverviewPanel->setScene(m_activeScene);

			AX_CORE_LOG_INFO("Successfully loaded Scene from drop: {0}", path.filename().string());
		});
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
						selectEntity(spawnedEntity);
						if (m_hierarchyPanel) m_hierarchyPanel->rebuildUI();
					}
				}
			}
		});
		m_viewportPanel->setVisualScriptDropCallback([this](const std::filesystem::path& path) {
			if (m_visualScriptPanel) {
				m_visualScriptPanel->openScript(path);
				if (m_dock) m_dock->focusTab("Visual Script");
			}
		});
		auto fullViewportPanel = m_viewportPanel->getWidget();

		m_visualScriptPanel = std::make_shared<VisualScriptPanel>();
		auto visualScriptWidget = m_visualScriptPanel->getWidget();

		m_assetManagerPanel = std::make_shared<AssetManagerPanel>();
		auto assetManagerWidget = m_assetManagerPanel->getWidget();

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
		m_dock->registerTab("Project Settings", projectSettings);
		m_dock->registerTab("Scene Settings", sceneSettings);
		m_dock->registerTab("Asset Inspector", assetManagerWidget);

		m_dock->loadLayout("AxionStudio/Config/EditorLayout.ini");

		if (!m_dock->getRootNode() || (m_dock->getRootNode()->tabs.size() <= 1 && m_dock->getRootNode()->splitDirection == Silica::SplitDirection::None)) {
			auto root = m_dock->getRootNode();
			m_dock->splitNode(root, Silica::SplitDirection::Vertical, 0.75f, "Content Browser", contentBrowserWidget, false);
			auto topHalf = root->child[0];
			m_dock->splitNode(topHalf, Silica::SplitDirection::Horizontal, 0.2f, "Viewport", fullViewportPanel, false);
			auto viewportNode = topHalf->child[1];
			viewportNode->tabs.push_back({
				.title = "Visual Script",
				.content = visualScriptWidget,
				.hitRect = {}
			});
			m_dock->splitNode(topHalf->child[0], Silica::SplitDirection::Vertical, 0.5f, "Properties", propertiesWidget, false);
			m_dock->splitNode(viewportNode, Silica::SplitDirection::Horizontal, 0.75f, "Project Settings", projectSettings, false);
			viewportNode->child[1]->tabs.push_back({
				.title = "Scene Settings",
				.content = sceneSettings,
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
		menuCallbacks.newScene = [this]() { newScene(); };
		menuCallbacks.openScene = [this]() { openScene(); };
		menuCallbacks.saveScene = [this]() { saveScene(); };
		menuCallbacks.saveSceneAs = [this]() { saveSceneAs(); };
		menuCallbacks.exitEditor = []() { /* Application::get().close(); */ };
		auto menuBar = EditorMenuBar::construct(m_dock, menuCallbacks);


		// ----- Assemble -----
		m_mainLayout = Silica::MakeWidget<Silica::SBorderLayout>({
			.topBar = menuBar,
			.contentArea = workspace
		});

		// -- Script Compiler Toast --
		auto compilationToast = Silica::MakeWidget<Silica::SLoadingToast>({
			.text = "Compiling C# Scripts...",
			.isVisible = []() { return ProjectManager::isCompilingScripts(); }
		});

		// -- Scene Loading Toast ---
		auto sceneLoadingToast = Silica::MakeWidget<Silica::SLoadingToast>({
			.text = "Loading Scene & Assets...",
			.isVisible = []() {
				return SceneManager::isLoadingScene() || AssetManager::isLoadingAssets();
			}
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


		EditorModalManager::initialize(m_silicaRoot, m_mainLayout);
		SilicaContext::bindWndProcCallback(m_silicaRoot);


		// -- Load Editor State --
		std::string settingsPath = "AxionStudio/Config/EditorSettings.yaml";
		if (std::filesystem::exists(settingsPath)) {
			try {
				YAML::Node config = YAML::LoadFile(settingsPath);

				if (config["MaxAssetsPerFrame"]) {
					AssetManager::setMaxAssetsPerFrame(config["MaxAssetsPerFrame"].as<uint32_t>());
				}

				m_contentBrowserPanel->loadSettings(config);
				// Load Other Panels Here As Well
			}
			catch (const YAML::Exception& e) {
				AX_CORE_LOG_WARN("Failed to parse Editor Settings: {}", e.what());
			}
		}

	}

	void EditorLayer::onDetach() {
		// -- Save Editor State --
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "MaxAssetsPerFrame" << YAML::Value << AssetManager::getMaxAssetsPerFrame();
		m_contentBrowserPanel->saveSettings(out);
		// Save Other Panels Here As Well
		out << YAML::EndMap;
		std::ofstream fout("AxionStudio/Config/EditorSettings.yaml");
		if (fout.is_open()) {
			fout << out.c_str();
			fout.close();
		}


		m_selectedEntity = {};
		if (m_propertiesPanel) m_propertiesPanel->setEntity({});
		if (m_dock) m_dock->saveLayout("AxionStudio/Config/EditorLayout.ini");

		SilicaContext::unbindWndProcCallback();

		m_silicaRoot = nullptr;
		m_dock = nullptr;
		m_hierarchyPanel = nullptr;
		m_propertiesPanel = nullptr;
		m_contentBrowserPanel = nullptr;
		m_projectOverviewPanel = nullptr;
		m_sceneOverviewPanel = nullptr;
		m_visualScriptPanel = nullptr;
		m_assetManagerPanel = nullptr;

		m_activeScene = nullptr;
		m_editorScene = nullptr;
		m_frameBuffer->release();

		EditorActionQueue::shutdown();
		EditorModalManager::shutdown();
		EditorResourceManager::shutdown();
		SilicaContext::shutdown();
	}

	void EditorLayer::onUpdate(Timestep ts) {

		// ----- Process Action Queue -----
		EditorActionQueue::execute();


		// ----- Process Logic -----
		bool processLogic = true;
		if (m_sceneState == EditorState::Pause) {
			if (m_stepFrames > 0) m_stepFrames--;
			else processLogic = false;
		}

		EditorState activeState = (m_sceneState == EditorState::Pause) ? m_prePauseState : m_sceneState;

		Silica::Vec2 currentViewSize = { 0.0f, 0.0f };
		if (m_viewportPanel) {
			currentViewSize = m_viewportPanel->getViewportSize();
			bool isHovering = m_viewportPanel->isHovered(Silica::Renderer::getMousePosition());
			m_editorCamera.setHoveringSceneViewport(isHovering);
		}

		if (currentViewSize.x > 0.0f && currentViewSize.y > 0.0f) {

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
				drawOverlay();
			}

			m_frameBuffer->unbind();
		}

		// ----- Draw Renderer Stats -----
		if (m_viewportPanel) {
			auto& stats = Renderer::getStats();
			char buffer[256];
			snprintf(buffer, sizeof(buffer),
				"FPS: %.0f\nFrame: %.2f ms\nDraw Calls: %d\nMeshes: %d\nInstances: %d",
				1000.0 / Renderer::getFrameTimeMs(), Renderer::getFrameTimeMs(), stats.drawCalls, stats.meshCount3D, stats.instanceCount3D
			);

			m_viewportPanel->setStatsText(buffer);
		}

		Renderer::renderToSwapChain();
	}

	void EditorLayer::onEvent(Event & e) {

		// ----- Pass Events To Editor Camera -----
		if (m_sceneState == EditorState::Edit || m_sceneState == EditorState::Simulate) {
			m_editorCamera.onEvent(e);
		}


		// ----- Pass Events To Scene -----
		if (m_activeScene) {
			m_activeScene->onEvent(e);
		}


		// ----- Pass Events To Panels -----
		if (m_contentBrowserPanel) m_contentBrowserPanel->onEvent(e);
		if (m_sceneOverviewPanel) m_sceneOverviewPanel->onEvent(e);
		if (m_projectOverviewPanel) m_projectOverviewPanel->onEvent(e);


		// ----- Scene Changed Event -----
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<SceneChangedEvent>([this](SceneChangedEvent& ev) {
			m_activeScene = SceneManager::getScene();
			m_currentScenePath = SceneManager::getScenePath();

			if (m_hierarchyPanel) m_hierarchyPanel->setScene(m_activeScene);
			if (m_sceneOverviewPanel) m_sceneOverviewPanel->setScene(m_activeScene);
			if (m_assetManagerPanel) m_assetManagerPanel->refresh();

			return false;
		});
		dispatcher.dispatch<KeyPressedEvent>(AX_BIND_EVENT_FN(EditorLayer::onKeyPressed));

	}

	void EditorLayer::onGuiRender() {
		// ----- Render Silica -----
		SilicaContext::newFrame();
		float width = (float)Application::get().getWindow().getWidth();
		float height = (float)Application::get().getWindow().getHeight();
		Silica::Renderer::render(m_silicaRoot, width, height);
		SilicaContext::renderDrawData(width, height);

		// ----- Render ImGuizmo Overlay -----
		if (m_sceneState == EditorState::Edit && m_selectedEntity && m_gizmoType != -1) {

			Silica::Vec2 viewPos = m_viewportPanel->getViewportPosition();
			Silica::Vec2 viewSize = m_viewportPanel->getViewportSize();

			if (viewSize.x > 0.0f && viewSize.y > 0.0f) {
				// -- Create An Invisible Window --
				ImGuiViewport* mainViewport = ImGui::GetMainViewport();
				ImGui::SetNextWindowPos({ viewPos.x + mainViewport->Pos.x, viewPos.y + mainViewport->Pos.y });
				ImGui::SetNextWindowSize({ viewSize.x, viewSize.y });
				ImGui::SetNextWindowViewport(mainViewport->ID);
				ImGui::SetNextWindowBgAlpha(0.0f);

				ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking |
					ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings |
					ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing;

				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
				ImGui::Begin("GizmoOverlay", nullptr, flags);

				// -- Setup ImGuizmo --
				ImGuizmo::SetOrthographic(m_editorCamera.is2D());
				ImGuizmo::SetDrawlist();
				ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

				// -- Input: Mode Switching --
				if (!ImGui::IsAnyItemActive() && !Input::isMouseButtonPressed(MouseButton::Right)) {
					if (ImGui::IsKeyPressed(ImGuiKey_Q)) m_gizmoType = -1;
					if (ImGui::IsKeyPressed(ImGuiKey_W)) m_gizmoType = ImGuizmo::TRANSLATE;
					if (ImGui::IsKeyPressed(ImGuiKey_E)) m_gizmoType = ImGuizmo::ROTATE;
					if (ImGui::IsKeyPressed(ImGuiKey_R)) m_gizmoType = ImGuizmo::SCALE;
				}

				// -- Camera --
				const Mat4& cameraView = m_editorCamera.getViewMatrix();
				const Mat4& cameraProjection = m_editorCamera.getProjectionMatrix();

				// -- Entity Transform --
				auto& tc = m_selectedEntity.getComponent<TransformComponent>();
				Mat4 worldM = m_activeScene->getWorldTransform(m_selectedEntity);

				// -- To float[16] for ImGuizmo --
				DirectX::XMFLOAT4X4 objF4;
				DirectX::XMStoreFloat4x4(&objF4, worldM.toXM());
				float object[16];
				memcpy(object, &objF4, sizeof(objF4));

				// -- Snapping --
				bool snap = Input::isKeyPressed(KeyCode::LeftControl);
				float snapValue = 0.5f;
				if (m_gizmoType == ImGuizmo::ROTATE) snapValue = 45.0f;
				float snapValues[3] = { snapValue, snapValue, snapValue };

				// -- Do gizmo stuff --
				ImGuizmo::Manipulate(
					cameraView.data(),
					cameraProjection.data(),
					(ImGuizmo::OPERATION)m_gizmoType,
					ImGuizmo::LOCAL,
					object,
					nullptr,
					snap ? snapValues : nullptr
				);

				// -- Apply changes --
				if (ImGuizmo::IsUsing()) {
					DirectX::XMMATRIX newM = DirectX::XMLoadFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(object));
					Mat4 updatedWorld = Mat4::fromXM(newM);

					Entity parent = m_selectedEntity.getParent();
					if (parent) {
						Mat4 parentWorld = m_activeScene->getWorldTransform(parent);
						Mat4 localM = parentWorld.inverse() * updatedWorld;

						TRSData trs = localM.decompose();
						tc.position = trs.translation;
						tc.rotation = trs.rotation;
						tc.scale = trs.scale;
					}
					else {
						TRSData trs = updatedWorld.decompose();
						tc.position = trs.translation;
						tc.rotation = trs.rotation;
						tc.scale = trs.scale;
					}
				}

				ImGui::End();
				ImGui::PopStyleVar();
			}
		}

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

				float maxScale = std::max(std::abs(worldScale.x), std::max(std::abs(worldScale.y), std::abs(worldScale.z)));
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

				float scaleXZ = std::max(std::abs(worldScale.x), std::abs(worldScale.z));
				float radius = cc.radius * scaleXZ;
				float halfHeight = cc.halfHeight * std::abs(worldScale.y);

				Mat4 cleanWorldTransform = Mat4::TRS(worldTransform.getTranslation(), worldTransform.getRotation(), Vec3::one());
				Mat4 colliderTransform = cleanWorldTransform * Mat4::translation(cc.offset);

				Vec4 color = cc.isTrigger ? Vec4(1.0f, 0.57f, 0.0f, 1.0f) : Vec4(0.0f, 1.0f, 0.0f, 1.0f);
				WireframeRenderer::drawCapsule(colliderTransform, radius, halfHeight, color);
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

		selectEntity({});

		if (m_hierarchyPanel) m_hierarchyPanel->setScene(m_activeScene);
		if (m_assetManagerPanel) m_assetManagerPanel->refresh();
		if (m_viewportPanel) m_viewportPanel->refreshToolbar();
	}

	void EditorLayer::setSkybox(const std::filesystem::path& path) {
		if (m_activeScene) {
			UUID skyboxUUID = AssetManager::getAssetUUID(path);
			if (skyboxUUID.isValid()) {
				m_activeScene->setSkybox(skyboxUUID);
				AX_CORE_LOG_INFO("Successfully applied Skybox: {0}", path.filename().string());

				if (m_sceneOverviewPanel) {
					m_sceneOverviewPanel->rebuildUI();
				}
			}
			else {
				AX_CORE_LOG_WARN("Attempted to drop an invalid Skybox asset!");
			}
		}
	}

	void EditorLayer::selectEntity(Entity selectedEntity) {
		EditorActionQueue::push([this, selectedEntity]() {
			m_selectedEntity = selectedEntity;

			if (m_propertiesPanel) {
				m_propertiesPanel->setEntity(selectedEntity);
			}
		});
	}

	void EditorLayer::newScene() {
		if (m_sceneState != EditorState::Edit) return;

		m_activeScene = std::make_shared<Scene>();
		m_currentScenePath = "";

		if (m_hierarchyPanel) m_hierarchyPanel->setScene(m_activeScene);
		if (m_sceneOverviewPanel) m_sceneOverviewPanel->setScene(m_activeScene);
		selectEntity({});

		AX_CORE_LOG_INFO("Created New Scene");
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
			m_activeScene = SceneManager::getScene();
			m_currentScenePath = path;

			if (m_hierarchyPanel) m_hierarchyPanel->setScene(m_activeScene);
			if (m_sceneOverviewPanel) m_sceneOverviewPanel->setScene(m_activeScene);
			selectEntity({});

			AX_CORE_LOG_INFO("Successfully loaded Scene: {0}", path.filename().string());
		}
	}

	void EditorLayer::saveScene() {
		if (m_sceneState != EditorState::Edit) return;

		if (!m_currentScenePath.empty()) {
			SceneSerializer serializer(m_activeScene);
			serializer.serializeText(m_currentScenePath, false);
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
			serializer.serializeText(path, false);
			m_currentScenePath = path;

			AX_CORE_LOG_INFO("Successfully saved Scene As: {0}", path.filename().string());
		}
	}

	bool EditorLayer::onKeyPressed(KeyPressedEvent& e) {
		if (Silica::SWidget::getFocusedWidget()) return false;

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
					selectEntity({});
				}
				break;
			}
			case KeyCode::Delete: {
				if (m_sceneState == EditorState::Edit && m_selectedEntity) {
					EditorActionQueue::push([this]() {
						// -- Remove From Parent --
						if (m_selectedEntity.hasComponent<RelationshipComponent>()) {
							auto& rel = m_selectedEntity.getComponent<RelationshipComponent>();
							if (rel.parent != entt::null) {
								Entity parent = { rel.parent, m_activeScene.get() };
								auto& parentRel = parent.getComponent<RelationshipComponent>();
								auto it = std::find(parentRel.children.begin(), parentRel.children.end(), (entt::entity)m_selectedEntity);
								if (it != parentRel.children.end()) parentRel.children.erase(it);
							}
						}

						// --Destroy Entity And All Descendants --
						auto destroyHierarchy = [this](Entity e, auto& self) -> void {
							if (e.hasComponent<RelationshipComponent>()) {
								auto childrenCopy = e.getComponent<RelationshipComponent>().children;
								for (auto childHandle : childrenCopy) {
									self(Entity{ childHandle, m_activeScene.get() }, self);
								}
							}
							m_activeScene->destroyEntity(e);
						};

						destroyHierarchy(m_selectedEntity, destroyHierarchy);

						selectEntity({});
						if (m_hierarchyPanel) m_hierarchyPanel->refresh();
					});
				}
				break;
			}
		}

		return false;
	}

}
