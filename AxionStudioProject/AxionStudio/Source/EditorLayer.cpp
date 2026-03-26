#include "EditorLayer.h"

#include "AxionEngine/Vendor/ImGuizmo/ImGuizmo.h"

#include "AxionEngine/Source/events/Event.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/scene/SceneSerializer.h"
#include "AxionEngine/Source/scene/SceneManager.h"
#include "AxionEngine/Source/scene/Prefab.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionStudio/Source/core/EditorResourceManager.h"
#include "AxionStudio/Source/core/EditorStateSerializer.h"
#include "AxionStudio/Source/core/EditorTheme.h"

#include "AxionAssetPipeline/Source/core/AssetPackager.h"

// -- Windows only --
#if AX_WIN_USING_CUSTOM_TITLE_BAR
#include "AxionStudio/Source/platform/windows/WindowsTitleBar.h"
#endif

namespace Axion {

	EditorLayer::EditorLayer() : Layer("AxionStudioLayer"), m_editorCamera(1280, 720) {}

	void EditorLayer::onAttach() {

		EditorResourceManager::initialize();

		// ----- Set scene -----
		m_sceneState = SceneState::Edit;
		m_activeScene = SceneManager::getScene();

		// ----- Setup all panels -----
		m_systemInfoPanel		= m_panelManager.addPanel<SystemInfoPanel>("SystemInfoPanel");
		m_sceneHierarchyPanel	= m_panelManager.addPanel<SceneHierarchyPanel>("SceneHierarchyPanel", SceneManager::getScene());
		m_editorCameraPanel		= m_panelManager.addPanel<EditorCameraPanel>("EditorCameraPanel", &m_editorCamera);
		m_contentBrowserPanel	= m_panelManager.addPanel<ContentBrowserPanel>("ContentBrowserPanel");
		m_projectPanel			= m_panelManager.addPanel<ProjectPanel>("ProjectPanel");
		m_sceneOverviewPanel	= m_panelManager.addPanel<SceneOverviewPanel>("SceneOverviewPanel");
		m_assetManagerPanel		= m_panelManager.addPanel<AssetManagerPanel>("AssetManagerPanel");
		m_panelManager.setupAll();


		// ----- Setup all modals -----
		m_skyboxImportModal				= m_modalManager.addModal<SkyboxImportModal>("SkyboxImportModal");
		m_meshImportModal				= m_modalManager.addModal<MeshImportModal>("MeshImportModal");
		m_audioImportModal				= m_modalManager.addModal<AudioImportModal>("AudioImportModal");
		m_shaderImportModal				= m_modalManager.addModal<ShaderImportModal>("ShaderImportModal");
		m_materialImportModal			= m_modalManager.addModal<MaterialImportModal>("MaterialImportModal");
		m_tex2dImportModal				= m_modalManager.addModal<Texture2DImportModal>("Texture2DImportModal");
		m_pipelineImportModal			= m_modalManager.addModal<PipelineImportModal>("PipelineImportModal");
		m_physicsMaterialImportModal	= m_modalManager.addModal<PhysicsMaterialImportModal>("PhysicsMaterialImportModal");
		m_textureCubeImportModal		= m_modalManager.addModal<TextureCubeImportModal>("TextureCubeImportModal");
		m_createProjectModal			= m_modalManager.addModal<CreateProjectModal>("CreateProjectModal");
		m_exportProjectModal			= m_modalManager.addModal<ExportProjectModal>("ExportProjectModal");


		// ----- Setup framebuffer for scene viewport -----
		FrameBufferSpecification fbs;
		fbs.width = 1280;
		fbs.height = 720;
		fbs.textureFormat = ColorFormat::RGBA8;
		fbs.depthStencilFormat = DepthStencilFormat::DEPTH32F;
		m_frameBuffer = FrameBuffer::create(fbs);
		m_viewportSize = { (float)fbs.width, (float)fbs.height };


		// ----- Setup imgui -----
		m_dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_None;
		m_windowFlags = ImGuiWindowFlags_MenuBar |
			ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoNavFocus;
		ImGuizmo::SetOrthographic(false);


		// ----- Load editor state from file -----
		EditorStateSerializer stateSerializer("AxionStudio/Config/State.yaml");
		stateSerializer.load(m_panelManager);


		// ----- Load icons -----
		EditorResourceManager::loadIcon("PlayButton", "AxionStudio/Resources/toolbar/PlayIcon.png");
		EditorResourceManager::loadIcon("StopButton", "AxionStudio/Resources/toolbar/StopIcon.png");
		EditorResourceManager::loadIcon("PauseButton", "AxionStudio/Resources/toolbar/PauseIcon.png");
		EditorResourceManager::loadIcon("SimulateButton", "AxionStudio/Resources/toolbar/SimulateIcon.png");
		EditorResourceManager::loadIcon("StepButton", "AxionStudio/Resources/toolbar/StepIcon.png");
		EditorResourceManager::loadIcon("CameraIcon", "AxionStudio/Resources/CameraIcon.png");
		EditorResourceManager::loadIcon("LightIcon", "AxionStudio/Resources/LightIcon.png");
		EditorResourceManager::loadIcon("2DCamIcon", "AxionStudio/Resources/toolbar/2dIcon.png");
		EditorResourceManager::loadIcon("3DCamIcon", "AxionStudio/Resources/toolbar/3dIcon.png");
	}

