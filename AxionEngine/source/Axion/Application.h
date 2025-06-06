#pragma once

#include "Core.h"
#include "core/Timestep.h"
#include "events/Event.h"
#include "events/ApplicationEvent.h"
#include "events/KeyEvent.h"
#include "Window.h"
#include "layers/LayerStack.h"

#include "render/OrthographicCamera.h"

namespace Axion {

	class Application {
	public:

		Application();
		virtual ~Application();

		void run();

		void onEvent(Event& e);

		void pushLayer(Layer* layer);
		void pushOverlay(Layer* layer);

		static Application& get() { return *s_instance; }

		Window& getWindow() { return *m_window; }

	private:

		static Application* s_instance;

		float m_lastFrameTime = 0.0f;

		Scope<Window> m_window;
		bool m_running = true;
		LayerStack m_layerStack;

		bool onWindowClose(Event& e);
		bool onKeyPressed(Event& e);

	};

	// define in client
	Application* createApplication();

}
