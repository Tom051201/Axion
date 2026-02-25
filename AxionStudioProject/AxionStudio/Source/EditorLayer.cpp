#include "EditorLayer.h"

#include "AxionEngine/Vendor/ImGuizmo/ImGuizmo.h"

#include "AxionEngine/Source/events/Event.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/scene/SceneSerializer.h"
#include "AxionEngine/Source/scene/SceneManager.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionStudio/Source/core/EditorResourceManager.h"
#include "AxionStudio/Source/core/EditorStateSerializer.h"

// -- Windows only --
#if AX_WIN_USING_CUSTOM_TITLE_BAR
#include "AxionStudio/Source/platform/windows/WindowsTitleBar.h"
#endif

// TODO: REMOVE this
#include "AxionEngine/Source/scripting/NativeScripts.h"

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
		m_skyboxImportModal		= m_modalManager.addModal<SkyboxImportModal>("SkyboxImportModal");
		m_meshImportModal		= m_modalManager.addModal<MeshImportModal>("MeshImportModal");
		m_audioImportModal		= m_modalManager.addModal<AudioImportModal>("AudioImportModal");
		m_shaderImportModal		= m_modalManager.addModal<ShaderImportModal>("ShaderImportModal");
		m_materialImportModal	= m_modalManager.addModal<MaterialImportModal>("MaterialImportModal");
		m_tex2dImportModal		= m_modalManager.addModal<Texture2DImportModal>("Texture2DImportModal");
		m_pipelineImportModal	= m_modalManager.addModal<PipelineImportModal>("PipelineImportModal");


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
	}

	void EditorLayer::onDetach() {
		m_frameBuffer->release();

		EditorStateSerializer stateSerializer("AxionStudio/Config/State.yaml");
		stateSerializer.save(m_panelManager);
		m_panelManager.shutdownAll();

		EditorResourceManager::shutdown();
	}

	void EditorLayer::onUpdate(Timestep ts) {

		m_editorCamera.onUpdate(ts);

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

			m_frameBuffer->bind();
			m_frameBuffer->clear();

			switch (activeState) {
				case SceneState::Edit: {
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
						m_activeScene->onUpdateSimulation(ts, m_editorCamera);
					}
					else {
						m_activeScene->onUpdateSimulation(Timestep(0.0f), m_editorCamera);
					}
					break;
				}
				case SceneState::Pause: {
					break;
				}
				default: { break; }
			}

			m_frameBuffer->unbind();
		}

		Renderer::renderToSwapChain();
	}

	void EditorLayer::onEvent(Event& e) {
		m_editorCamera.onEvent(e);
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

		drawToolBar();

		drawSceneViewport();

		m_panelManager.renderAll();

		m_modalManager.renderAll();

		// -- New project popup --
		if (m_openNewProjectPopup) {
			ImGui::OpenPopup("Create New Project");
			m_openNewProjectPopup = false;
		}
		drawNewProjectWindow();

		drawMenuBar();

		endDockspace();
	}

	bool EditorLayer::onKeyPressed(KeyPressedEvent& e) {
		if (e.getKeyCode() == KeyCode::Tab) {
			if (m_editorCamera.is2D()) {
				m_editorCamera.set3D();
			}
			else {
				m_editorCamera.set2D();
			}
		}

		if (e.getKeyCode() == KeyCode::Enter) {
			Entity e = m_activeScene->createEntity();

			//Ref<Mesh> mesh = Mesh::createPBRCube();
			//AssetHandle<Mesh> meshHandle = AssetManager::insert(mesh);
			//e.addComponent<MeshComponent>(meshHandle);

			//AssetHandle<Material> matHandle = AssetManager::load<Material>(std::filesystem::absolute("AxionStudio/Projects/ExampleProject/Assets/materials/Green Metal.axmat").string());
			//Ref<Material> mat = AssetManager::get<Material>(matHandle);

			//e.addComponent<MaterialComponent>(matHandle);
			//e.addComponent<NativeScriptComponent>().bind<CameraController>();
			//e.addComponent<AudioComponent>();
			//AssetHandle<AudioClip> clip = AssetManager::load<AudioClip>(std::filesystem::absolute("AxionStudio/Projects/ExampleProject/Assets/audio/ping.axaudio").string());
			//e.getComponent<AudioComponent>().audio = std::make_shared<AudioSource>(clip);
			//e.getComponent<AudioComponent>().isSource = true;
			e.addComponent<CameraComponent>();

			//e.addComponent<DirectionalLightComponent>();
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
					std::string path = FileDialogs::saveFile({ {"Axion Scene", "*.axscene"} });
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
		m_activeScene = SceneManager::getScene();
		return false;
	}

	bool EditorLayer::onFileDrop(FileDropEvent& e) {
		if (e.getPaths().empty() || !ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) { // TODO: set cursor when not droppable // TODO: add function for this
			return false;
		}

		const auto& paths = e.getPaths();
		const auto& path = paths[0];
		std::string ext = path.extension().string();

		if (ext == ".obj") {
			m_meshImportModal->presetFromFile(path);
			m_meshImportModal->open();
		}
		else if (ext == ".mp3" || ext == ".wav") {
			m_audioImportModal->presetFromFile(path);
			m_audioImportModal->open();
		}
		else {
			AX_CORE_LOG_WARN("Unsupported dropped file: {}", path.string());
		}

		return true;
	}

	void EditorLayer::drawNewProjectWindow() {
		if (ImGui::BeginPopupModal("Create New Project", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			// ----- Name -----
			ImGui::InputText("Project Name", m_newNameBuffer, IM_ARRAYSIZE(m_newNameBuffer));

			// ----- Locatation -----
			ImGui::InputText("Location", m_newLocationBuffer, IM_ARRAYSIZE(m_newLocationBuffer));
			ImGui::SameLine();
			if (ImGui::Button("Browse...")) {
				std::string folder = FileDialogs::openFolder();
				if (!folder.empty()) {
					strcpy_s(m_newLocationBuffer, IM_ARRAYSIZE(m_newLocationBuffer), folder.c_str());
					m_newLocationBuffer[IM_ARRAYSIZE(m_newLocationBuffer) - 1] = '\0';
				}
			}

			ImGui::SeparatorText("Optional");

			ImGui::InputText("Author", m_newProjectAuthor, IM_ARRAYSIZE(m_newProjectAuthor));
			ImGui::InputText("Company", m_newProjectCompany, IM_ARRAYSIZE(m_newProjectCompany));
			ImGui::InputText("Description", m_newProjectDescription, IM_ARRAYSIZE(m_newProjectDescription));

			ImGui::Separator();


			// ----- Clearing values function lambda -----
			auto clearNewProjectsFields = [this]() {
				m_newNameBuffer[0] = '\0';
				m_newLocationBuffer[0] = '\0';
				m_newProjectAuthor[0] = '\0';
				m_newProjectCompany[0] = '\0';
				m_newProjectDescription[0] = '\0';
			};


			// ----- Validate input -----
			std::filesystem::path locPath(m_newLocationBuffer);
			bool validLocation = std::filesystem::is_directory(locPath);
			std::filesystem::path projecFolder = locPath / m_newNameBuffer;
			bool invalidName = std::filesystem::exists(projecFolder);

			bool disabled = (strlen(m_newNameBuffer) == 0 || strlen(m_newLocationBuffer) == 0 || !validLocation || invalidName);

			// ----- Create Project -----
			ImGui::BeginDisabled(disabled);
			if (ImGui::Button("Create Project")) {
				ProjectSpecification spec;
				spec.name = m_newNameBuffer;
				spec.location = m_newLocationBuffer;
				spec.author = m_newProjectAuthor;
				spec.company = m_newProjectCompany;
				spec.description = m_newProjectDescription;

				if (!spec.name.empty() && !spec.location.empty()) {
					ProjectManager::newProject(spec);
					ImGui::CloseCurrentPopup();
					clearNewProjectsFields();
				}
			}
			ImGui::EndDisabled();


			// ----- Cancel -----
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) {
				ImGui::CloseCurrentPopup();
				clearNewProjectsFields();
			}
			if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
				ImGui::CloseCurrentPopup();
				clearNewProjectsFields();
			}

			ImGui::EndPopup();
		}
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
					AX_CORE_LOG_WARN(path);
					// -- Try loading a skybox --
					if (path.find(".axsky") != std::string::npos) {
						std::string absPath = AssetManager::getAbsolute(path);
						AssetHandle<Skybox> handle = AssetManager::load<Skybox>(absPath);
						SceneManager::getScene()->setSkybox(handle);
					}
				}
				ImGui::EndDragDropTarget();
			}

			// ----- Draw gizmo -----
			drawGizmo();
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
			Mat4 worldM = tc.getTransform();


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
				Mat4 updated = Mat4::fromXM(newM);
				TRSData trs = updated.decompose();

				tc.position = trs.translation;
				tc.rotation = trs.rotation;
				tc.scale = trs.scale;
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
					if (ImGui::MenuItem("Material")) { m_materialImportModal->open(); }
					if (ImGui::MenuItem("Skybox")) { m_skyboxImportModal->open(); }
					if (ImGui::MenuItem("Shader")) { m_shaderImportModal->open(); }
					if (ImGui::MenuItem("Pipeline")) { m_pipelineImportModal->open(); }
					if (ImGui::MenuItem("Audio")) { m_audioImportModal->open(); }

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
				ImGui::EndMenu();
			}


			// ----- Project menu -----
			if (ImGui::BeginMenu("  Project  ")) {
				if (ImGui::MenuItem("New...")) { m_openNewProjectPopup = true; }
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

		int numButtons = 4;
		float totalWidth = (size * numButtons) + (ImGui::GetStyle().ItemInnerSpacing.x * (numButtons - 1));
		ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (totalWidth * 0.5f));

		bool isEdit = m_sceneState == SceneState::Edit;
		bool isPlay = m_sceneState == SceneState::Play || (m_sceneState == SceneState::Pause && m_prePauseState == SceneState::Play);
		bool isSim = m_sceneState == SceneState::Simulate || (m_sceneState == SceneState::Pause && m_prePauseState == SceneState::Simulate);
		bool isPaused = m_sceneState == SceneState::Pause;


		// ----- Simulate Button -----
		ImGui::BeginDisabled(isPlay);
		Ref<Texture2D> simIcon = isSim ? EditorResourceManager::getIcon("StopButton") : EditorResourceManager::getIcon("SimulateButton");
		if (ImGui::ImageButton("sim_btn", (ImTextureID)GraphicsContext::get()->getImGuiTextureID(simIcon), { size, size }, { 0, 1 }, { 1, 0 })) {
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
		if (ImGui::ImageButton("play_btn", (ImTextureID)GraphicsContext::get()->getImGuiTextureID(playIcon), { size, size }, { 0, 1 }, { 1, 0 })) {
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
		if (ImGui::ImageButton("pause_btn", (ImTextureID)GraphicsContext::get()->getImGuiTextureID(pauseIcon), { size, size }, { 0, 1 }, { 1, 0 })) {
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
		if (ImGui::ImageButton("step_btn", (ImTextureID)GraphicsContext::get()->getImGuiTextureID(EditorResourceManager::getIcon("StepButton")), { size, size }, { 0, 1 }, { 1, 0 })) {
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
		serializer.serializeText(tempPath);

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
		serializer.serializeText(tempPath);

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

}
