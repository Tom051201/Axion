#include "EditorLayer.h"

#include "AxionEngine/Vendor/ImGuizmo/ImGuizmo.h"

#include "AxionEngine/Source/events/Event.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/scene/SceneSerializer.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionStudio/Source/core/EditorStateSerializer.h"

// -- Windows only --
#ifdef AX_PLATFORM_WINDOWS
#include "AxionEngine/Platform/windows/WindowsWindow.h"
#endif

namespace Axion {

	EditorLayer::EditorLayer() : Layer("AxionStudioLayer"), m_editorCamera(1280.0f / 720.0f) {}

	void EditorLayer::onAttach() {

		// ----- Set scene -----
		m_sceneState = SceneState::Editing;
		m_activeScene = std::make_shared<Scene>();


		// ----- Setup all panels -----
		m_systemInfoPanel		= m_panelManager.addPanel<SystemInfoPanel>("SystemInfoPanel");
		m_sceneHierarchyPanel	= m_panelManager.addPanel<SceneHierarchyPanel>("SceneHierarchyPanel", m_activeScene);
		m_editorCameraPanel		= m_panelManager.addPanel<EditorCameraPanel>("EditorCameraPanel", &m_editorCamera);
		m_contentBrowserPanel	= m_panelManager.addPanel<ContentBrowserPanel>("ContentBrowserPanel");
		m_projectPanel			= m_panelManager.addPanel<ProjectPanel>("ProjectPanel");
		m_panelManager.setupAll();


		// ----- Setup framebuffer for scene viewport -----
		FrameBufferSpecification fbs;
		fbs.width = 1280;
		fbs.height = 720;
		fbs.textureFormat = ColorFormat::RGBA8;
		fbs.depthStencilFormat = DepthStencilFormat::DEPTH32F;
		m_frameBuffer = FrameBuffer::create(fbs);
		m_viewportSize = { (float)fbs.width, (float)fbs.height };


		// ----- Setup dockspace -----
		m_dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_None;
		m_windowFlags = ImGuiWindowFlags_MenuBar |
			ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoNavFocus;


		// ----- Setup application -----
		Application::get().setWindowTitle("Axion Studio");
		Application::get().setWindowIcon("AxionStudio/Resources/logo.ico");


		// ----- Load editor state from file -----
		EditorStateSerializer stateSerializer("AxionStudio/Config/State.yaml");
		stateSerializer.load(m_panelManager);
	}

	void EditorLayer::onDetach() {
		m_frameBuffer->release();

		EditorStateSerializer stateSerializer("AxionStudio/Config/State.yaml");
		stateSerializer.save(m_panelManager);
		m_panelManager.shutdownAll();
	}

	void EditorLayer::onUpdate(Timestep ts) {

		m_editorCamera.onUpdate(ts);

		if (m_viewportSize.x > 0 && m_viewportSize.y > 0) {

			m_frameBuffer->bind();
			m_frameBuffer->clear();

			switch (m_sceneState) {
				case Axion::SceneState::Editing: {
					m_activeScene->onUpdate(ts, m_editorCamera);
					break;
				}
				case Axion::SceneState::Playing: {
					m_activeScene->onUpdate(ts);
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
	}

	void EditorLayer::onGuiRender() {
		beginDockspace();

		drawSceneViewport();
	
		// -- Draw all panels --
		m_panelManager.renderAll();

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
		// -> Shortcuts only from here on
		if (e.getRepeatCount() > 0) return false;
		bool controlPressed = Input::isKeyPressed(KeyCode::LeftControl) || Input::isKeyPressed(KeyCode::RightControl);
		bool shiftPressed = Input::isKeyPressed(KeyCode::LeftShift) || Input::isKeyPressed(KeyCode::RightShift);
		switch (e.getKeyCode()) {
			case KeyCode::N: {
				// -- Ctrl + N --
				if (controlPressed) { m_requests |= RequestFlags::NewScene; }
				break;
			}
			case KeyCode::O: {
				// -- Ctrl + O --
				if (controlPressed) { m_requests |= RequestFlags::OpenScene; }
				break;
			}
			case KeyCode::S: {
				// -- Ctrl + Shift + S --
				if (controlPressed && shiftPressed) { m_requests |= RequestFlags::SaveSceneAs; }
				
				// -- Ctrl + S
				else if (controlPressed && (!shiftPressed)) { m_requests |= RequestFlags::SaveScene; }
				break;
			}
			default: break;
		}

		return false;
	}

	bool EditorLayer::onRenderingFinished(RenderingFinishedEvent& e) {
		// ----- Handle requests -----
		handleSceneRequests();
		handleProjectRequests();


		// ----- Resize viewport -----
		if (m_viewportResized) {
			m_frameBuffer->resize((uint32_t)m_viewportSize.x, (uint32_t)m_viewportSize.y);
			m_editorCamera.resize(m_viewportSize.x, m_viewportSize.y);
			m_viewportResized = false;
		}


		clearFlags(m_requests);
		return false;
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

			// ----- Create Project -----
			if (ImGui::Button("Create Project")) {
				//std::string name(m_newNameBuffer);
				//std::string location(m_newLocationBuffer);
				//std::string author(m_newProjectAuthor);
				//std::string company(m_newProjectCompany);
				//std::string description(m_newProjectDescription);

				ProjectSpecification spec;
				spec.name = m_newNameBuffer;
				spec.location = m_newLocationBuffer;
				spec.author = m_newProjectAuthor;
				spec.company = m_newProjectCompany;
				spec.description = m_newProjectDescription;

				if (!spec.name.empty() && !spec.location.empty()) {
					ProjectManager::setActiveProject(Project::createNew(spec));
					AX_CORE_LOG_INFO("Created Project {} at {}", ProjectManager::getActiveProject()->getName(), ProjectManager::getActiveProject()->getProjectPath());
					ImGui::CloseCurrentPopup();
				}
			}


			// ----- Cancel on escape -----
			if (ImGui::IsKeyPressed(ImGuiKey_Escape)) { ImGui::CloseCurrentPopup(); }

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
		ImGui::Begin("Editor Viewport");
		m_editorCamera.setIsHoveringSceneViewport(ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem));
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
			ImGuizmo::SetOrthographic(false); // TODO: review, maybe not every frame needed
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
			if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) currentOp = ImGuizmo::TRANSLATE;
			if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) currentOp = ImGuizmo::ROTATE;
			if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) currentOp = ImGuizmo::SCALE;


