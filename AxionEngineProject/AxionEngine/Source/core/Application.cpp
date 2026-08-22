#include "axpch.h"
#include "Application.h"

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/EngineAssets.h"
#include "AxionEngine/Source/core/JobSystem.h"
#include "AxionEngine/Source/graphics/Renderer.h"
#include "AxionEngine/Source/graphics/Renderer2D.h"
#include "AxionEngine/Source/graphics/Renderer3D.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionEngine/Source/scene/SceneManager.h"
#include "AxionEngine/Source/audio/AudioManager.h"
#include "AxionEngine/Source/physics/PhysicsSystem.h"
#include "AxionEngine/Source/scripting/ScriptEngine.h"

namespace Axion {

	Application* Application::s_instance = nullptr;

	Application::Application(const ApplicationSpecification& spec)
		: m_specification(spec) {
		s_instance = this;

		JobSystem::initialize();

		m_window = Scope<Window>(Window::create(spec.windowProperties));
		m_window->setEventCallback(AX_BIND_EVENT_FN(Application::onEvent));
		m_cursor = Scope<Cursor>(Cursor::create(m_window.get()));

		// -- Setup script engine --
		ScriptEngine::initialize();

		// Sets startup backend
		Renderer::setAPI(RendererAPI::DirectX11);
		Renderer::initialize(m_window.get(), AX_BIND_EVENT_FN(Application::onEvent));

		EngineAssets::initialize();

		Renderer2D::initialize();
		Renderer3D::initialize();

		AudioManager::initialize();
		PhysicsSystem::initialize();

		AssetManager::initialize(m_specification.assetLoader);
		SceneManager::initialize(AX_BIND_EVENT_FN(Application::onEvent));
		ProjectManager::initialize(AX_BIND_EVENT_FN(Application::onEvent));

		if (m_specification.vsync) { activeVsync(); }
		else { deactiveVsync(); }
	}

	Application::~Application() {
		m_layerStack.clear();

		SceneManager::shutdown();

		AssetManager::shutdown();
		ProjectManager::shutdown();
		AudioManager::shutdown();
		PhysicsSystem::shutdown();

		Renderer3D::shutdown();
		Renderer2D::shutdown();

		EngineAssets::shutdown();
		Renderer::shutdown();

		ScriptEngine::shutdown();

		JobSystem::shutdown();

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
			if (ts > 0.1f) ts = 0.1f;
			lastTime = currentTime;

			Renderer::prepareRendering();

			for (Layer* layer : m_layerStack) {
				layer->onUpdate(ts);
			}

			for (Layer* layer : m_layerStack) {
				layer->onGuiRender();
			}

			Renderer::finishRendering();
		}

	}

	void Application::close() {
		WindowCloseEvent event;
		onEvent(event);
		#ifdef AX_PLATFORM_WINDOWS
		PostQuitMessage(0);
		#endif
	}

	void Application::closeOnError(const char* msg) {
		close();
		AX_CORE_LOG_ERROR(msg);
	}

	EventReply Application::onWindowClose(WindowCloseEvent& e) {
		m_running = false;
		return EventReply::handled();
	}

	EventReply Application::onKeyPressed(KeyPressedEvent& e) {
		return EventReply::handled();
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
		// TODO: Add Graphics Backend Hot-Swapping
		AX_CORE_ASSERT(false, "Setting a Graphics Backend is not supported yet!");
	}

	void Application::setWindowTitle(const std::string& title) {
		m_window->setTitle(title);
	}

	void Application::setWindowIcon(const std::filesystem::path& path) {
		m_window->setIcon(path);
	}

	void Application::minimizeWindow() {
		m_window->minimize();
	}

	void Application::maximizeOrRestoreWindow() {
		m_window->maximizeOrRestore();
	}

}
