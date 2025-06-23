#pragma once

#include "Axion/core/Core.h"
#include "Axion/core/Timestep.h"
#include "Axion/core/Window.h"

#include "Axion/events/Event.h"
#include "Axion/events/ApplicationEvent.h"
#include "Axion/events/KeyEvent.h"

#include "Axion/layers/LayerStack.h"
#include "Axion/imgui/ImGuiLayer.h"

#include "Axion/render/OrthographicCamera.h"

namespace Axion {

	class Application {
	public:

		Application();
		virtual ~Application();

		void run();
		void close();

		void onEvent(Event& e);
		
		void pushLayer(Layer* layer);
		void pushOverlay(Layer* layer);
		void removeLayer(Layer* layer);
		void removeOverlay(Layer* layer);

		static Application& get() { return *s_instance; }

		Window& getWindow() { return *m_window; }

	private:

		static Application* s_instance;

		float m_lastFrameTime = 0.0f;

		Scope<Window> m_window;
		bool m_running = true;
		LayerStack m_layerStack;
		ImGuiLayer* m_imGuiLayer;

		bool onWindowClose(Event& e);
		bool onKeyPressed(Event& e);

	};

	// define in client
	Application* createApplication();

}
