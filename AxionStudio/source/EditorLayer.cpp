#include "EditorLayer.h"

#include "Axion/events/Event.h"
#include "Axion/core/PlatformInfo.h"

// TEMP
#include "platform/directx/D12FrameBuffer.h"

namespace Axion {

	EditorLayer::EditorLayer() : Layer("AxionStudioLayer"), m_cameraController(1280.0f / 720.0f, true) {}

	void EditorLayer::onAttach() {

		m_buffer1 = ConstantBuffer::create(sizeof(ObjectBuffer));
		m_texture = Texture2D::create("assets/logo.png");

		Renderer2D::setClearColor({ 0.3f, 0.3f, 0.3f, 1.0f });

		m_systemInfoPanel = std::make_unique<SystemInfoPanel>();
		m_systemInfoPanel->setup();

		FrameBufferSpecification fbs;
		fbs.width = 1280;
		fbs.height = 720;
		m_frameBuffer = FrameBuffer::create(fbs);
		m_viewportDim = { (float)fbs.width, (float)fbs.height };

		m_context = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext());

		m_activeScene = std::make_shared<Scene>();

		auto square = m_activeScene->createEntity("Square");
		square.addComponent<SpriteRendererComponent>(Vec4{ 1.0f, 0.0f, 0.0f, 1.0f });
		m_squareEntity = square;

		m_cameraEntity = m_activeScene->createEntity("Camera Entity");
		m_cameraEntity.addComponent<CameraComponent>(Mat4::orthographic(-16.0f, 16.0f, -9.0f, 9.0f, -1.0f, 1.0f));
		m_cameraEntity.getComponent<CameraComponent>().isPrimary = true;

		m_dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_None;
		m_windowFlags = ImGuiWindowFlags_MenuBar |
			ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoNavFocus;
	}

	void EditorLayer::onDetach() {
		m_buffer1->release();
		m_texture->release();
		m_frameBuffer->release();

		m_systemInfoPanel->shutdown();
	}

	void EditorLayer::onUpdate(Timestep ts) {

		m_cameraController.resize(m_viewportDim.x, m_viewportDim.y);
		m_cameraController.onUpdate(ts);

		//Renderer2D::beginScene(m_cameraController.getCamera());

		if (m_viewportDim.x > 0 && m_viewportDim.y > 0) {
			m_frameBuffer->resize((uint32_t)m_viewportDim.x, (uint32_t)m_viewportDim.y);	//TODO: make it update only when values changed

			m_frameBuffer->bind();
			m_frameBuffer->clear();

			m_activeScene->onUpdate(ts);

			m_frameBuffer->unbind();
		}

		m_context->getSwapChainWrapper().setAsRenderTarget();
	}

	void EditorLayer::onEvent(Event& e) {
		m_cameraController.onEvent(e);
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
		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		if (viewportPanelSize.x > 0 && viewportPanelSize.y > 0) {
			m_viewportDim = { viewportPanelSize.x, viewportPanelSize.y };
			D12FrameBuffer* framebuffer = static_cast<D12FrameBuffer*>(m_frameBuffer.get());
			D12Context* context = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext());

			ImGui::Image(
				(ImTextureID)context->getSrvHeapWrapper().getGpuHandle(framebuffer->getSrvHeapIndex()).ptr,
				ImVec2((float)framebuffer->getSpecification().width, (float)framebuffer->getSpecification().height)
			);
		}
		ImGui::End();
		ImGui::PopStyleVar();

		// scene view
		ImGui::Begin("Scene View");
		ImGui::Text("%s", m_squareEntity.getComponent<TagComponent>().tag.c_str());
		auto& color = m_squareEntity.getComponent<SpriteRendererComponent>().color;
		ImGui::ColorEdit4("Color", color.data());
		ImGui::End();
	
		// system info panel
		if (m_showSystemInfoPanel) { m_systemInfoPanel->onGuiRender(); }
		
	
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
				ImGui::EndMenu();
			}

			// help menu
			if (ImGui::BeginMenu("  Help  ")) {
				if (ImGui::MenuItem("System Info", nullptr, &m_showSystemInfoPanel)) { m_showSystemInfoPanel != m_showSystemInfoPanel; }
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

}
