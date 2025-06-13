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

	ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer") {
		
	}

	ImGuiLayer::~ImGuiLayer() {

	}

	void ImGuiLayer::onAttach() {

		m_context = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext());

		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors = 1;  // Typically 1 for font texture
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		desc.NodeMask = 0;

		HRESULT hr = m_context->getDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_srvDescHeap));
		AX_THROW_IF_FAILED_HR(hr, "Failed to create ImGui descriptor heap");


		IMGUI_CHECKVERSION();
		ImGui_ImplWin32_EnableDpiAwareness();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		ImGui::StyleColorsDark();
	
		ImGui_ImplWin32_Init((HWND)Application::get().getWindow().getNativeHandle());
		ImGui_ImplDX12_Init(
			m_context->getDevice(),
			2,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			m_srvDescHeap.Get(),
			m_srvDescHeap->GetCPUDescriptorHandleForHeapStart(),
			m_srvDescHeap->GetGPUDescriptorHandleForHeapStart()
		);

		AX_CORE_LOG_TRACE("ImGui layer attached");
	}

	void ImGuiLayer::onDetach() {
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyPlatformWindows();
		ImGui::DestroyContext();
		m_srvDescHeap.Reset();

		AX_CORE_LOG_TRACE("ImGuiLayer detatched");
	}

//	void ImGuiLayer::onUpdate(Timestep ts) {
//		if (m_active) {					// TODO: find workaround to prevent crash when viewport is inside actual window
//			ImGui_ImplDX12_NewFrame();
//			ImGui_ImplWin32_NewFrame();
//			ImGui::NewFrame();
//
//			static bool show = true;
//			ImGui::ShowDemoWindow(&show);
//
//			ImGui::Render();
//
//			ID3D12DescriptorHeap* heaps[] = { m_srvDescHeap.Get() };
//			m_context->getCommandList()->SetDescriptorHeaps(_countof(heaps), heaps);
//			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_context->getCommandList());
//
//			// for multiple viewports
//			ImGuiIO& io = ImGui::GetIO();
//			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable && !ImGui::GetIO().WantSaveIniSettings) {
//				ImGui::UpdatePlatformWindows();
//				ImGui::RenderPlatformWindowsDefault(nullptr, m_context->getCommandQueue());
//			}
//		}
//		
//	}
	
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

			ID3D12DescriptorHeap* heaps[] = { m_srvDescHeap.Get() };
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

}