			// ----- Do gizmo stuff -----
			ImGuizmo::Manipulate(
				cameraView.data(),
				cameraProjection.data(),
				currentOp,
				ImGuizmo::LOCAL,
				object
			);


			// ----- Apply changes -----
			if (ImGuizmo::IsUsing()) {
				DirectX::XMMATRIX newM = DirectX::XMLoadFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(object));
				Mat4 updated = Mat4::fromXM(newM);
				TRSData trs = updated.decompose();

				tc.position = trs.translation;
				tc.rotation = trs.rotationEuler;	// TODO: rotation from the gizmo does not work
				tc.scale = trs.scale;

				worldM = Mat4::TRS(tc.position, tc.rotation, tc.scale);
			}

		}
	}

	void EditorLayer::drawMenuBar() {
		if (ImGui::BeginMenuBar()) {
			ImGui::Text("      Axion Studio           ");

			// ----- File menu -----
			if (ImGui::BeginMenu("  File  ")) {
				if (ImGui::MenuItem("New Scene", "Ctrl+N")) { m_requests |= RequestFlags::NewScene; }
				if (ImGui::MenuItem("Open Scene...", "Ctrl+O")) { m_requests |= RequestFlags::OpenScene; }
				ImGui::Separator();
				if (ImGui::MenuItem("Save Scene", "Ctrl+S")) { m_requests |= RequestFlags::SaveScene; }
				if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S")) { m_requests |= RequestFlags::SaveSceneAs; }
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
				ImGui::MenuItem("Editor Camera Properties", nullptr, &m_editorCameraPanel->isVisible());
				ImGui::EndMenu();
			}


			// ----- Project menu -----
			if (ImGui::BeginMenu("  Project  ")) {
				if (ImGui::MenuItem("New...")) { m_openNewProjectPopup = true; }
				if (ImGui::MenuItem("Open...")) { m_requests |= RequestFlags::OpenProject; }
				if (ImGui::MenuItem("Save")) { m_requests |= RequestFlags::SaveProject; }
				ImGui::EndMenu();
			}


			// ----- Help menu -----
			if (ImGui::BeginMenu("  Help  ")) {
				ImGui::MenuItem("System Info", nullptr, &m_systemInfoPanel->isVisible());
				ImGui::EndMenu();
			}


			// ----- WIN32 custom title bar -----
			#if AX_WIN_USING_CUSTOM_TITLE_BAR
			drawCustomTitleBarWin32();
			#endif

			ImGui::EndMenuBar();

		}
	}

	void EditorLayer::handleSceneRequests() {
		// ----- New Scene -----
		if (hasFlag(m_requests, RequestFlags::NewScene)) {
			m_activeScene = std::make_shared<Scene>();
			m_sceneHierarchyPanel->setContext(m_activeScene);
			m_activeSceneFilePath.clear();
		}


		// ----- Open Scene -----
		if (hasFlag(m_requests, RequestFlags::OpenScene)) {
			std::string path = FileDialogs::openFile({ {"Axion Scene", "*.axion"} });
			if (!path.empty()) {
				m_activeScene = std::make_shared<Scene>();
				m_sceneHierarchyPanel->setContext(m_activeScene);
				SceneSerializer serializer(m_activeScene);
				serializer.deserializeText(path);
				m_activeSceneFilePath = path;
			}
		}


		// ----- Save Scene -----
		if (hasFlag(m_requests, RequestFlags::SaveScene)) {
			if (!m_activeSceneFilePath.empty()) {
				SceneSerializer serializer(m_activeScene);
				serializer.serializeText(m_activeSceneFilePath);
			}
		}


		// ----- Save Scene As -----
		if (hasFlag(m_requests, RequestFlags::SaveSceneAs)) {
			std::string filePath = FileDialogs::saveFile({ {"Axion Scene", "*.axion"} });
			if (!filePath.empty()) {
				SceneSerializer serializer(m_activeScene);
				serializer.serializeText(filePath);
			}
		}

	}

	void EditorLayer::handleProjectRequests() {
		// ----- Open Project -----
		if (hasFlag(m_requests, RequestFlags::OpenProject)) {
			std::string filePath = FileDialogs::openFile({ {"Axion Project", "*.axproj"} });
			if (!filePath.empty()) {
				if (!m_activeProjectFilePath.empty()) ProjectManager::getActiveProject()->save(m_activeProjectFilePath);

				ProjectManager::setActiveProject(Project::load(filePath));
				m_activeProjectFilePath = ProjectManager::getActiveProject()->getProjectPath();
				AX_CORE_LOG_TRACE("Loaded project {}", ProjectManager::getActiveProject()->getName());
			}
		}


		// ----- Save Project -----
		if (hasFlag(m_requests, RequestFlags::SaveProject)) {
			if (ProjectManager::hasActiveProject()) {
				Ref<Project> project = ProjectManager::getActiveProject();
				std::string savePath = m_activeProjectFilePath;
				if (savePath.empty()) {
					savePath = FileDialogs::saveFile({ {"Axion Project", "*.axproj"} });
				}

				if (!savePath.empty()) {
					project->save(savePath);
					m_activeProjectFilePath = savePath;
					AX_CORE_LOG_TRACE("Saved Project {}", project->getName());
				}
			}
			else {
				AX_CORE_LOG_WARN("No active project to save");
			}
		}
	}

	#if AX_WIN_USING_CUSTOM_TITLE_BAR
	void EditorLayer::drawCustomTitleBarWin32() {
		m_lastTitleBarMenuX = ImGui::GetCursorPosX();

		// ----- Detect dragging on empty space in the menu bar -----
		static bool draggingWindow = false;
		ImVec2 winPos = ImGui::GetWindowPos();
		ImVec2 cursor = ImGui::GetMousePos();
		ImVec2 local = ImVec2(cursor.x - winPos.x, cursor.y - winPos.y);
		float menuBarHeight = ImGui::GetFrameHeight();
		float buttonWidth = 42.0f;
		float buttonHeight = 24.0f;
		float buttonSpacing = 4.0f;
		float totalButtonWidth = 3 * buttonWidth + 2 * buttonSpacing;
		float dragZoneStartX = m_lastTitleBarMenuX;
		float dragZoneEndX = ImGui::GetWindowWidth() - totalButtonWidth;


		// ----- Callback for the WindowsWindow -----
		if (local.y >= 0 && local.y <= menuBarHeight &&
			local.x >= dragZoneStartX && local.x <= dragZoneEndX) {
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
				draggingWindow = true;
				HWND hwnd = static_cast<HWND>(Application::get().getWindow().getNativeHandle());
				WindowsWindow& win = reinterpret_cast<WindowsWindow&>(Application::get().getWindow());
				win.isDragZone = [menuBarHeight, dragZoneStartX, dragZoneEndX](int x, int y) {
					return y >= 0 && y <= menuBarHeight && x >= dragZoneStartX && x <= dragZoneEndX;
				};
			}
		}
		if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
			draggingWindow = false;
		}


		// ----- Set custom icons for buttons -----
		ImGui::SameLine(ImGui::GetWindowWidth() - 134);
		if (ImGui::Button(u8"\uE15B", { buttonWidth, buttonHeight })) { Application::get().minimizeWindow(); }
		ImGui::SameLine();
		if (ImGui::Button(u8"\uE5D1", { buttonWidth, buttonHeight })) { Application::get().maximizeOrRestoreWindow(); }
		ImGui::SameLine();
		if (ImGui::Button(u8"\uE5CD", { buttonWidth, buttonHeight })) { Application::get().close(); }
	}
	#endif

}
