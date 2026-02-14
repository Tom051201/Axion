#include "EditorLayer.h"

#include "AxionEngine/Vendor/ImGuizmo/ImGuizmo.h"

#include "AxionEngine/Source/events/Event.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/scene/SceneSerializer.h"
#include "AxionEngine/Source/scene/SceneManager.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionStudio/Source/core/EditorStateSerializer.h"

// -- Windows only --
#if AX_WIN_USING_CUSTOM_TITLE_BAR
#include "AxionStudio/Source/platform/windows/WindowsTitleBar.h"
#endif

// TODO: REMOVE this
#include "AxionEngine/Source/scripting/NativeScripts.h"

namespace Axion {

	EditorLayer::EditorLayer()
		: Layer("AxionStudioLayer"), m_editorCamera(1280, 720) {}

	void EditorLayer::onAttach() {

		// ----- Set scene -----
		m_sceneState = SceneState::Editing;
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
		ImGuizmo::SetOrthographic(false); // TODO maybe do orthographic


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
		dispatcher.dispatch<SceneChangedEvent>(AX_BIND_EVENT_FN(EditorLayer::onSceneChanged));
		dispatcher.dispatch<FileDropEvent>(AX_BIND_EVENT_FN(EditorLayer::onFileDrop));
	}

	void EditorLayer::onGuiRender() {
		beginDockspace();

		drawSceneViewport();

		// -- Draw all panels --
		m_panelManager.renderAll();

		// -- Draw all modals --
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
		if (e.getKeyCode() == KeyCode::Space) {
			if (m_sceneState == SceneState::Editing) {
				m_sceneState = SceneState::Playing;
			}
			else {
				m_sceneState = SceneState::Editing;
			}
		}

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
			e.addComponent<SpriteComponent>();
			e.addComponent<NativeScriptComponent>().bind<CameraController>();
			e.addComponent<AudioComponent>();
			AssetHandle<AudioClip> clip = AssetManager::load<AudioClip>(std::filesystem::absolute("AxionStudio/Projects/ExampleProject/Assets/audio/ping.axaudio").string());
			e.getComponent<AudioComponent>().audio = std::make_shared<AudioSource>(clip);
			e.getComponent<AudioComponent>().isSource = true;
			e.addComponent<CameraComponent>();
		}

		// -> Shortcuts only from here on
		if (e.getRepeatCount() > 0) return false;
		bool controlPressed = Input::isKeyPressed(KeyCode::LeftControl) || Input::isKeyPressed(KeyCode::RightControl);
		bool shiftPressed = Input::isKeyPressed(KeyCode::LeftShift) || Input::isKeyPressed(KeyCode::RightShift);
		switch (e.getKeyCode()) {
			case KeyCode::N: {
				// -- Ctrl + N --
				if (controlPressed) {
					SceneManager::newScene();
				}
				break;
			}
			case KeyCode::O: {
				// -- Ctrl + O --
				if (controlPressed) {
					std::string path = FileDialogs::openFile({ {"Axion Scene", "*.axscene"} });
					if (!path.empty()) SceneManager::loadScene(path);
				}
				break;
			}
			case KeyCode::S: {
				// -- Ctrl + Shift + S --
				if (controlPressed && shiftPressed) {
					std::string path = FileDialogs::saveFile({ {"Axion Scene", "*.axscene"} });
					if (!path.empty()) SceneManager::saveScene(path);
				}

				// -- Ctrl + S
				else if (controlPressed && (!shiftPressed)) {
					std::string path = SceneManager::getScenePath();
					if (!path.empty()) SceneManager::saveScene(path);
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
			if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) currentOp = ImGuizmo::TRANSLATE;
			if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) currentOp = ImGuizmo::ROTATE;
			if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) currentOp = ImGuizmo::SCALE;

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
				tc.scale = trs.scale;
				
				tc.rotation.x = Math::toDegrees(trs.rotationEuler.x);
				tc.rotation.y = Math::toDegrees(trs.rotationEuler.y);
				tc.rotation.z = Math::toDegrees(trs.rotationEuler.z);

				worldM = Mat4::TRS(tc.position, tc.rotation, tc.scale);
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

}
