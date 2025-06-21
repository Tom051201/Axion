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

	struct SystemInfo {
		std::string gpuName = "?";
		std::string gpuDriverVersion = "?";
		uint64_t vramMB= 0;

		std::string cpuName = "?";
		uint32_t cores = 0;

		uint64_t totalRamMB = 0;
		std::string os = "?";
	};

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

		SystemInfo& getSystemInfo() { return m_systemInfo; }
		const SystemInfo& getSystemInfo() const { return m_systemInfo; }

		static Application& get() { return *s_instance; }

		Window& getWindow() { return *m_window; }

	private:

		static Application* s_instance;

		float m_lastFrameTime = 0.0f;

		Scope<Window> m_window;
		bool m_running = true;
		LayerStack m_layerStack;
		ImGuiLayer* m_imGuiLayer;

		SystemInfo m_systemInfo;

		bool onWindowClose(Event& e);
		bool onKeyPressed(Event& e);

		void setupSystemInfo();

	};

	// define in client
	Application* createApplication();

}
