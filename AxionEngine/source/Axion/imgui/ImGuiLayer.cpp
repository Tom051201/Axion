#include "axpch.h"
#include "ImGuiLayer.h"

#include "Axion/core/Core.h"

#include "Axion/core/Application.h"
#include "Axion/render/GraphicsContext.h"
#include "Axion/imgui/ImGuiInputMapper.h"

#include "imgui/imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx12.h"

#include "platform/directx/D12Context.h"

namespace Axion {

	ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer") {}

	ImGuiLayer::~ImGuiLayer() {}

	void ImGuiLayer::onAttach() {

		m_context = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext());
		auto& srvHeap = m_context->getSrvHeapWrapper();
		m_srvHeapIndex = srvHeap.allocate();

		IMGUI_CHECKVERSION();
		ImGui_ImplWin32_EnableDpiAwareness();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		//ImGui::StyleColorsDark();
		
		setStyle();
	
		ImGui_ImplWin32_Init((HWND)Application::get().getWindow().getNativeHandle());
		ImGui_ImplDX12_Init(
			m_context->getDevice(),
			2,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			srvHeap.getHeap(),
			srvHeap.getCpuHandle(m_srvHeapIndex),
			srvHeap.getGpuHandle(m_srvHeapIndex)
		);

		AX_CORE_LOG_TRACE("ImGui layer attached");
	}

	void ImGuiLayer::onDetach() {
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyPlatformWindows();
		ImGui::DestroyContext();

		AX_CORE_LOG_TRACE("ImGuiLayer detatched");
	}
	
	void ImGuiLayer::onEvent(Event& ev) {
		EventDispatcher dispatcher(ev);
		dispatcher.dispatch<MouseButtonPressedEvent>(BIND_EVENT_FN(ImGuiLayer::onMouseButtonPressedEvent));
		dispatcher.dispatch<MouseButtonReleasedEvent>(BIND_EVENT_FN(ImGuiLayer::onMouseButtonReleasedEvent));
		dispatcher.dispatch<MouseMovedEvent>(BIND_EVENT_FN(ImGuiLayer::onMouseMovedEvent));
		dispatcher.dispatch<MouseScrolledEvent>(BIND_EVENT_FN(ImGuiLayer::onMouseScrolledEvent));
		dispatcher.dispatch<KeyPressedEvent>(BIND_EVENT_FN(ImGuiLayer::onKeyPressedEvent));
		dispatcher.dispatch<KeyReleasedEvent>(BIND_EVENT_FN(ImGuiLayer::onKeyReleasedEvent));
		//dispatcher.dispatch<KeyTypedEvent>(BIND_EVENT_FN(ImGuiLayer::onKeyTypedEvent));
		dispatcher.dispatch<WindowResizeEvent>(BIND_EVENT_FN(ImGuiLayer::onWindowResizeEvent));

		dispatcher.dispatch<WindowCloseEvent>(BIND_EVENT_FN(ImGuiLayer::onWindowCloseEvent));
	}

	void ImGuiLayer::beginRender() {
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiLayer::endRender() {
		if (m_active) { // TODO: find workaround to prevent crash when viewport is inside actual window
			ImGui::Render();

			ID3D12DescriptorHeap* heaps[] = { m_context->getSrvHeapWrapper().getHeap() };
			m_context->getCommandList()->SetDescriptorHeaps(_countof(heaps), heaps);
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_context->getCommandList());

			// for multiple viewports
			ImGuiIO& io = ImGui::GetIO();
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable && !ImGui::GetIO().WantSaveIniSettings) {
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault(nullptr, m_context->getCommandQueue());
			}
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
		colors[ImGuiCol_WindowBg]		= ImVec4(0.176f, 0.176f, 0.188f, 1.0f);
		colors[ImGuiCol_Text]			= ImVec4(0.91f, 0.91f, 0.91f, 1.0f);
		colors[ImGuiCol_Header]			= ImVec4(1.00f, 0.25f, 0.25f, 1.0f);
		colors[ImGuiCol_HeaderHovered]	= ImVec4(0.35f, 1.00f, 0.35f, 1.0f);
		colors[ImGuiCol_HeaderActive]	= ImVec4(0.0f, 0.0f, 1.0f, 1.0f);

		colors[ImGuiCol_TitleBg]			= ImVec4(0.08f, 0.08f, 0.08f, 1.0f);
		colors[ImGuiCol_TitleBgActive]		= ImVec4(0.08f, 0.08f, 0.08f, 1.0f);
		colors[ImGuiCol_TitleBgCollapsed]	= ImVec4(0.08f, 0.08f, 0.08f, 1.0f);

		ImVec4 myAccent = ImVec4(0.30f, 0.60f, 0.90f, 1.00f);
		ImVec4 myTitleColor = ImVec4(0.18f, 0.22f, 0.25f, 1.00f); // Change to your preferred color

		style.Colors[ImGuiCol_Tab] = myTitleColor;
		style.Colors[ImGuiCol_TabHovered] = ImVec4(0.25f, 0.30f, 0.35f, 1.00f);
		style.Colors[ImGuiCol_TabActive] = myTitleColor;
		style.Colors[ImGuiCol_TabUnfocused] = myTitleColor;
		style.Colors[ImGuiCol_TabUnfocusedActive] = myTitleColor;
		

		// sets fonts
		ImGuiIO& io = ImGui::GetIO();
		io.Fonts->Clear();
		
		io.Fonts->AddFontFromFileTTF("assets/fonts/OpenSans-Bold.ttf", 18.0f);
		
		io.FontDefault = io.Fonts->Fonts[0];
		
		ImGui_ImplDX12_InvalidateDeviceObjects();
		ImGui_ImplDX12_CreateDeviceObjects();
	
	}

}
