#include "EditorLayer.h"

#include "AxionEngine/Source/events/Event.h"
#include "AxionEngine/Source/core/PlatformInfo.h"

namespace Axion {

	EditorLayer::EditorLayer() : Layer("AxionStudioLayer"), m_editorCamera(1280.0f / 720.0f) {}

	void EditorLayer::onAttach() {

		m_activeScene = std::make_shared<Scene>();
		m_sceneState = SceneState::Editing;

		m_systemInfoPanel = std::make_unique<SystemInfoPanel>();
		m_systemInfoPanel->setup();
		m_sceneHierarchyPanel = std::make_unique<SceneHierarchyPanel>();
		m_sceneHierarchyPanel->setup(m_activeScene);
		m_editorCameraPanel = std::make_unique<EditorCameraPanel>();
		m_editorCameraPanel->setup(&m_editorCamera);

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


		std::vector<Axion::Vertex> vertices = {
			// Front face
			Axion::Vertex(-0.5f, -0.5f,  0.5f,   1, 0, 0, 1,   0.0f, 0.0f),
			Axion::Vertex(0.5f, -0.5f,  0.5f,   0, 1, 0, 1,   1.0f, 0.0f),
			Axion::Vertex(0.5f,  0.5f,  0.5f,   0, 0, 1, 1,   1.0f, 1.0f),
			Axion::Vertex(-0.5f,  0.5f,  0.5f,   1, 1, 0, 1,   0.0f, 1.0f),

			// Back face
			Axion::Vertex(-0.5f, -0.5f, -0.5f,   1, 0, 1, 1,   1.0f, 0.0f),
			Axion::Vertex(0.5f, -0.5f, -0.5f,   0, 1, 1, 1,   0.0f, 0.0f),
			Axion::Vertex(0.5f,  0.5f, -0.5f,   1, 1, 1, 1,   0.0f, 1.0f),
			Axion::Vertex(-0.5f,  0.5f, -0.5f,   0, 0, 0, 1,   1.0f, 1.0f)
		};
		std::vector<uint32_t> indices = {
			// Front face
			0, 1, 2,
			2, 3, 0,

			// Right face
			1, 5, 6,
			6, 2, 1,

			// Back face
			5, 4, 7,
			7, 6, 5,

			// Left face
			4, 0, 3,
			3, 7, 4,

			// Top face
			3, 2, 6,
			6, 7, 3,

			// Bottom face
			4, 5, 1,
			1, 0, 4
		};
		auto cubeMesh = Mesh::create(vertices, indices);

		ShaderSpecification shaderSpec;
		shaderSpec.name = "Shader3D";
		shaderSpec.vertexLayout = {
			{ "POSITION", Axion::ShaderDataType::Float3 },
			{ "COLOR", Axion::ShaderDataType::Float4 },
			{ "TEXCOORD", Axion::ShaderDataType::Float2 }
		};
		Ref<Shader> shader = Shader::create(shaderSpec);
		shader->compileFromFile("AxionStudio/Assets/shaders/PositionShader.hlsl");
		auto cubeMaterial = Material::create("BasicMaterial", { 0.0f, 1.0f, 0.0f, 1.0f }, shader);

		auto cubeCB = ConstantBuffer::create(sizeof(ObjectBuffer));

		auto cube = m_activeScene->createEntity("Cube");
		cube.getComponent<TransformComponent>().position = Vec3::zero();
		cube.getComponent<TransformComponent>().rotation = Vec3::zero();
		cube.getComponent<TransformComponent>().scale = Vec3::one();
		cube.addComponent<MeshComponent>(cubeMesh);
		cube.addComponent<MaterialComponent>(cubeMaterial);
		cube.addComponent<ConstantBufferComponent>(cubeCB);


		auto cubeMesh2 = Mesh::create(vertices, indices);
		auto cubeMesh3 = Mesh::create(vertices, indices);
		auto cubeMesh4 = Mesh::create(vertices, indices);

		auto cubeCB2 = ConstantBuffer::create(sizeof(ObjectBuffer));
		auto cubeCB3 = ConstantBuffer::create(sizeof(ObjectBuffer));
		auto cubeCB4 = ConstantBuffer::create(sizeof(ObjectBuffer));

		auto cube2 = m_activeScene->createEntity("Cube2");
		cube2.getComponent<TransformComponent>().position = Vec3::zero();
		cube2.getComponent<TransformComponent>().rotation = Vec3::zero();
		cube2.getComponent<TransformComponent>().scale = Vec3::one();
		cube2.addComponent<MeshComponent>(cubeMesh2);
		cube2.addComponent<MaterialComponent>(cubeMaterial);
		cube2.addComponent<ConstantBufferComponent>(cubeCB2);

		auto cube3 = m_activeScene->createEntity("Cube3");
		cube3.getComponent<TransformComponent>().position = Vec3::zero();
		cube3.getComponent<TransformComponent>().rotation = Vec3::zero();
		cube3.getComponent<TransformComponent>().scale = Vec3::one();
		cube3.addComponent<MeshComponent>(cubeMesh3);
		cube3.addComponent<MaterialComponent>(cubeMaterial);
		cube3.addComponent<ConstantBufferComponent>(cubeCB3);

		auto cube4 = m_activeScene->createEntity("Cube4");
		cube4.getComponent<TransformComponent>().position = Vec3::zero();
		cube4.getComponent<TransformComponent>().rotation = Vec3::zero();
		cube4.getComponent<TransformComponent>().scale = Vec3::one();
		cube4.addComponent<MeshComponent>(cubeMesh4);
		cube4.addComponent<MaterialComponent>(cubeMaterial);
		cube4.addComponent<ConstantBufferComponent>(cubeCB4);

		m_camEntity = m_activeScene->createEntity("Camera");
		m_camEntity.addComponent<CameraComponent>(Mat4::orthographic(1080, 720, 0.1f, 100.0f));
		m_camEntity.getComponent<CameraComponent>().isPrimary = true;

		Application::get().setWindowTitle("Axion Studio - DirectX");
	}

