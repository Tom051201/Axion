#include "EditorLayer.h"

#include "Axion/events/Event.h"

// TEMP
#include "platform/directx/D12FrameBuffer.h"

namespace Axion {

	bool EditorLayer::s_isDragging = false;
	POINT EditorLayer::s_dragOffset = { 0, 0 };

	EditorLayer::EditorLayer() : Layer("AxionStudioLayer"), m_cameraController(1280.0f / 720.0f, true) {}

	void EditorLayer::onAttach() {

		m_buffer1 = ConstantBuffer::create(sizeof(ObjectBuffer));
		m_texture = Texture2D::create("assets/logo.png");

		Renderer2D::setClearColor({ 0.3f, 0.3f, 0.3f, 1.0f });

		FrameBufferSpecification fbs;
		fbs.width = 1280;
		fbs.height = 720;
		m_frameBuffer = FrameBuffer::create(fbs);
		m_viewportDim = { (float)fbs.width, (float)fbs.height };

		m_context = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext());

		m_dockspaceFlags = ImGuiDockNodeFlags_None;
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
	}

	void EditorLayer::onUpdate(Timestep ts) {

		m_cameraController.onUpdate(ts);

		Renderer2D::beginScene(m_cameraController.getCamera());
		m_frameBuffer->bind();
		m_frameBuffer->clear(m_testColor);
		
		Renderer2D::drawTexture({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }, m_texture, m_buffer1);

		m_frameBuffer->unbind();

		m_context->getSwapChainWrapper().setAsRenderTarget();
	}

	void EditorLayer::onEvent(Event& e) {
		m_cameraController.onEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.dispatch<WindowResizeEvent>(BIND_EVENT_FN(EditorLayer::onWindowResize));

	}

	void EditorLayer::onGuiRender() {
		
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f)); // TODO: Maybe remove
		
		ImGui::Begin("DockSpaceFrame", nullptr, m_windowFlags);
		ImGui::PopStyleVar(3);
		
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), m_dockspaceFlags);
			
		}

		// renders framebuffer
		ImGui::Begin("Editor Viewport");
		D12FrameBuffer* framebuffer = static_cast<D12FrameBuffer*>(m_frameBuffer.get());
		D12Context* context = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext());

		ImGui::Image(
			(ImTextureID)context->getSrvHeapWrapper().getGpuHandle(framebuffer->getSrvHeapIndex()).ptr,
			ImVec2((float)framebuffer->getSpecification().width, (float)framebuffer->getSpecification().height)
		);
		ImGui::End();

		// scene view
		ImGui::Begin("Scene View");
		ImGui::ColorEdit4("Color", m_testColor);
		ImGui::Text(m_context->getDeviceWrapper().getAdapterName().c_str());
		static bool checked = true;
		ImGui::Checkbox("Check", &checked);
		ImGui::Button("Button");
		ImGui::End();
		
		// menu bar
		if (ImGui::BeginMenuBar()) {
			ImGui::Text("      Axion Studio           ");

			// file menu
			if (ImGui::BeginMenu("  File  ")) {
				if (ImGui::MenuItem("Exit", NULL)) { Application::get().close(); }
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
		m_frameBuffer->resize(e.getWidth(), e.getHeight()); // TODO: calculate correctly
		return false;
	}

}
