#include "EditorLayer.h"

#include "AxionStudio/Vendor/Silica/include/SOverlay.h"
#include "AxionStudio/Vendor/Silica/include/SAlign.h"
#include "AxionStudio/Vendor/Silica/include/SWorkspace.h"
#include "AxionStudio/Vendor/Silica/include/SBorderLayout.h"

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
		}
		else {
			AX_CORE_LOG_WARN("Silica: Failed to load OpenSans font!");
		}


		// ----- Build UI -----
		m_hierarchyPanel = std::make_shared<HierarchyPanel>();
		m_hierarchyPanel->setSelectionCallback([this](Entity e) { selectEntity(e); });
		m_hierarchyPanel->setScene(m_activeScene);
		auto hierarchyWidget = m_hierarchyPanel->getWidget(&m_font);

		m_propertiesPanel = std::make_shared<EntityPropertiesPanel>();
		m_propertiesPanel->setHierarchyRefreshCallback([this]() { if (m_hierarchyPanel) m_hierarchyPanel->refresh(); });
		auto propertiesWidget = m_propertiesPanel->getWidget(&m_font);

		m_contentBrowserPanel = std::make_shared<ContentBrowser>();
		m_contentBrowserPanel->setup();
		m_contentBrowserPanel->setOpenVisualScriptPanelCallback([this](const std::filesystem::path& path) { m_visualScriptPanel->openScript(path); /* TODO: add focus for dockspace here*/});
		auto contentBrowserWidget = m_contentBrowserPanel->getWidget(&m_font);

		m_projectOverviewPanel = std::make_shared<ProjectPanel>();
		m_projectOverviewPanel->setProject(ProjectManager::getProject());
		auto projectSettings = m_projectOverviewPanel->getWidget(&m_font);

		m_sceneOverviewPanel = std::make_shared<SceneOverviewPanel>();
		m_sceneOverviewPanel->setScene(m_activeScene);
		auto sceneSettings = m_sceneOverviewPanel->getWidget(&m_font);

		m_viewportTextureID = SilicaContext::getFrameBufferTextureID(m_frameBuffer);
		m_viewportPanel = std::make_shared<ViewportPanel>();
		m_viewportPanel->setup(&m_sceneState, &m_prePauseState, &m_stepFrames, &m_editorCamera);
		m_viewportPanel->setCallbacks([this]() { playScene(); }, [this]() { simScene(); }, [this]() { stopScene(); });
		auto fullViewportPanel = m_viewportPanel->getWidget(&m_font);

		m_visualScriptPanel = std::make_shared<VisualScriptPanel>();
		auto visualScriptWidget = m_visualScriptPanel->getWidget(&m_font);

		m_assetManagerPanel = std::make_shared<AssetManagerPanel>();
		auto assetManagerWidget = m_assetManagerPanel->getWidget(&m_font);

		// ----- Setup Workspace And DockSpace -----
		auto workspace = Silica::MakeWidget<Silica::SWorkspace>({
			.initialTitle = "Hierarchy",
			.initialContent = hierarchyWidget,
			.font = &m_font
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
		auto menuBar = EditorMenuBar::construct(&m_font, m_dock);


		// ----- Assemble -----
		m_mainLayout = Silica::MakeWidget<Silica::SBorderLayout>({
			.topBar = menuBar,
			.contentArea = workspace
		});

		m_silicaRoot = Silica::MakeWidget<Silica::SBox>({
			.child = m_mainLayout
		});


		EditorModalManager::initialize(m_silicaRoot, m_mainLayout);
		SilicaContext::bindWndProcCallback(m_silicaRoot);
	}

	void EditorLayer::onDetach() {
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
			bool isHovering = m_viewportPanel->isHovered(Silica::Renderer::s_mousePosition);
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

			if (m_hierarchyPanel) m_hierarchyPanel->setScene(m_activeScene);
			if (m_assetManagerPanel) m_assetManagerPanel->refresh();

			return false;
		});

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

			// -- Setup ImGuizmo
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

	void EditorLayer::selectEntity(Entity selectedEntity) {
		EditorActionQueue::push([this, selectedEntity]() {
			m_selectedEntity = selectedEntity;

			if (m_propertiesPanel) {
				m_propertiesPanel->setEntity(selectedEntity);
			}
		});
	}

}
