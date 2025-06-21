#include "axpch.h"
#include "Application.h"

#include "Axion/render/Renderer.h"

#include "platform/directx/D12Context.h"

// TEMP
#include <intrin.h>
#include <Windows.h>
#include <winternl.h>

namespace Axion {

	Application* Application::s_instance = nullptr;

	Application::Application() {
		s_instance = this;

		m_window = Scope<Window>(Window::create());
		m_window->setEventCallback(BIND_EVENT_FN(Application::onEvent));

		GraphicsContext::set(new D12Context());

		Renderer::initialize(m_window.get());
		
		m_imGuiLayer = new ImGuiLayer();
		pushOverlay(m_imGuiLayer);

		// setup system info
		setupSystemInfo();

	}

	Application::~Application() {
		removeOverlay(m_imGuiLayer);
		Renderer::release();
	}


	void Application::onEvent(Event& e) {

		if (e.getEventType() == EventType::WindowResize) {
			auto& resizeEvent = static_cast<WindowResizeEvent&>(e);
			uint32_t width = resizeEvent.getWidth();
			uint32_t height = resizeEvent.getHeight();
			if (width > 0 && height > 0) {
				static_cast<D12Context*>(GraphicsContext::get()->getNativeContext())->resize(width, height);
			}
		}

		EventDispatcher dispatcher(e);
		dispatcher.dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::onWindowClose));
		dispatcher.dispatch<KeyPressedEvent>(BIND_EVENT_FN(Application::onKeyPressed));

		for (auto it = m_layerStack.end(); it != m_layerStack.begin();) {
			(*--it)->onEvent(e);
			if (e.handled) break;
		}
	}

	void Application::run() {
		MSG msg = {};

		// high resolution timing for windows
		LARGE_INTEGER frequency;
		LARGE_INTEGER lastTime, currentTime;
		QueryPerformanceFrequency(&frequency);
		QueryPerformanceCounter(&lastTime);

		while (m_running) {

			m_window->onUpdate();

			// time calculation
			QueryPerformanceCounter(&currentTime);
			Timestep ts = static_cast<float>(currentTime.QuadPart - lastTime.QuadPart) / frequency.QuadPart;
			lastTime = currentTime;
			
			for (Layer* layer : m_layerStack) {
				layer->onUpdate(ts);
			}

			m_imGuiLayer->beginRender();
			for (Layer* layer : m_layerStack) {
				if (layer->m_active) layer->onGuiRender();
			}
			m_imGuiLayer->endRender();

			Axion::Renderer::endScene();
			Axion::Renderer::present();
		}

	}

	void Application::close() {
		onEvent(WindowCloseEvent());
		#ifdef AX_PLATFORM_WINDOWS
		PostQuitMessage(0);
		#endif
	}

	bool Application::onWindowClose(Event& e) {
		m_running = false;
		return true;
	}

	bool Application::onKeyPressed(Event& e) {
		return true;
	}

	void Application::pushLayer(Layer* layer) {
		m_layerStack.pushLayer(layer);
		layer->onAttach();
	}

	void Application::pushOverlay(Layer* layer) {
		m_layerStack.pushOverlay(layer);
		layer->onAttach();
	}

	void Application::removeLayer(Layer* layer) {
		m_layerStack.removeLayer(layer);
		layer->onDetach();
	}

	void Application::removeOverlay(Layer* layer) {
		m_layerStack.removeOverlay(layer);
		layer->onDetach();
	}
	
	void Application::setupSystemInfo() {	// TODO: do not make this here os specific!
		// cpu
		char cpuBrand[0x40] = {};
		int cpuInfo[4] = {};
		__cpuid(cpuInfo, 0x80000000);
		unsigned int maxExtended = cpuInfo[0];

		if (maxExtended >= 0x80000004) {
			__cpuid((int*)cpuBrand, 0x80000002);
			__cpuid((int*)(cpuBrand + 16), 0x80000003);
			__cpuid((int*)(cpuBrand + 32), 0x80000004);
			m_systemInfo.cpuName = cpuBrand;
		}

		// cores
		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);
		m_systemInfo.cores = sysInfo.dwNumberOfProcessors;

		// ram
		MEMORYSTATUSEX memInfo = {};
		memInfo.dwLength = sizeof(memInfo);
		if (GlobalMemoryStatusEx(&memInfo)) {
			m_systemInfo.totalRamMB = static_cast<uint64_t>(memInfo.ullTotalPhys / (1024 * 1024));
		}

		// os
		typedef LONG(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
		HMODULE hMod = ::GetModuleHandleW(L"ntdll.dll");
		if (hMod) {
			RtlGetVersionPtr fxPtr = (RtlGetVersionPtr)::GetProcAddress(hMod, "RtlGetVersion");
			if (fxPtr != nullptr) {
				RTL_OSVERSIONINFOW rovi = { 0 };
				rovi.dwOSVersionInfoSize = sizeof(rovi);
				if (fxPtr(&rovi) == 0) {
					wchar_t buf[128];
					swprintf_s(buf, 128, L"Windows %d.%d (Build %d)", rovi.dwMajorVersion, rovi.dwMinorVersion, rovi.dwBuildNumber);
					std::wstring wstr(buf);
					m_systemInfo.os = std::string(wstr.begin(), wstr.end());
				}
			}
		}
	}

}

