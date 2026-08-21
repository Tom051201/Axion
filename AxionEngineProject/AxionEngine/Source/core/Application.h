#pragma once

#include <filesystem>

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/core/Timestep.h"
#include "AxionEngine/Source/core/Window.h"
#include "AxionEngine/Source/core/Cursor.h"
#include "AxionEngine/Source/core/AssetLoader.h"
#include "AxionEngine/Source/graphics/GraphicsContext.h"
#include "AxionEngine/Source/graphics/Renderer.h"
#include "AxionEngine/Source/events/Event.h"
#include "AxionEngine/Source/events/ApplicationEvent.h"
#include "AxionEngine/Source/events/KeyEvent.h"
#include "AxionEngine/Source/layers/LayerStack.h"

namespace Axion {

	struct ApplicationCommandLineArgs {
		int count = 0;
		char** args = nullptr;

		const char* operator[](int index) const {
			return args[index];
		}
	};

	struct ApplicationSpecification {
		WindowProperties windowProperties;
		AssetLoader* assetLoader = nullptr;
		ApplicationCommandLineArgs commandLineArgs;
		bool vsync = true;
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
		void setWindowIcon(const std::filesystem::path& path);
		void minimizeWindow();
		void maximizeOrRestoreWindow();

		void activeVsync() const { GraphicsContext::get()->activateVsync(); }
		void deactiveVsync() const { GraphicsContext::get()->deactivateVsync(); }

		static Application& get() { return *s_instance; }

		Window& getWindow() { return *m_window; }
		Cursor& getCursor() { return *m_cursor; }

		const ApplicationCommandLineArgs& getCommandLineArgs() const { return m_specification.commandLineArgs; }

	private:

		static Application* s_instance;
		ApplicationSpecification m_specification;

		Scope<Window> m_window;
		Scope<Cursor> m_cursor;
		LayerStack m_layerStack;
		bool m_running = true;

		EventReply onWindowClose(WindowCloseEvent& e);
		EventReply onKeyPressed(KeyPressedEvent& e);

	};

	// define in client
	Application* createApplication(ApplicationCommandLineArgs args);

}
