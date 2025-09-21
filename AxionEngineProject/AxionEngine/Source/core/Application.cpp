#include "axpch.h"
#include "Application.h"

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/render/Renderer.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionEngine/Source/scene/SceneManager.h"

namespace Axion {

	Application* Application::s_instance = nullptr;

	Application::Application(const ApplicationSpecification& spec)
		: m_specification(spec) {
		s_instance = this;

		m_window = Scope<Window>(Window::create());
		m_window->setEventCallback(AX_BIND_EVENT_FN(Application::onEvent));
		m_cursor = Scope<Cursor>(Cursor::create(m_window.get()));

		// Sets startup backend
		Renderer::setAPI(RendererAPI::DirectX12);
		Renderer::initialize(m_window.get(), AX_BIND_EVENT_FN(Application::onEvent));

		AssetManager::initialize();
		SceneManager::initialize(AX_BIND_EVENT_FN(Application::onEvent));
		ProjectManager::initialize(AX_BIND_EVENT_FN(Application::onEvent));

		m_imGuiLayer = new ImGuiLayer(m_specification.guiSyleSetter, m_specification.guiLayoutFilePath);
		pushOverlay(m_imGuiLayer);
	}

	Application::~Application() {
		removeOverlay(m_imGuiLayer);
		Renderer::release();

		ProjectManager::release();
		SceneManager::release();
		AssetManager::release();
	}

	void Application::onEvent(Event& e) {

		if (e.getEventType() == EventType::WindowResize) {
			auto& resizeEvent = static_cast<WindowResizeEvent&>(e);
			uint32_t width = resizeEvent.getWidth();
			uint32_t height = resizeEvent.getHeight();
			if (width > 0 && height > 0) {
				GraphicsContext::get()->resize(width, height);
			}
		}

		EventDispatcher dispatcher(e);
		dispatcher.dispatch<WindowCloseEvent>(AX_BIND_EVENT_FN(Application::onWindowClose));
		dispatcher.dispatch<KeyPressedEvent>(AX_BIND_EVENT_FN(Application::onKeyPressed));

		SceneManager::onEvent(e);
		ProjectManager::onEvent(e);
		AssetManager::onEvent(e);

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

			Renderer::prepareRendering();

			for (Layer* layer : m_layerStack) {
				layer->onUpdate(ts);
			}

			m_imGuiLayer->beginRender();
			for (Layer* layer : m_layerStack) {
				layer->onGuiRender();
			}
			m_imGuiLayer->endRender();

			Renderer::finishRendering();
		}

	}

	void Application::close() {
		onEvent(WindowCloseEvent());
		#ifdef AX_PLATFORM_WINDOWS
		PostQuitMessage(0);
		#endif
	}

	void Application::closeOnError(const char* msg) {
		close();
		AX_CORE_LOG_ERROR(msg);
	}

	bool Application::onWindowClose(Event& e) {
		m_running = false;
		return true;
	}

	bool Application::onKeyPressed(KeyPressedEvent& e) {
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

	void Application::setGraphicsBackend(RendererAPI api) {
		
		// shutting down and releasing
		Renderer::release();

		for (Layer* layer : m_layerStack) {
			layer->onDetach();
		}


		// reinitializing everything
		Renderer::setAPI(api);
		Renderer::initialize(m_window.get(), AX_BIND_EVENT_FN(Application::onEvent));

		m_layerStack.removeOverlay(m_imGuiLayer);

		for (Layer* layer : m_layerStack) {
			layer->onAttach();
		}

		m_imGuiLayer = new ImGuiLayer(m_specification.guiSyleSetter, m_specification.guiLayoutFilePath);
		pushOverlay(m_imGuiLayer);

	}

	void Application::setWindowTitle(const std::string& title) {
		m_window->setTitle(title);
	}

	void Application::setWindowIcon(const std::string& path) {
		m_window->setIcon(path);
	}

	void Application::minimizeWindow() {
		m_window->minimize();
	}

	void Application::maximizeOrRestoreWindow() {
		m_window->maximizeOrRestore();
	}

}
