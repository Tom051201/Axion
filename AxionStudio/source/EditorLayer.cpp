#include "EditorLayer.h"

// TEMP
#include "imgui.h"
#include "platform/directx/D12FrameBuffer.h"

namespace Axion {

	EditorLayer::EditorLayer() : Layer("AxionStudioLayer"), m_cameraController(1280.0f / 720.0f, true) {}

	void EditorLayer::onAttach() {
		m_buffer1 = ConstantBuffer::create(sizeof(ObjectBuffer));

		m_buffer2 = ConstantBuffer::create(sizeof(ObjectBuffer));
		m_texture = Texture2D::create("assets/logo.png");

		Renderer2D::setClearColor({ 0.3f, 0.3f, 0.3f, 1.0f });

		FrameBufferSpecification fbs;
		fbs.width = 1280.0f;
		fbs.height = 720.0f;
		m_frameBuffer = FrameBuffer::create(fbs);

		m_context = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext());
	}

	void EditorLayer::onDetach() {
		m_buffer1->release();
		m_buffer2->release();
		m_texture->release();
		m_frameBuffer->release();
	}

	void EditorLayer::onUpdate(Timestep ts) {

		m_cameraController.onUpdate(ts);

		Renderer2D::beginScene(m_cameraController.getCamera());
		m_frameBuffer->bind();
		m_frameBuffer->clear({ 0.0f, 0.0f, 0.0f, 1.0f });

		Renderer2D::drawQuad({ -0.55f, 0.0f, 0.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f }, m_buffer1);
		
		Renderer2D::drawTexture({ 0.55f, 0.0f, 0.0f }, { 1.0f, 1.0f }, m_texture, m_buffer2);

		m_frameBuffer->unbind();

		m_context->getSwapChainWrapper().setAsRenderTarget();

		// maybe add back in later when needed
		//m_context->getCommandListWrapper().close();
		//m_context->getCommandQueueWrapper().executeCommandList(m_context->getCommandListWrapper().getCommandList());
		//m_context->waitForPreviousFrame();
		//m_context->getCommandListWrapper().reset();
	}

	void EditorLayer::onEvent(Event& e) {
		m_cameraController.onEvent(e);
	}

	void EditorLayer::onGuiRender() {

		static bool dockspaceOpen = true;
		static bool opt_fullscreen = true;
		static bool opt_padding = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
		
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen) {
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		else {
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}
		
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) window_flags |= ImGuiWindowFlags_NoBackground;
		if (!opt_padding) ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
		if (!opt_padding) ImGui::PopStyleVar();
		
		if (opt_fullscreen) ImGui::PopStyleVar(2);
		
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		// renders framebuffer
		D12FrameBuffer* framebuffer = static_cast<D12FrameBuffer*>(m_frameBuffer.get());
		D12Context* context = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext());
		ImGui::Begin("Image");
		ImGui::Image(
			(ImTextureID)context->getSrvHeapWrapper().getGpuHandle(framebuffer->getSrvHeapIndex()).ptr,
			ImVec2((float)framebuffer->getSpecification().width, (float)framebuffer->getSpecification().height)
		);
		ImGui::End();
		
		
		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("Options")) {
				// Disabling fullscreen would allow the window to be moved to the front of other windows,
				// which we can't undo at the moment without finer window depth/z control.
				ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
				ImGui::MenuItem("Padding", NULL, &opt_padding);
				ImGui::Separator();
		
				if (ImGui::MenuItem("Flag: NoDockingOverCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingOverCentralNode) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingOverCentralNode; }
				if (ImGui::MenuItem("Flag: NoDockingSplit", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingSplit) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingSplit; }
				if (ImGui::MenuItem("Flag: NoUndocking", "", (dockspace_flags & ImGuiDockNodeFlags_NoUndocking) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoUndocking; }
				if (ImGui::MenuItem("Flag: NoResize", "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoResize; }
				if (ImGui::MenuItem("Flag: AutoHideTabBar", "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar; }
				if (ImGui::MenuItem("Flag: PassthruCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, opt_fullscreen)) { dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; }
				ImGui::Separator();
		
				if (ImGui::MenuItem("Close", NULL, false)) dockspaceOpen = false;
				ImGui::EndMenu();
			}
		
			ImGui::EndMenuBar();
		}
		
		ImGui::End();
	}


}
