#include "EditorLayer.h"

#include "AxionEngine/Vendor/ImGuizmo/ImGuizmo.h"

#include "AxionEngine/Source/events/Event.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/scene/SceneSerializer.h"

namespace Axion {

	EditorLayer::EditorLayer() : Layer("AxionStudioLayer"), m_editorCamera(1280.0f / 720.0f) {}

	void EditorLayer::onAttach() {

		m_sceneState = SceneState::Editing;
		m_activeScene = std::make_shared<Scene>();
		
		//Ref<Skybox> sky = std::make_shared<Skybox>("AxionStudio/Assets/skybox/Daylight_Box_UV.png");
		//m_activeScene->setSkybox(sky);

		m_systemInfoPanel = std::make_unique<SystemInfoPanel>();
		m_systemInfoPanel->setup();
		m_sceneHierarchyPanel = std::make_unique<SceneHierarchyPanel>();
		m_sceneHierarchyPanel->setup(m_activeScene);
		m_editorCameraPanel = std::make_unique<EditorCameraPanel>();
		m_editorCameraPanel->setup(&m_editorCamera);
		m_contentBrowserPanel = std::make_unique<ContentBrowserPanel>();
		m_contentBrowserPanel->setup();
		m_projectPanel = std::make_unique<ProjectPanel>();
		m_projectPanel->setup();

		FrameBufferSpecification fbs;
		fbs.width = 1280;
		fbs.height = 720;
		fbs.textureFormat = ColorFormat::RGBA8;
		fbs.depthStencilFormat = DepthStencilFormat::DEPTH32F;
		m_frameBuffer = FrameBuffer::create(fbs);

		m_viewportDim = { (float)fbs.width, (float)fbs.height };

		m_dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_None;
		m_windowFlags = ImGuiWindowFlags_MenuBar |
			ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoNavFocus;

		Application::get().setWindowTitle("Axion Studio");
		Application::get().setWindowIcon("AxionStudio/Resources/logo.ico");

		m_activeProject = std::make_shared<Project>("Unknown");
		m_projectPanel->setProject(m_activeProject);
	}

	void EditorLayer::onDetach() {
		m_frameBuffer->release();

		m_systemInfoPanel->shutdown();
		m_sceneHierarchyPanel->shutdown();
		m_editorCameraPanel->shutdown();
		m_contentBrowserPanel->shutdown();
		m_projectPanel->shutdown();
	}

