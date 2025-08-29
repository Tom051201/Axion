#pragma once

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/core/Timestep.h"
#include "AxionEngine/Source/core/Window.h"
#include "AxionEngine/Source/core/Cursor.h"

#include "AxionEngine/Source/events/Event.h"
#include "AxionEngine/Source/events/ApplicationEvent.h"
#include "AxionEngine/Source/events/KeyEvent.h"

#include "AxionEngine/Source/layers/LayerStack.h"
#include "AxionEngine/Source/imgui/ImGuiLayer.h"

#include "AxionEngine/Source/render/Renderer.h"

namespace Axion {

	struct ApplicationSpecification {
		std::function<void()> guiSyleSetter = nullptr;
	};

	class Application {
	public:

		Application(const ApplicationSpecification& spec = ApplicationSpecification());
		virtual ~Application();

		void run();
		void close();
		void closeOnError(const char* msg);

		void onEvent(Event& e);
		
		void pushLayer(Layer* layer);
		void pushOverlay(Layer* layer);
		void removeLayer(Layer* layer);
		void removeOverlay(Layer* layer);
		
		void setGraphicsBackend(RendererAPI api);

		void setWindowTitle(const std::string& title);
		void setWindowIcon(const std::string& path);
		void minimizeWindow();
		void maximizeOrRestoreWindow();

		static Application& get() { return *s_instance; }

		Window& getWindow() { return *m_window; }
		Cursor& getCursor() { return *m_cursor; }

	private:

		static Application* s_instance;
		ApplicationSpecification m_specification;
		bool m_firstStart = false;

		float m_lastFrameTime = 0.0f;

		Scope<Window> m_window;
		Scope<Cursor> m_cursor;
		bool m_running = true;
		LayerStack m_layerStack;
		ImGuiLayer* m_imGuiLayer;

		bool onWindowClose(Event& e);
		bool onKeyPressed(KeyPressedEvent& e);

	};

	// define in client
	Application* createApplication();

}
