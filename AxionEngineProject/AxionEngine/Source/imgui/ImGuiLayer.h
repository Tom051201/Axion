#pragma once

#include "AxionEngine/Source/layers/Layer.h"
#include "AxionEngine/Source/core/Window.h"

#include "AxionEngine/Source/render/Renderer.h"

#include "AxionEngine/Source/events/MouseEvent.h"
#include "AxionEngine/Source/events/KeyEvent.h"
#include "AxionEngine/Source/events/ApplicationEvent.h"

#include "AxionEngine/Platform/directx/D12Context.h"
#include "AxionEngine/Platform/opengl/OpenGL3Context.h"

namespace Axion {

	class ImGuiLayer : public Layer {
	public:

		ImGuiLayer();
		~ImGuiLayer();

		void onAttach() override;
		void onDetach() override;
		void onEvent(Event& ev) override;

		void beginRender();
		void endRender();

	private:

		RendererAPI m_activeAPI;

		// directx12 specifics
		D12Context* m_d12Context = nullptr;
		uint32_t m_srvHeapIndex = 0;

		// opengl3 specifics
		OpenGL3Context* m_gl3Context = nullptr;

		void setupD12();
		void setupOpenGL();

		bool onMouseButtonPressedEvent(MouseButtonPressedEvent& e);
		bool onMouseButtonReleasedEvent(MouseButtonReleasedEvent& e);
		bool onMouseMovedEvent(MouseMovedEvent& e);
		bool onMouseScrolledEvent(MouseScrolledEvent& e);
		bool onKeyPressedEvent(KeyPressedEvent& e);
		bool onKeyReleasedEvent(KeyReleasedEvent& e);
		bool onKeyTypedEvent(KeyTypedEvent& e);
		bool onWindowResizeEvent(WindowResizeEvent& e);
		bool onWindowCloseEvent(WindowCloseEvent& e);

		void setStyle();

	};

}