	void EditorLayer::onUpdate(Timestep ts) {

		m_editorCamera.onUpdate(ts);

		if (m_viewportDim.x > 0 && m_viewportDim.y > 0) {

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
		
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<KeyPressedEvent>(AX_BIND_EVENT_FN(EditorLayer::onKeyPressed));
		dispatcher.dispatch<RenderingFinishedEvent>(AX_BIND_EVENT_FN(EditorLayer::onRenderingFinished));
	}

	void EditorLayer::onGuiRender() {
		
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

		// renders framebuffer
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });
		ImGui::Begin("Editor Viewport");
		m_editorCamera.setIsHoveringSceneViewport(ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem));
		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		
		if (viewportPanelSize.x > 0 && viewportPanelSize.y > 0) {
			if (m_viewportDim.x != viewportPanelSize.x || m_viewportDim.y != viewportPanelSize.y) {
				m_viewportDim = { viewportPanelSize.x, viewportPanelSize.y };
				m_viewportResized = true;
			}

			ImGui::Image(
				reinterpret_cast<ImTextureID>(m_frameBuffer->getColorAttachmentHandle()),
				ImVec2((float)m_frameBuffer->getSpecification().width, (float)m_frameBuffer->getSpecification().height)
			);

			// Drag drop
			if (ImGui::BeginDragDropTarget()) {

				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
					std::string path = static_cast<const char*>(payload->Data);
					AX_CORE_LOG_WARN(path);
				}
				ImGui::EndDragDropTarget();
			}


			// Gizmos begin
			Entity selectedEntity = m_sceneHierarchyPanel->getSelectedEntity();
			if (selectedEntity) {
				ImGuizmo::SetOrthographic(false); // TODO: review, maybe not every frame needed
				ImGuizmo::SetDrawlist();
				float windowWidth = (float)ImGui::GetWindowWidth();
				float windowHeight = (float)ImGui::GetWindowHeight();
				ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

				// camera
				const Mat4& cameraView = m_editorCamera.getViewMatrix();
				const Mat4& cameraProjection = m_editorCamera.getProjectionMatrix();

				// entity transform
				auto& tc = selectedEntity.getComponent<TransformComponent>();
				Mat4 worldM = tc.getTransform();

				// to float[16] for ImGuizmo
				DirectX::XMFLOAT4X4 objF4;
				DirectX::XMStoreFloat4x4(&objF4, worldM.toXM());
				float object[16];
				memcpy(object, &objF4, sizeof(objF4));

				static ImGuizmo::OPERATION currentOp = ImGuizmo::TRANSLATE;
				if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) currentOp = ImGuizmo::TRANSLATE;
				if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) currentOp = ImGuizmo::ROTATE;
				if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) currentOp = ImGuizmo::SCALE;

				ImGuizmo::Manipulate(
					cameraView.data(),
					cameraProjection.data(),
					currentOp,
					ImGuizmo::LOCAL, // TODO: maybe set to world
					object
				);

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
			// Gizmos end
		}
		ImGui::End();
		ImGui::PopStyleVar();
	
		// panels
		if (m_showSystemInfoPanel) { m_systemInfoPanel->onGuiRender(); }
		if (m_showSceneHierarchyPanel) { m_sceneHierarchyPanel->onGuiRender(); }
		if (m_showEditorCameraPanel) { m_editorCameraPanel->onGuiRender(); }
		if (m_showContentBrowserPanel) { m_contentBrowserPanel->onGuiRender(); }
		if (m_showProjectPanel) { m_projectPanel->onGuiRender(); }
		
		if (m_showNewProjectWindow) drawNewProjectWindow();

		// menu bar
		if (ImGui::BeginMenuBar()) {
			ImGui::Text("      Axion Studio           ");

			// file menu
			if (ImGui::BeginMenu("  File  ")) {
				if (ImGui::MenuItem("New Scene", "Ctrl+N")) { m_newSceneRequested = true; }
				if (ImGui::MenuItem("Open Scene...", "Ctrl+O")) { m_openSceneRequested = true; }
				ImGui::Separator();
				if (ImGui::MenuItem("Save Scene", "Ctrl+S")) { m_saveSceneRequested = true; }
				if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S")) { m_saveSceneAsRequested = true; }
				ImGui::Separator();
				if (ImGui::MenuItem("Exit")) { Application::get().close(); }
				ImGui::EndMenu();
			}

			// edit menu
			if (ImGui::BeginMenu("  Edit  ")) {
				ImGui::EndMenu();
			}

			// view menu
			if (ImGui::BeginMenu("  View  ")) {
				ImGui::MenuItem("Scene Hierarchy", nullptr, &m_showSceneHierarchyPanel);
				ImGui::MenuItem("Content Browser", nullptr, &m_showContentBrowserPanel);
				ImGui::MenuItem("Project Overview", nullptr, &m_showProjectPanel);
				ImGui::MenuItem("Editor Camera Properties", nullptr, &m_showEditorCameraPanel);
				ImGui::EndMenu();
			}

			// project menu
			if (ImGui::BeginMenu("  Project  ")) {
				if (ImGui::MenuItem("New...")) { m_showNewProjectWindow = true; }
				if (ImGui::MenuItem("Open...")) { m_openProjectRequested = true; }
				if (ImGui::MenuItem("Save")) { m_saveProjectRequested = true; }
				ImGui::EndMenu();
			}

			// help menu
			if (ImGui::BeginMenu("  Help  ")) {
				ImGui::MenuItem("System Info", nullptr, &m_showSystemInfoPanel);
				ImGui::EndMenu();
			}

			#if AX_WIN_USING_CUSTOM_TITLE_BAR

			// Detect dragging on empty space in the menu bar
			float dragZoneStartX = 285.0f;
			float dragZoneEndX = ImGui::GetWindowWidth() - 134.0f;
			
			ImVec2 winPos = ImGui::GetWindowPos();
			ImVec2 cursor = ImGui::GetMousePos();
			ImVec2 local = ImVec2(cursor.x - winPos.x, cursor.y - winPos.y);

			float menuBarHeight = ImGui::GetFrameHeight();

			if (local.y >= 0 && local.y <= menuBarHeight && local.x >= dragZoneStartX && local.x <= dragZoneEndX && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
				HWND hwnd = static_cast<HWND>(Application::get().getWindow().getNativeHandle());
				ReleaseCapture();
				SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
			}

			ImGui::SameLine(ImGui::GetWindowWidth() - 134);
			if (ImGui::Button(u8"\uE15B", { 42, 24 })) { Application::get().minimizeWindow(); }
			ImGui::SameLine();
			if (ImGui::Button(u8"\uE5D1", { 42, 24 })) { Application::get().maximizeOrRestoreWindow(); }
			ImGui::SameLine();
			if (ImGui::Button(u8"\uE5CD", { 42, 24 })) { Application::get().close(); }
			#endif

			ImGui::EndMenuBar();

		}

		ImGui::End();
	}

	bool EditorLayer::onKeyPressed(KeyPressedEvent& e) {
		// shortcuts only from here on
		if (e.getRepeatCount() > 0) return false;
		bool controlPressed = Input::isKeyPressed(KeyCode::LeftControl) || Input::isKeyPressed(KeyCode::RightControl);
		bool shiftPressed = Input::isKeyPressed(KeyCode::LeftShift) || Input::isKeyPressed(KeyCode::RightShift);
		switch (e.getKeyCode()) {
			case KeyCode::N: {
				if (controlPressed) { m_newSceneRequested = true; }
				break;
			}
			case KeyCode::O: {
				if (controlPressed) { m_openSceneRequested = true; }
				break;
			}
			case KeyCode::S: {
				if (controlPressed && shiftPressed) { m_saveSceneAsRequested = true; }
				else if (controlPressed && (!shiftPressed)) { m_saveSceneRequested = true; }
				break;
			}
			default: break;
		}

		return false;
	}

	bool EditorLayer::onRenderingFinished(RenderingFinishedEvent& e) {
		if (m_newSceneRequested) {
			m_activeScene = std::make_shared<Scene>();
			m_sceneHierarchyPanel->setContext(m_activeScene);
			m_activeSceneFilePath.clear();
			m_newSceneRequested = false;
		}

		if (m_openSceneRequested) {
			std::string path = FileDialogs::openFile({ {"Axion Scene", "*.axion"} });
			if (!path.empty()) {
				m_activeScene = std::make_shared<Scene>();
				m_sceneHierarchyPanel->setContext(m_activeScene);
				SceneSerializer serializer(m_activeScene);
				serializer.deserializeText(path);
				m_activeSceneFilePath = path;
			}
			m_openSceneRequested = false;
		}

		if (m_saveSceneRequested) {
			if (!m_activeSceneFilePath.empty()) {
				SceneSerializer serializer(m_activeScene);
				serializer.serializeText(m_activeSceneFilePath);
			}
			m_saveSceneRequested = false;
		}

		if (m_saveSceneAsRequested) {
			std::string filePath = FileDialogs::saveFile({ {"Axion Scene", "*.axion"} });
			if (!filePath.empty()) {
				SceneSerializer serializer(m_activeScene);
				serializer.serializeText(filePath);
			}
			m_saveSceneAsRequested = false;
		}

		if (m_newProjectRequested) {
			std::string folderPath = FileDialogs::openFolder();
			AX_CORE_LOG_WARN(folderPath);
			m_newProjectRequested = false;
		}

		if (m_openProjectRequested) {
			std::string filePath = FileDialogs::openFile({ {"Axion Project", "*.axproj"} });
			if (!filePath.empty()) {
				if (!m_activeProjectFilePath.empty()) m_activeProject->save(m_activeProjectFilePath);
				
				m_activeProject->load(filePath);
				m_activeProjectFilePath = m_activeProject->getProjectPath();
				AX_CORE_LOG_TRACE("Loaded project {}", m_activeProject->getName());
			}

			m_openProjectRequested = false;
		}

		if (m_saveProjectRequested) {
			if (!m_activeProjectFilePath.empty()) {
				m_activeProject->save(m_activeProjectFilePath);
				AX_CORE_LOG_TRACE("Saved Project {}", m_activeProject->getName());
			}

			m_saveProjectRequested = false;
		}

		if (m_viewportResized) {
			m_frameBuffer->resize((uint32_t)m_viewportDim.x, (uint32_t)m_viewportDim.y);
			m_editorCamera.resize(m_viewportDim.x, m_viewportDim.y);
			m_viewportResized = false;
		}

		return false;
	}

	void EditorLayer::drawNewProjectWindow() {
		ImGui::Begin("Create new Project", &m_showNewProjectWindow);

		ImGui::InputText("Project Name", m_newNameBuffer, IM_ARRAYSIZE(m_newNameBuffer));

		ImGui::InputText("Location", m_newLocationBuffer, IM_ARRAYSIZE(m_newLocationBuffer));
		ImGui::SameLine();
		if (ImGui::Button("Browse...")) {
			std::string folder = FileDialogs::openFolder();
			if (!folder.empty()) {
				strncpy(m_newLocationBuffer, folder.c_str(), IM_ARRAYSIZE(m_newLocationBuffer));
				m_newLocationBuffer[IM_ARRAYSIZE(m_newLocationBuffer) - 1] = '\0';
			}
		}

		ImGui::Separator();

		if (ImGui::Button("Create Project")) {
			std::string name(m_newNameBuffer);
			std::string location(m_newLocationBuffer);

			if (!name.empty() && !location.empty()) {
				m_activeProject = Project::createNew(location, name);
				AX_CORE_LOG_INFO("Created Project {} at {}", m_activeProject->getName(), m_activeProject->getProjectPath());
				m_showNewProjectWindow = false;
			}
		}

		ImGui::End();
	}

}
