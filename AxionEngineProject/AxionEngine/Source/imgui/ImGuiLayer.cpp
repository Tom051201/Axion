#include "axpch.h"
#include "ImGuiLayer.h"

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/core/Application.h"
#include "AxionEngine/Source/render/GraphicsContext.h"
#include "AxionEngine/Source/imgui/ImGuiInputMapper.h"

#include "AxionEngine/Vendor/imgui/imgui.h"
#include "AxionEngine/Vendor/imgui/backends/imgui_impl_win32.h"
#include "AxionEngine/Vendor/imgui/backends/imgui_impl_dx12.h"
#include "AxionEngine/Vendor/imgui/backends/imgui_impl_opengl3.h"

namespace Axion {

	struct OpenGL3WindowData {
		HDC hdc;
		HWND hwnd;
		OpenGL3Context* ctx;
	};

	ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer"), m_activeAPI(RendererAPI::None) {}

	ImGuiLayer::~ImGuiLayer() {}

	void ImGuiLayer::onAttach() {


		m_activeAPI = Renderer::getAPI();

		IMGUI_CHECKVERSION();
		ImGui_ImplWin32_EnableDpiAwareness();
		ImGui::CreateContext();
		
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		
		// TODO: enable custom styles!
		//setStyle();
		ImGui::StyleColorsDark();

		switch (m_activeAPI) {
			case Axion::RendererAPI::None: { AX_CORE_LOG_ERROR("None is not supported yet"); return; }
			case Axion::RendererAPI::DirectX12: { setupD12(); break; }
			case Axion::RendererAPI::OpenGL3: { setupOpenGL(); break; }
		}

		AX_CORE_LOG_TRACE("ImGui layer attached");
	}

	void ImGuiLayer::onDetach() {

		switch (m_activeAPI) {
			case Axion::RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet"); return; }
			case Axion::RendererAPI::DirectX12: { ImGui_ImplDX12_Shutdown(); break; }
			case Axion::RendererAPI::OpenGL3: { ImGui_ImplOpenGL3_Shutdown(); break; }
		}

		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyPlatformWindows();
		ImGui::DestroyContext();

		AX_CORE_LOG_TRACE("ImGuiLayer detatched");
	}
	
	void ImGuiLayer::onEvent(Event& ev) {
		EventDispatcher dispatcher(ev);
		dispatcher.dispatch<MouseButtonPressedEvent>(AX_BIND_EVENT_FN(ImGuiLayer::onMouseButtonPressedEvent));
		dispatcher.dispatch<MouseButtonReleasedEvent>(AX_BIND_EVENT_FN(ImGuiLayer::onMouseButtonReleasedEvent));
		dispatcher.dispatch<MouseMovedEvent>(AX_BIND_EVENT_FN(ImGuiLayer::onMouseMovedEvent));
		dispatcher.dispatch<MouseScrolledEvent>(AX_BIND_EVENT_FN(ImGuiLayer::onMouseScrolledEvent));
		dispatcher.dispatch<KeyPressedEvent>(AX_BIND_EVENT_FN(ImGuiLayer::onKeyPressedEvent));
		dispatcher.dispatch<KeyReleasedEvent>(AX_BIND_EVENT_FN(ImGuiLayer::onKeyReleasedEvent));
		dispatcher.dispatch<KeyTypedEvent>(AX_BIND_EVENT_FN(ImGuiLayer::onKeyTypedEvent));
		dispatcher.dispatch<WindowResizeEvent>(AX_BIND_EVENT_FN(ImGuiLayer::onWindowResizeEvent));
		dispatcher.dispatch<WindowCloseEvent>(AX_BIND_EVENT_FN(ImGuiLayer::onWindowCloseEvent));
	}