	void EditorLayer::onDetach() {
		m_frameBuffer->release();

		EditorStateSerializer stateSerializer("AxionStudio/Config/State.yaml");
		stateSerializer.save(m_panelManager);
		m_panelManager.shutdownAll();

		EditorResourceManager::shutdown();
	}

	void EditorLayer::onUpdate(Timestep ts) {

		bool processLogic = true;
		if (m_sceneState == SceneState::Pause) {
			if (m_stepFrames > 0) {
				m_stepFrames--;
			}
			else {
				processLogic = false;
			}
		}

		SceneState activeState = m_sceneState;
		if (m_sceneState == SceneState::Pause) {
			activeState = m_prePauseState;
		}

		if (m_viewportSize.x > 0 && m_viewportSize.y > 0) {

			Renderer::setRenderTarget(m_frameBuffer.get());
			m_frameBuffer->bind();
			m_frameBuffer->clear();

			switch (activeState) {
				case SceneState::Edit: {
					m_editorCamera.onUpdate(ts);
					m_activeScene->onUpdate(ts, m_editorCamera);
					break;
				}
				case SceneState::Play: {
					if (processLogic) {
						m_activeScene->onUpdate(ts);
					}
					else {
						m_activeScene->onUpdate(Timestep(0.0f));
					}
					break;
				}
				case SceneState::Simulate: {
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
				case SceneState::Pause: {
					break;
				}
				default: { break; }
			}

			// -- Overlays --
			Renderer2D::beginScene(m_editorCamera);
			if (m_sceneState == SceneState::Edit) drawOverlay();
			Renderer2D::endScene();

			m_frameBuffer->unbind();
		}

		Renderer::renderToSwapChain();
	}

	void EditorLayer::onEvent(Event& e) {
		if (m_sceneState == SceneState::Edit || m_sceneState == SceneState::Simulate) m_editorCamera.onEvent(e);
		m_activeScene->onEvent(e);
		m_panelManager.onEventAll(e);

		EventDispatcher dispatcher(e);
		dispatcher.dispatch<KeyPressedEvent>(AX_BIND_EVENT_FN(EditorLayer::onKeyPressed));
		dispatcher.dispatch<RenderingFinishedEvent>(AX_BIND_EVENT_FN(EditorLayer::onRenderingFinished));
		dispatcher.dispatch<SceneChangedEvent>(AX_BIND_EVENT_FN(EditorLayer::onSceneChanged));
		dispatcher.dispatch<FileDropEvent>(AX_BIND_EVENT_FN(EditorLayer::onFileDrop));
	}

	void EditorLayer::onGuiRender() {
		beginDockspace();

		processPendingDroppedFiles();

		drawToolBar();

		drawSceneViewport();

		m_panelManager.renderAll();

		m_modalManager.renderAll();

		drawMenuBar();

		endDockspace();
	}

	bool EditorLayer::onKeyPressed(KeyPressedEvent& e) {
		if (m_sceneState == SceneState::Edit && e.getKeyCode() == KeyCode::Tab) {
			if (m_editorCamera.is2D()) {
				m_editorCamera.set3D();
			}
			else {
				m_editorCamera.set2D();
			}
		}

		// -> Shortcuts only from here on
		if (e.getRepeatCount() > 0) return false;
		bool controlPressed = Input::isKeyPressed(KeyCode::LeftControl) || Input::isKeyPressed(KeyCode::RightControl);
		bool shiftPressed = Input::isKeyPressed(KeyCode::LeftShift) || Input::isKeyPressed(KeyCode::RightShift);
		bool altPressed = Input::isKeyPressed(KeyCode::LeftAlt) || Input::isKeyPressed(KeyCode::RightAlt);

		switch (e.getKeyCode()) {
			case KeyCode::N: {
				// -- Ctrl + N (New Scene) --
				if (controlPressed) {
					SceneManager::newScene();
				}
				break;
			}
			case KeyCode::O: {
				// -- Ctrl + O (Open Scene) --
				if (controlPressed) {
					std::string path = FileDialogs::openFile({ {"Axion Scene", "*.axscene"} });
					if (!path.empty()) SceneManager::loadScene(path);
				}
				break;
			}
			case KeyCode::S: {
				// -- Ctrl + Shift + S (Save As) --
				if (controlPressed && shiftPressed) {
					std::string defaultPath = ProjectManager::getProject()->getAssetsPath();
					std::string path = FileDialogs::saveFile({ {"Axion Scene", "*.axscene"} }, defaultPath);
					if (!path.empty()) SceneManager::saveScene(path);
				}

				// -- Ctrl + S (Save) --
				else if (controlPressed && (!shiftPressed)) {
					std::string path = SceneManager::getScenePath();
					if (!path.empty()) SceneManager::saveScene(path);
				}

				// -- Alt + S (Simulate / Stop Simulate) --
				else if (altPressed && !controlPressed && !shiftPressed) {
					if (m_sceneState == SceneState::Edit) {
						onSceneSimulate();
					}
					else {
						onSceneStop();
					}
				}
				break;
			}
			case KeyCode::P: {
				// -- Ctrl + Shift + P (Pause / Resume) --
				if (controlPressed && shiftPressed) {
					if (m_sceneState == SceneState::Play || m_sceneState == SceneState::Simulate) {
						m_prePauseState = m_sceneState;
						m_sceneState = SceneState::Pause;
					}
					else if (m_sceneState == SceneState::Pause) {
						m_sceneState = m_prePauseState;
					}
				}

				// -- Ctrl + P (Play / Stop) --
				else if (controlPressed && !shiftPressed && !altPressed) {
					if (m_sceneState == SceneState::Edit) {
						onScenePlay();
					}
					else if (m_sceneState == SceneState::Play || m_prePauseState == SceneState::Play) {
						onSceneStop();
					}
				}
				break;
			}
			case KeyCode::F10: {
				// -- F10 (Step Frame) --
				if (m_sceneState == SceneState::Pause) {
					m_stepFrames = 1;
				}
				break;
			}
			case KeyCode::Escape: {
				// -- Escape (Emergency Stop) --
				if (m_sceneState != SceneState::Edit) {
					onSceneStop();
				}
				break;
			}

			default: break;
		}

		return false;
	}

	bool EditorLayer::onRenderingFinished(RenderingFinishedEvent& e) {

		// ----- Resize viewport -----
		if (m_viewportResized) {
			m_frameBuffer->resize((uint32_t)m_viewportSize.x, (uint32_t)m_viewportSize.y);
			m_editorCamera.resize((uint32_t)m_viewportSize.x, (uint32_t)m_viewportSize.y);
			m_viewportResized = false;
		}

		return false;
	}

	bool EditorLayer::onSceneChanged(SceneChangedEvent& e) {
		if (m_sceneState != SceneState::Edit) {
			onSceneStop();
		}
		m_sceneState = SceneState::Edit;
		m_activeScene = SceneManager::getScene();
		return false;
	}

	bool EditorLayer::onFileDrop(FileDropEvent& e) {
		if (e.getPaths().empty()) {
			AX_CORE_LOG_WARN("File drop event did not contain any paths!");
			return false;
		}

		m_pendingDropPath = e.getPaths()[0];

		return true;
	}

	void EditorLayer::beginDockspace() {
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGui::Begin("DockSpaceFrame", nullptr, m_windowFlags);
		ImGui::PopStyleVar(3);

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), m_dockspaceFlags);
		}
	}

	void EditorLayer::endDockspace() {
		ImGui::End();
	}

	void EditorLayer::drawSceneViewport() {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });
		ImGui::Begin("Editor Viewport", nullptr, ImGuiWindowFlags_NoScrollbar);
		m_editorCamera.setHoveringSceneViewport(ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem));
		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

		if (viewportPanelSize.x > 0 && viewportPanelSize.y > 0) {
			// ----- Check if the size of the viewport has changed -----
			if (m_viewportSize.x != viewportPanelSize.x || m_viewportSize.y != viewportPanelSize.y) {
				m_viewportSize = { viewportPanelSize.x, viewportPanelSize.y };
				m_viewportResized = true;
			}


			// ----- Render framebuffer -----
			ImGui::Image(
				reinterpret_cast<ImTextureID>(m_frameBuffer->getColorAttachmentHandle()),
				ImVec2((float)m_frameBuffer->getSpecification().width, (float)m_frameBuffer->getSpecification().height)
			);


			// ----- Drag drop target -----
			if (ImGui::BeginDragDropTarget()) {

				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
					std::string path = static_cast<const char*>(payload->Data);
					AX_CORE_LOG_TRACE("Dropped: {}", path);
					// -- Try loading a skybox --
					if (path.find(".axsky") != std::string::npos) {
						std::string absPath = AssetManager::getAbsolute(path);

						UUID assetUUID = AssetManager::getAssetUUID(absPath);
						AssetHandle<Skybox> handle = AssetManager::load<Skybox>(assetUUID);
						SceneManager::getScene()->setSkybox(handle);
					}
					// -- Try adding prefab --
					if (path.find(".axprefab") != std::string::npos) {
						std::string absPath = AssetManager::getAbsolute(path);
						UUID assetUUID = AssetManager::getAssetUUID(absPath);
						if (assetUUID.isValid()) {
							AssetHandle<Prefab> handle = AssetManager::load<Prefab>(assetUUID);
							Ref<Prefab> prefab = AssetManager::get<Prefab>(handle);
							if (prefab) {
								YAML::Node node = prefab->getEntityNode();
								Entity newEntity = SceneSerializer::deserializeEntityNode(m_activeScene.get(), node, true);
								if (newEntity) {
									auto& transform = newEntity.getComponent<TransformComponent>();
									transform.position = m_editorCamera.getPosition();
								}
							}
							else {
								AX_CORE_LOG_WARN("Failed to load prefab!");
							}
						}
					}
				}
				ImGui::EndDragDropTarget();
			}

			// ----- Draw gizmo -----
			if (m_sceneState == SceneState::Edit) drawGizmo();
		}
		ImGui::End();
		ImGui::PopStyleVar();
	}

	void EditorLayer::drawGizmo() {
		Entity selectedEntity = m_sceneHierarchyPanel->getSelectedEntity();
		if (selectedEntity) {
			ImGuizmo::SetDrawlist();
			float windowWidth = (float)ImGui::GetWindowWidth();
			float windowHeight = (float)ImGui::GetWindowHeight();
			ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);


			// ----- Camera -----
			const Mat4& cameraView = m_editorCamera.getViewMatrix();
			const Mat4& cameraProjection = m_editorCamera.getProjectionMatrix();

			// ----- Entity transform -----
			auto& tc = selectedEntity.getComponent<TransformComponent>();
			Mat4 worldM = m_activeScene->getWorldTransform(selectedEntity);


			// ----- To float[16] for ImGuizmo -----
			DirectX::XMFLOAT4X4 objF4;
			DirectX::XMStoreFloat4x4(&objF4, worldM.toXM());
			float object[16];
			memcpy(object, &objF4, sizeof(objF4));


			// ----- Set Translate / Rotate / Scale -----
			static ImGuizmo::OPERATION currentOp = ImGuizmo::TRANSLATE;
			if (!ImGui::IsAnyItemActive() && !Input::isMouseButtonPressed(MouseButton::Right)) {
				if (ImGui::IsKeyPressed(ImGuiKey_W)) currentOp = ImGuizmo::TRANSLATE;
				if (ImGui::IsKeyPressed(ImGuiKey_E)) currentOp = ImGuizmo::ROTATE;
				if (ImGui::IsKeyPressed(ImGuiKey_R)) currentOp = ImGuizmo::SCALE;
			}

			// ----- Snapping -----
			bool snap = Input::isKeyPressed(KeyCode::LeftControl);
			float snapValue = 0.5f;
			if (currentOp == ImGuizmo::ROTATE) snapValue = 45.0f;
			float snapValues[3] = { snapValue, snapValue, snapValue };

			// ----- Do gizmo stuff -----
			ImGuizmo::Manipulate(
				cameraView.data(),
				cameraProjection.data(),
				currentOp,
				ImGuizmo::LOCAL,
				object,
				nullptr,
				snap ? snapValues : nullptr
			);


			// ----- Apply changes -----
			if (ImGuizmo::IsUsing()) {
				DirectX::XMMATRIX newM = DirectX::XMLoadFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(object));
				Mat4 updatedWorld = Mat4::fromXM(newM);

				Entity parent = selectedEntity.getParent();
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

		}
	}

	void EditorLayer::drawMenuBar() {
		if (ImGui::BeginMenuBar()) {
			ImGui::Text("      Axion Studio           ");

			// ----- File menu -----
			if (ImGui::BeginMenu("  File  ")) {
				if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
					SceneManager::newScene();
				}
				if (ImGui::MenuItem("Open Scene...", "Ctrl+O")) {
					std::string path = FileDialogs::openFile({ {"Axion Scene", "*.axscene"} });
					if (!path.empty()) SceneManager::loadScene(path);
				}
				ImGui::Separator();
				ImGui::BeginDisabled(SceneManager::isNewScene());
				if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
					std::string path = SceneManager::getScenePath();
					if (!path.empty()) SceneManager::saveScene(path);
				}
				ImGui::EndDisabled();
				if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S")) {
					std::string path = FileDialogs::saveFile({ {"Axion Scene", "*.axscene"} });
					if (!path.empty()) {
						SceneManager::saveScene(path);
						m_contentBrowserPanel->refresh();
					}
				}

				ImGui::Separator();
				if (ImGui::BeginMenu("Import##_menu")) {
					if (ImGui::MenuItem("Mesh")) { m_meshImportModal->open(); }
					if (ImGui::MenuItem("2D Texture")) { m_tex2dImportModal->open(); }
					if (ImGui::MenuItem("Cube Texture")) { m_textureCubeImportModal->open(); }
					if (ImGui::MenuItem("Material")) { m_materialImportModal->open(); }
					if (ImGui::MenuItem("Skybox")) { m_skyboxImportModal->open(); }
					if (ImGui::MenuItem("Shader")) { m_shaderImportModal->open(); }
					if (ImGui::MenuItem("Pipeline")) { m_pipelineImportModal->open(); }
					if (ImGui::MenuItem("Audio")) { m_audioImportModal->open(); }
					if (ImGui::MenuItem("Physics Material")) { m_physicsMaterialImportModal->open(); }

					ImGui::EndMenu();
				}

				ImGui::Separator();
				if (ImGui::MenuItem("Exit")) { Application::get().close(); }
				ImGui::EndMenu();
			}


			// ----- Edit menu -----
			if (ImGui::BeginMenu("  Edit  ")) {
				ImGui::EndMenu();
			}


			// ----- View menu -----
			if (ImGui::BeginMenu("  View  ")) {
				ImGui::MenuItem("Scene Hierarchy", nullptr, &m_sceneHierarchyPanel->isVisible());
				ImGui::MenuItem("Content Browser", nullptr, &m_contentBrowserPanel->isVisible());
				ImGui::MenuItem("Project Overview", nullptr, &m_projectPanel->isVisible());
				ImGui::MenuItem("Scene Overview", nullptr, &m_sceneOverviewPanel->isVisible());
				ImGui::MenuItem("Editor Camera Properties", nullptr, &m_editorCameraPanel->isVisible());
				ImGui::Separator();
				if (ImGui::BeginMenu("Theme")) {
					EditorTheme::Theme currentTheme = EditorTheme::loadTheme("AxionStudio/Config/Settings.yaml");

					if (ImGui::MenuItem("RED BLACK (Release I)", nullptr, currentTheme == EditorTheme::Theme::RedBlack)) {
						EditorTheme::setTheme(EditorTheme::Theme::RedBlack);
						EditorTheme::saveTheme(EditorTheme::Theme::RedBlack, "AxionStudio/Config/Settings.yaml");
					}

					if (ImGui::MenuItem("PURPLE ACCENTS (Release II)", nullptr, currentTheme == EditorTheme::Theme::Purple)) {
						EditorTheme::setTheme(EditorTheme::Theme::Purple);
						EditorTheme::saveTheme(EditorTheme::Theme::Purple, "AxionStudio/Config/Settings.yaml");
					}

					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}


			// ----- Project menu -----
			if (ImGui::BeginMenu("  Project  ")) {
				if (ImGui::MenuItem("New...")) {
					m_createProjectModal->open();
				}
				if (ImGui::MenuItem("Open...")) {
					std::string filePath = FileDialogs::openFile({ {"Axion Project", "*.axproj"} });
					if (!filePath.empty()) ProjectManager::loadProject(filePath);
				}
				if (ImGui::MenuItem("Save")) {
					std::string filePath = FileDialogs::saveFile({ {"Axion Project", "*.axproj"} });
					if (!filePath.empty()) ProjectManager::saveProject(filePath);
				}
				if (ImGui::MenuItem("Close")) {
					ProjectManager::unloadProject();
					SceneManager::newScene();
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Export")) {
					m_exportProjectModal->open();
				}
				ImGui::EndMenu();
			}


			// ----- Help menu -----
			if (ImGui::BeginMenu("  Help  ")) {
				ImGui::MenuItem("System Info", nullptr, &m_systemInfoPanel->isVisible());
				ImGui::MenuItem("Asset Manager Inspector", nullptr, &m_assetManagerPanel->isVisible());
				ImGui::EndMenu();
			}


			// ----- WIN32 custom title bar -----
			#if AX_WIN_USING_CUSTOM_TITLE_BAR
			WindowsTitleBar::drawCustomTitleBar();
			#endif

			ImGui::EndMenuBar();

		}
	}

	void EditorLayer::drawToolBar() {

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		auto& colors = ImGui::GetStyle().Colors;
		const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
		const auto& buttonActive = colors[ImGuiCol_ButtonActive];
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

		ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar);
		float size = ImGui::GetContentRegionAvail().y - 4.0f;

		int numButtons = 5;
		float totalWidth = (size * numButtons) + (ImGui::GetStyle().ItemInnerSpacing.x * (numButtons - 1));
		ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (totalWidth * 0.5f));

		bool isEdit = m_sceneState == SceneState::Edit;
		bool isPlay = m_sceneState == SceneState::Play || (m_sceneState == SceneState::Pause && m_prePauseState == SceneState::Play);
		bool isSim = m_sceneState == SceneState::Simulate || (m_sceneState == SceneState::Pause && m_prePauseState == SceneState::Simulate);
		bool isPaused = m_sceneState == SceneState::Pause;

		// ----- 2D / 3D Toggle Button -----
		ImGui::BeginDisabled(!isEdit);
		Ref<Texture2D> camIcon = m_editorCamera.is2D() ? EditorResourceManager::getIcon("2DCamIcon") : EditorResourceManager::getIcon("3DCamIcon");
		if (ImGui::ImageButton("cam_btn", (ImTextureID)GraphicsContext::get()->getImGuiTextureID(camIcon), { size, size })) {
			if (m_editorCamera.is2D()) {
				m_editorCamera.set3D();
			}
			else {
				m_editorCamera.set2D();
			}
		}
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
			ImGui::SetTooltip("Toggle 2D/3D Editor Camera (Tab)");
		}
		ImGui::EndDisabled();
		ImGui::SameLine();

		// ----- Simulate Button -----
		ImGui::BeginDisabled(isPlay);
		Ref<Texture2D> simIcon = isSim ? EditorResourceManager::getIcon("StopButton") : EditorResourceManager::getIcon("SimulateButton");
		if (ImGui::ImageButton("sim_btn", (ImTextureID)GraphicsContext::get()->getImGuiTextureID(simIcon), { size, size })) {
			if (isSim) {
				onSceneStop();
			}
			else {
				onSceneSimulate();
			}
		}
		ImGui::EndDisabled();
		ImGui::SameLine();


		// ----- Play Button -----
		ImGui::BeginDisabled(isSim);
		Ref<Texture2D> playIcon = isPlay ? EditorResourceManager::getIcon("StopButton") : EditorResourceManager::getIcon("PlayButton");
		if (ImGui::ImageButton("play_btn", (ImTextureID)GraphicsContext::get()->getImGuiTextureID(playIcon), { size, size })) {
			if (isPlay) {
				onSceneStop();
			}
			else {
				onScenePlay();
			}
		}
		ImGui::EndDisabled();
		ImGui::SameLine();


		// ----- Pause Button -----
		ImGui::BeginDisabled(isEdit);
		Ref<Texture2D> pauseIcon = isPaused ? EditorResourceManager::getIcon("PlayButton") : EditorResourceManager::getIcon("PauseButton");
		if (ImGui::ImageButton("pause_btn", (ImTextureID)GraphicsContext::get()->getImGuiTextureID(pauseIcon), { size, size })) {
			if (isPaused) {
				m_sceneState = m_prePauseState;
			}
			else {
				m_prePauseState = m_sceneState;
				m_sceneState = SceneState::Pause;
			}
		}
		ImGui::EndDisabled();
		ImGui::SameLine();


		// ----- Step Button -----
		ImGui::BeginDisabled(!isPaused);
		if (ImGui::ImageButton("step_btn", (ImTextureID)GraphicsContext::get()->getImGuiTextureID(EditorResourceManager::getIcon("StepButton")), { size, size })) {
			m_stepFrames = 1;
		}
		ImGui::EndDisabled();


		ImGui::End();
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);

	}

	void EditorLayer::onScenePlay() {
		m_sceneState = SceneState::Play;
		m_editorScene = m_activeScene;

		std::string tempPath = "AxionStudio/Config/TempScene.axscene";
		SceneSerializer serializer(m_editorScene);
		serializer.serializeText(tempPath, false);

		m_activeScene = std::make_shared<Scene>();
		SceneSerializer deserializer(m_activeScene);
		deserializer.deserializeText(tempPath);

		m_activeScene->onPhysicsStart();

		m_sceneHierarchyPanel->setContext(m_activeScene);

		m_activeScene->onViewportResized((uint32_t)m_viewportSize.x, (uint32_t)m_viewportSize.y);
	}

	void EditorLayer::onSceneSimulate() {
		m_sceneState = SceneState::Simulate;
		m_editorScene = m_activeScene;

		std::string tempPath = "AxionStudio/Config/TempScene.axscene";
		SceneSerializer serializer(m_editorScene);
		serializer.serializeText(tempPath, false);

		m_activeScene = std::make_shared<Scene>();
		SceneSerializer deserializer(m_activeScene);
		deserializer.deserializeText(tempPath);

		m_activeScene->onPhysicsStart();

		m_sceneHierarchyPanel->setContext(m_activeScene);

		m_activeScene->onViewportResized((uint32_t)m_viewportSize.x, (uint32_t)m_viewportSize.y);
	}

	void EditorLayer::onSceneStop() {
		m_sceneState = SceneState::Edit;
		m_prePauseState = SceneState::Edit;

		m_activeScene->onPhysicsStop();

		m_activeScene = m_editorScene;

		m_sceneHierarchyPanel->setContext(m_activeScene);
	}

	void EditorLayer::drawOverlay() {
		if (m_sceneState == SceneState::Pause) return;

		Entity selectedEntity = m_sceneHierarchyPanel->getSelectedEntity();

		if (selectedEntity) {

			// -- Draw Overlay for Box Collider --
			if (selectedEntity.hasComponent<BoxColliderComponent>()) {
				auto& bc = selectedEntity.getComponent<BoxColliderComponent>();

				Mat4 worldTransform = m_activeScene->getWorldTransform(selectedEntity);
				Vec3 worldScale = worldTransform.getScale();

				worldScale.x = std::abs(worldScale.x);
				worldScale.y = std::abs(worldScale.y);
				worldScale.z = std::abs(worldScale.z);

				Mat4 cleanWorldTransform = Mat4::TRS(worldTransform.getTranslation(), worldTransform.getRotation(), worldScale);
				Mat4 colliderTransform = cleanWorldTransform * Mat4::translation(bc.offset) * Mat4::scale(bc.halfExtents * 2.0f);

				Vec4 color = { 0.0f, 1.0f, 0.0f, 1.0f };
				if (bc.isTrigger) {
					color = { 1.0f, 0.57f, 0.0f, 1.0f };
				}

				drawWireframeBox(colliderTransform, color);
			}

			// -- Draw Overlay for Sphere Collider --
			if (selectedEntity.hasComponent<SphereColliderComponent>()) {
				auto& sc = selectedEntity.getComponent<SphereColliderComponent>();

				Mat4 worldTransform = m_activeScene->getWorldTransform(selectedEntity);
				Vec3 worldScale = worldTransform.getScale();

				float maxScale = std::max(std::abs(worldScale.x), std::max(std::abs(worldScale.y), std::abs(worldScale.z)));
				float radius = sc.radius * maxScale;

				Mat4 cleanWorldTransform = Mat4::TRS(worldTransform.getTranslation(), worldTransform.getRotation(), Vec3::one());
				Mat4 colliderTransform = cleanWorldTransform * Mat4::translation(sc.offset);

				Vec4 color = { 0.0f, 1.0f, 0.0f, 1.0f };
				if (sc.isTrigger) {
					color = { 1.0f, 0.57f, 0.0f, 1.0f };
				}

				drawWireframeSphere(colliderTransform, radius, color);
			}

			// -- Draw Overlay for Capsule Collider --
			if (selectedEntity.hasComponent<CapsuleColliderComponent>()) {
				auto& cc = selectedEntity.getComponent<CapsuleColliderComponent>();

				Mat4 worldTransform = m_activeScene->getWorldTransform(selectedEntity);
				Vec3 worldScale = worldTransform.getScale();

				float scaleXZ = std::max(std::abs(worldScale.x), std::abs(worldScale.z));
				float radius = cc.radius * scaleXZ;
				float halfHeight = cc.halfHeight * std::abs(worldScale.y);

				Mat4 cleanWorldTransform = Mat4::TRS(worldTransform.getTranslation(), worldTransform.getRotation(), Vec3::one());
				Mat4 colliderTransform = cleanWorldTransform * Mat4::translation(cc.offset);

				Vec4 color = { 0.0f, 1.0f, 0.0f, 1.0f };
				if (cc.isTrigger) {
					color = { 1.0f, 0.57f, 0.0f, 1.0f };
				}

				drawWireframeCapsule(colliderTransform, radius, halfHeight, color);
			}

		}

		Mat4 cameraViewMatrix = m_editorCamera.getViewMatrix();

		// -- Draw Camera Icons --
		auto cameraView = m_activeScene->getRegistry().view<CameraComponent>();
		for (auto [entity, camera] : cameraView.each()) {
			Vec3 position = m_activeScene->getWorldTransform({ entity, m_activeScene.get() }).getTranslation();
			Renderer2D::drawBillboard(position, Vec2::one(), cameraViewMatrix, EditorResourceManager::getIcon("CameraIcon"));
		}

		// -- Draw Directional Light Icons --
		auto dirLightView = m_activeScene->getRegistry().view<DirectionalLightComponent>();
		for (auto [entity, light] : dirLightView.each()) {
			Vec3 position = m_activeScene->getWorldTransform({ entity, m_activeScene.get() }).getTranslation();
			Renderer2D::drawBillboard(position, Vec2::one(), cameraViewMatrix, EditorResourceManager::getIcon("LightIcon"));
		}

		// -- Draw Point Light Icons --
		auto pointLightView = m_activeScene->getRegistry().view<PointLightComponent>();
		for (auto [entity, light] : pointLightView.each()) {
			Vec3 position = m_activeScene->getWorldTransform({ entity, m_activeScene.get() }).getTranslation();
			Renderer2D::drawBillboard(position, Vec2::one(), cameraViewMatrix, EditorResourceManager::getIcon("LightIcon"));
		}

		// -- Draw Spot Light Icons --
		auto spotLightView = m_activeScene->getRegistry().view<SpotLightComponent>();
		for (auto [entity, light] : spotLightView.each()) {
			Vec3 position = m_activeScene->getWorldTransform({ entity, m_activeScene.get() }).getTranslation();
			Renderer2D::drawBillboard(position, Vec2::one(), cameraViewMatrix, EditorResourceManager::getIcon("LightIcon"));
		}

	}

	void EditorLayer::drawWireframeBox(const Mat4& transform, const Vec4& color) {
		Vec3 corners[8] = {
			{ -0.5f, -0.5f, -0.5f }, {  0.5f, -0.5f, -0.5f },
			{  0.5f,  0.5f, -0.5f }, { -0.5f,  0.5f, -0.5f },
			{ -0.5f, -0.5f,  0.5f }, {  0.5f, -0.5f,  0.5f },
			{  0.5f,  0.5f,  0.5f }, { -0.5f,  0.5f,  0.5f }
		};

		for (int i = 0; i < 8; i++) {
			corners[i] = (transform * corners[i]);
		}

		// Bottom
		Renderer2D::drawLine(corners[0], corners[1], color);
		Renderer2D::drawLine(corners[1], corners[2], color);
		Renderer2D::drawLine(corners[2], corners[3], color);
		Renderer2D::drawLine(corners[3], corners[0], color);

		// Top
		Renderer2D::drawLine(corners[4], corners[5], color);
		Renderer2D::drawLine(corners[5], corners[6], color);
		Renderer2D::drawLine(corners[6], corners[7], color);
		Renderer2D::drawLine(corners[7], corners[4], color);

		// Sides
		Renderer2D::drawLine(corners[0], corners[4], color);
		Renderer2D::drawLine(corners[1], corners[5], color);
		Renderer2D::drawLine(corners[2], corners[6], color);
		Renderer2D::drawLine(corners[3], corners[7], color);

	}

	void EditorLayer::drawWireframeSphere(const Mat4& transform, float radius, const Vec4& color) {
		constexpr int segments = 24;

		static std::array<Vec3, segments> unitCircleXY;
		static std::array<Vec3, segments> unitCircleXZ;
		static std::array<Vec3, segments> unitCircleYZ;
		static bool initialized = false;

		if (!initialized) {
			float step = 360.0f / segments;
			for (int i = 0; i < segments; i++) {
				float angle = Math::toRadians(i * step);
				float c = std::cos(angle);
				float s = std::sin(angle);

				unitCircleXY[i] = { c, s, 0.0f };
				unitCircleXZ[i] = { c, 0.0f, s };
				unitCircleYZ[i] = { 0.0f, c, s };
			}
			initialized = true;
		}

		for (int i = 0; i < segments; i++) {
			int next = (i + 1) % segments;

			// -- XY Circle --
			Vec3 p1_xy = transform * (unitCircleXY[i] * radius);
			Vec3 p2_xy = transform * (unitCircleXY[next] * radius);
			Renderer2D::drawLine(p1_xy, p2_xy, color);

			// -- XZ Circle --
			Vec3 p1_xz = transform * (unitCircleXZ[i] * radius);
			Vec3 p2_xz = transform * (unitCircleXZ[next] * radius);
			Renderer2D::drawLine(p1_xz, p2_xz, color);

			// -- YZ Circle --
			Vec3 p1_yz = transform * (unitCircleYZ[i] * radius);
			Vec3 p2_yz = transform * (unitCircleYZ[next] * radius);
			Renderer2D::drawLine(p1_yz, p2_yz, color);
		}

	}

	void EditorLayer::drawWireframeCapsule(const Mat4& transform, float radius, float halfHeight, const Vec4& color) {
		int segments = 24;
		float step = 360.0f / segments;

		// -- Cylindrical body --
		for (int i = 0; i < segments; i++) {
			float angle1 = Math::toRadians(i * step);
			float angle2 = Math::toRadians((i + 1) * step);

			float sin1 = std::sin(angle1) * radius;
			float cos1 = std::cos(angle1) * radius;
			float sin2 = std::sin(angle2) * radius;
			float cos2 = std::cos(angle2) * radius;

			// Top XZ Ring
			Vec3 topRing1 = transform * Vec3(cos1, halfHeight, sin1);
			Vec3 topRing2 = transform * Vec3(cos2, halfHeight, sin2);
			Renderer2D::drawLine(topRing1, topRing2, color);

			// Bottom XZ Ring
			Vec3 bottomRing1 = transform * Vec3(cos1, -halfHeight, sin1);
			Vec3 bottomRing2 = transform * Vec3(cos2, -halfHeight, sin2);
			Renderer2D::drawLine(bottomRing1, bottomRing2, color);

			// Vertical connecting lines (draw 4 evenly spaced lines to represent the cylinder walls)
			if (i % (segments / 4) == 0) {
				Renderer2D::drawLine(topRing1, bottomRing1, color);
			}
		}

		// -- Hemispheres (Top and bottom) --
		int halfSegments = segments / 2;
		float halfStep = 180.0f / halfSegments;
		for (int i = 0; i < halfSegments; i++) {
			float angle1 = Math::toRadians(i * halfStep);
			float angle2 = Math::toRadians((i + 1) * halfStep);

			float cos1 = std::cos(angle1) * radius;
			float sin1 = std::sin(angle1) * radius;
			float cos2 = std::cos(angle2) * radius;
			float sin2 = std::sin(angle2) * radius;

			// Top Hemisphere
			Renderer2D::drawLine(transform * Vec3(cos1, sin1 + halfHeight, 0.0f), transform * Vec3(cos2, sin2 + halfHeight, 0.0f), color); // XY plane
			Renderer2D::drawLine(transform * Vec3(0.0f, sin1 + halfHeight, cos1), transform * Vec3(0.0f, sin2 + halfHeight, cos2), color); // YZ plane

			// Bottom Hemisphere
			Renderer2D::drawLine(transform * Vec3(cos1, -sin1 - halfHeight, 0.0f), transform * Vec3(cos2, -sin2 - halfHeight, 0.0f), color); // XY plane
			Renderer2D::drawLine(transform * Vec3(0.0f, -sin1 - halfHeight, cos1), transform * Vec3(0.0f, -sin2 - halfHeight, cos2), color); // YZ plane
		}

	}

	void EditorLayer::processPendingDroppedFiles() {
		if (!m_pendingDropPath.empty()) {
			std::string ext = m_pendingDropPath.extension().string();
			std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });

			if (ext == ".obj") {
				m_meshImportModal->presetFromFile(m_pendingDropPath);
				m_meshImportModal->open();
			}
			else if (ext == ".mp3" || ext == ".wav" || ext == ".ogg") {
				m_audioImportModal->presetFromFile(m_pendingDropPath);
				m_audioImportModal->open();
			}
			else {
				AX_CORE_LOG_WARN("Unsupported dropped file: {}", m_pendingDropPath.string());
			}

			m_pendingDropPath.clear();
		}
	}

}
