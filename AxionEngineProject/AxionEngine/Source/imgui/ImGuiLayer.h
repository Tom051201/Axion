#pragma once

#include "AxionEngine/Source/layers/Layer.h"
#include "AxionEngine/Source/core/Window.h"

#include "AxionEngine/Source/events/MouseEvent.h"
#include "AxionEngine/Source/events/KeyEvent.h"
#include "AxionEngine/Source/events/ApplicationEvent.h"

#include "AxionEngine/Platform/directx/D12Context.h"

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

		D12Context* m_context = nullptr;
		uint32_t m_srvHeapIndex = 0;

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