	void ImGuiLayer::beginRender() {
		switch (m_activeAPI) {
			case Axion::RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet!"); return; }
			case Axion::RendererAPI::DirectX12: { ImGui_ImplDX12_NewFrame(); break; }
			case Axion::RendererAPI::OpenGL3: { ImGui_ImplOpenGL3_NewFrame(); break; }
		}

		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiLayer::endRender() {
		ImGui::Render();

		if (!m_active) return;

		switch (m_activeAPI) {
			case Axion::RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet"); return; }

			case Axion::RendererAPI::DirectX12: {
				ID3D12DescriptorHeap* heaps[] = { m_d12Context->getSrvHeapWrapper().getHeap() };
				m_d12Context->getCommandList()->SetDescriptorHeaps(_countof(heaps), heaps);
				ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_d12Context->getCommandList());
				break;
			}

			case Axion::RendererAPI::OpenGL3: {
				ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
				break;
			}
		}

		// for multiple viewports
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable/* && !ImGui::GetIO().WantSaveIniSettings*/) {
			ImGui::UpdatePlatformWindows();
			if (m_activeAPI == RendererAPI::DirectX12) {
				ImGui::RenderPlatformWindowsDefault(nullptr, m_d12Context->getCommandQueue());
			}
			else if (m_activeAPI == RendererAPI::OpenGL3) {
				ImGui::RenderPlatformWindowsDefault();
			}
		}

	}

	void ImGuiLayer::setupD12() {
		m_d12Context = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext());
		auto& srvHeap = m_d12Context->getSrvHeapWrapper();
		m_srvHeapIndex = srvHeap.allocate();

		ImGui_ImplWin32_Init((HWND)Application::get().getWindow().getNativeHandle());
		ImGui_ImplDX12_Init(
			m_d12Context->getDevice(),
			2,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			srvHeap.getHeap(),
			srvHeap.getCpuHandle(m_srvHeapIndex),
			srvHeap.getGpuHandle(m_srvHeapIndex)
		);
	}

	void ImGuiLayer::setupOpenGL() {
		const char* glsl_version = "#version 130";
		ImGui_ImplWin32_InitForOpenGL(Application::get().getWindow().getNativeHandle());
		ImGui_ImplOpenGL3_Init(glsl_version);

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
			IM_ASSERT(platform_io.Renderer_CreateWindow == NULL);
			IM_ASSERT(platform_io.Renderer_DestroyWindow == NULL);
			IM_ASSERT(platform_io.Renderer_SwapBuffers == NULL);
			IM_ASSERT(platform_io.Platform_RenderWindow == NULL);

			platform_io.Renderer_CreateWindow = [](ImGuiViewport* viewport) {
				auto* context = static_cast<OpenGL3Context*>(GraphicsContext::get()->getNativeContext());
				
				OpenGL3WindowData* data = IM_NEW(OpenGL3WindowData);
				data->hwnd = (HWND)viewport->PlatformHandle;
				data->hdc = GetDC(data->hwnd);
				data->ctx = context;

				viewport->RendererUserData = data;
				wglMakeCurrent(data->hdc, context->getHGLRC());
			};

			platform_io.Renderer_DestroyWindow = [](ImGuiViewport* viewport) {
				auto* data = static_cast<OpenGL3WindowData*>(viewport->RendererUserData);
				if (data) {
					wglMakeCurrent(nullptr, nullptr);
					ReleaseDC(data->hwnd, data->hdc);
					IM_DELETE(data);
					viewport->RendererUserData = nullptr;
				}
			};

			platform_io.Renderer_SwapBuffers = [](ImGuiViewport* viewport, void*) {
				auto* data = static_cast<OpenGL3WindowData*>(viewport->RendererUserData);
				if (data) {
					SwapBuffers(data->hdc);
				}
			};
			
			platform_io.Platform_RenderWindow = [](ImGuiViewport* viewport, void*) {
				auto* data = static_cast<OpenGL3WindowData*>(viewport->RendererUserData);
				if (data) {
					wglMakeCurrent(data->hdc, data->ctx->getHGLRC());
				}
			};
		}
	}

	bool ImGuiLayer::onMouseButtonPressedEvent(MouseButtonPressedEvent& e) {
		ImGuiIO& io = ImGui::GetIO();
		io.MouseDown[e.getMouseButton()] = true;
		return false;
	}

	bool ImGuiLayer::onMouseButtonReleasedEvent(MouseButtonReleasedEvent& e) {
		ImGuiIO& io = ImGui::GetIO();
		io.MouseDown[e.getMouseButton()] = false;
		return false;
	}

	bool ImGuiLayer::onMouseMovedEvent(MouseMovedEvent& e) {
		ImGuiIO& io = ImGui::GetIO();
		io.MousePos = ImVec2(e.getX(), e.getY());
		return false;
	}

	bool ImGuiLayer::onMouseScrolledEvent(MouseScrolledEvent& e) {
		ImGuiIO& io = ImGui::GetIO();
		io.MouseWheelH += e.getXOffset();
		io.MouseWheel += e.getYOffset();
		return false;
	}

	bool ImGuiLayer::onKeyPressedEvent(KeyPressedEvent& e) {
		ImGuiIO& io = ImGui::GetIO();
		ImGuiKey key = ImGuiInputMapper::toImGuiKeyCode(e.getKeyCode());

		if (key != ImGuiKey_None) {
			io.AddKeyEvent(key, true);
		}

		return false;
	}

	bool ImGuiLayer::onKeyReleasedEvent(KeyReleasedEvent& e) {
		ImGuiIO& io = ImGui::GetIO();
		ImGuiKey key = ImGuiInputMapper::toImGuiKeyCode(e.getKeyCode());

		if (key != ImGuiKey_None) {
			io.AddKeyEvent(key, false);
		}

		return false;
	}

	// no need for this function when 'ImGui_ImplWin32_WndProcHandler' is used
	// it handels type events without needing this
	bool ImGuiLayer::onKeyTypedEvent(KeyTypedEvent& e) {
		//ImGuiIO& io = ImGui::GetIO();
		//io.AddInputCharacter(e.getChar());
		return false;
	}

	bool ImGuiLayer::onWindowResizeEvent(WindowResizeEvent& e) {
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(static_cast<float>(e.getWidth()), static_cast<float>(e.getHeight()));
		return false;
	}

	bool ImGuiLayer::onWindowCloseEvent(WindowCloseEvent& e) {
		m_active = false;
		return false;
	}

	void ImGuiLayer::setStyle() {
		ImGuiStyle& style = ImGui::GetStyle();

		// sets layout
		style.FrameRounding = 2.0f;
		style.PopupRounding = 2.0f;
		style.ScrollbarRounding = 2.0f;
		style.GrabRounding = 2.0f;

		style.FramePadding = ImVec2(3.0f, 3.0f);
		style.ItemSpacing = ImVec2(3.0f, 3.0f);

		// sets colors
		ImVec4* colors = style.Colors;
		colors[ImGuiCol_WindowBg] = ImVec4(0.176f, 0.176f, 0.188f, 1.0f);
		colors[ImGuiCol_Text] = ImVec4(0.91f, 0.91f, 0.91f, 1.0f);

		colors[ImGuiCol_Header] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
		colors[ImGuiCol_HeaderActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.45f);
		
		colors[ImGuiCol_Button] = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
		colors[ImGuiCol_ButtonActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.45f);

		colors[ImGuiCol_MenuBarBg] = ImVec4(0.076f, 0.076f, 0.088f, 1.0f);
		
		colors[ImGuiCol_Border] = ImVec4(0.32f, 0.0f, 0.0f, 1.0f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);

		colors[ImGuiCol_TitleBg] = ImVec4(0.076f, 0.076f, 0.088f, 1.0f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.076f, 0.076f, 0.088f, 1.0f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);

		colors[ImGuiCol_CheckMark] = ImVec4(0.32f, 0.0f, 0.0f, 1.0f);

		colors[ImGuiCol_FrameBg] = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.32f, 0.0f, 0.0f, 1.0f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.45f);

		colors[ImGuiCol_ResizeGrip] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.45f);

		colors[ImGuiCol_Tab] = ImVec4(0.32f, 0.0f, 0.0f, 1.0f);
		colors[ImGuiCol_TabActive] = ImVec4(0.32f, 0.0f, 0.0f, 1.0f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.42f, 0.0f, 0.0f, 1.0f);
		colors[ImGuiCol_TabDimmed] = ImVec4(0.32f, 0.0f, 0.0f, 1.0f);
		colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.32f, 0.0f, 0.0f, 1.0f);
		colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.62f, 0.0f, 0.0f, 1.0f);
		colors[ImGuiCol_TabSelected] = ImVec4(0.52f, 0.0f, 0.0f, 1.0f);
		colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.62f, 0.0f, 0.0f, 1.0f);

		// sets fonts
		ImGuiIO& io = ImGui::GetIO();
		io.Fonts->Clear();
		
		// TODO: figure a fucking way out to make this fucking project independen!!!
		io.Fonts->AddFontFromFileTTF("AxionStudio/Assets/fonts/openSans/OpenSans-Bold.ttf", 18.0f);
		ImFontConfig icons_config;
		icons_config.MergeMode = true;
		icons_config.PixelSnapH = true;
		static const ImWchar icon_ranges[] = { 0xE000, 0xF8FF, 0 };
		io.Fonts->AddFontFromFileTTF("AxionStudio/Assets/fonts/icons/MaterialSymbolsOutlined-Regular.ttf", 20.0f, &icons_config, icon_ranges);

		io.FontDefault = io.Fonts->Fonts[0];
		
		switch (m_activeAPI) {
			case Axion::RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet"); return; }
			
			case Axion::RendererAPI::DirectX12: {
				ImGui_ImplDX12_InvalidateDeviceObjects();
				ImGui_ImplDX12_CreateDeviceObjects();
				break;
			}
			
			case Axion::RendererAPI::OpenGL3: {
				//ImGui_ImplOpenGL3_DestroyDeviceObjects();
				//ImGui_ImplOpenGL3_CreateDeviceObjects();	
				break;
			}
		}

	}

}