	void EditorLayer::onDetach() {
		m_frameBuffer->release();

		m_systemInfoPanel->shutdown();
		m_sceneHierarchyPanel->shutdown();
		m_editorCameraPanel->shutdown();
	}

	void EditorLayer::onUpdate(Timestep ts) {

		m_editorCamera.onUpdate(ts);

		if (m_viewportResized) {
			m_frameBuffer->resize((uint32_t)m_viewportDim.x, (uint32_t)m_viewportDim.y);
			m_editorCamera.resize(m_viewportDim.x, m_viewportDim.y);
			m_viewportResized = false;
		}

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

			// TODO: check transpose for opengl when rendering when nothing is rendered!
		}

		Renderer::renderToSwapChain();
	}

	void EditorLayer::onEvent(Event& e) {
		m_editorCamera.onEvent(e);
		
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<KeyPressedEvent>(AX_BIND_EVENT_FN(EditorLayer::onKeyPressed));
		dispatcher.dispatch<WindowResizeEvent>(AX_BIND_EVENT_FN(EditorLayer::onWindowResize));
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
		}
		ImGui::End();
		ImGui::PopStyleVar();
	
		// panels
		if (m_showSystemInfoPanel) { m_systemInfoPanel->onGuiRender(); }
		if (m_showSceneHierarchyPanel) { m_sceneHierarchyPanel->onGuiRender(); }
		if (m_showEditorCameraPanel) { m_editorCameraPanel->onGuiRender(); }
		
	
		// menu bar
		if (ImGui::BeginMenuBar()) {
			ImGui::Text("      Axion Studio           ");

			// file menu
			if (ImGui::BeginMenu("  File  ")) {
				if (ImGui::MenuItem("New Scene", "Ctrl+N")) { AX_LOG_WARN("Creating a new scene is not supported yet!"); }
				if (ImGui::MenuItem("Open Project", "Ctrl+O")) { AX_LOG_WARN("Opening a project is not supported yet!"); }
				ImGui::Separator();
				if (ImGui::MenuItem("Save Scene", "Ctrl+S")) { AX_LOG_WARN("Saving a scene is not supported yet!"); }
				if (ImGui::MenuItem("Save Scene As", "Ctrl+Shift+S")) { AX_LOG_WARN("Saving a scene is not supported yet!"); }
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
				ImGui::MenuItem("Editor Camera Properties", nullptr, &m_showEditorCameraPanel);
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
			if (ImGui::Button(u8"\uE15B", {42, 24})) {}
			ImGui::SameLine();
			if (ImGui::Button(u8"\uE5D1", {42, 24})) {}
			ImGui::SameLine();
			if (ImGui::Button(u8"\uE5CD", {42, 24})) { Application::get().close(); }
			#endif

			ImGui::EndMenuBar();

		}

		ImGui::End();
	}

	bool EditorLayer::onWindowResize(WindowResizeEvent& e) {
		return false;
	}

	bool EditorLayer::onKeyPressed(KeyPressedEvent& e) {
		if (e.getKeyCode() == KeyCode::T) {
			Application::get().setGraphicsBackend(RendererAPI::DirectX12);
			Application::get().setWindowTitle("Axion Studio - DirectX");
		}

		return false;
	}

}
