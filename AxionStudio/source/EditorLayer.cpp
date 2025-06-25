#include "EditorLayer.h"

#include "Axion/events/Event.h"
#include "Axion/core/PlatformInfo.h"

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

		m_dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_None;
		m_windowFlags = ImGuiWindowFlags_MenuBar |
			ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoNavFocus;

		setupSystemInfo();
	}

	void EditorLayer::onDetach() {
		m_buffer1->release();
		m_texture->release();
		m_frameBuffer->release();
	}

	void EditorLayer::onUpdate(Timestep ts) {

		m_cameraController.onUpdate(ts);

		Renderer2D::beginScene(m_cameraController.getCamera());
		
		m_cameraController.resize(m_viewportDim.x, m_viewportDim.y);

		if (m_viewportDim.x > 0 && m_viewportDim.y > 0) {
			m_frameBuffer->resize((uint32_t)m_viewportDim.x, (uint32_t)m_viewportDim.y);	//TODO: make it update only when values changed

			m_frameBuffer->bind();
			m_frameBuffer->clear(m_testColor);

			Renderer2D::drawTexture({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }, m_texture, m_buffer1);

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
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f)); // TODO: Maybe remove
		
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
		ImGui::ColorEdit4("Color", m_testColor);
		ImGui::Text(m_context->getDeviceWrapper().getAdapterName().c_str());
		static bool checked = true;
		ImGui::Checkbox("Check", &checked);
		ImGui::Button("Button");
		ImGui::End();
	
		// system info panel
		if (m_showSystemInfoWindow) {
			if (ImGui::Begin("System Info"), &m_showSystemInfoWindow) {
				const auto& info = m_systemInfo;

				ImGui::Columns(2, nullptr, false);

				ImGui::Text("GPU:");		ImGui::NextColumn();	ImGui::Text("%s", info.gpuName.c_str());			ImGui::NextColumn();
				ImGui::Text("VRAM:");		ImGui::NextColumn();	ImGui::Text("%llu MB", info.vramMB);				ImGui::NextColumn();
				ImGui::Text("Driver:");		ImGui::NextColumn();	ImGui::Text("%s", info.gpuDriverVersion.c_str());	ImGui::NextColumn();

				ImGui::Separator();			ImGui::Columns(2, nullptr, false);

				ImGui::Text("CPU:");		ImGui::NextColumn();	ImGui::Text("%s", info.cpuName.c_str());			ImGui::NextColumn();
				ImGui::Text("Cores:");		ImGui::NextColumn();	ImGui::Text("%u", info.cores);						ImGui::NextColumn();
				ImGui::Text("RAM:");		ImGui::NextColumn();	ImGui::Text("%llu MB", info.totalRamMB);			ImGui::NextColumn();

				ImGui::Separator();			ImGui::Columns(2, nullptr, false);

				ImGui::Text("OS:");			ImGui::NextColumn();	ImGui::TextWrapped("%s", info.os.c_str());			ImGui::NextColumn();

				ImGui::Columns(1);
			}
			ImGui::End();
		}
		
	
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

			if (ImGui::BeginMenu("Help")) {
				if (ImGui::MenuItem("System Info", nullptr, &m_showSystemInfoWindow)) {
					m_showSystemInfoWindow != m_showSystemInfoWindow;
				}
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

	void EditorLayer::setupSystemInfo() {
		m_systemInfo.gpuName = GraphicsContext::get()->getGpuName();
		m_systemInfo.gpuDriverVersion = GraphicsContext::get()->getGpuDriverVersion();
		m_systemInfo.vramMB = GraphicsContext::get()->getVramMB();

		m_systemInfo.cpuName = PlatformInfo::getCpuName();
		m_systemInfo.cores = PlatformInfo::getCpuCores();

		m_systemInfo.totalRamMB = PlatformInfo::getRamMB();
		m_systemInfo.os = PlatformInfo::getOsVersion();
	}

}
