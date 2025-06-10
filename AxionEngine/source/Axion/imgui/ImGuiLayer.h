#pragma once

#include "Axion/layers/Layer.h"
#include "Axion/core/Window.h"

#include "Axion/events/MouseEvent.h"
#include "Axion/events/KeyEvent.h"
#include "Axion/events/ApplicationEvent.h"

#include "platform/directx/D12Context.h"

namespace Axion {

	class ImGuiLayer : public Layer {
	public:

		ImGuiLayer();
		~ImGuiLayer();

		void onAttach() override;
		void onDetach() override;
		void onUpdate(Timestep ts) override;
		void onEvent(Event& ev) override;

	private:

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvDescHeap;
		D12Context* m_context = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext());

		bool onMouseButtonPressedEvent(MouseButtonPressedEvent& e);
		bool onMouseButtonReleasedEvent(MouseButtonReleasedEvent& e);
		bool onMouseMovedEvent(MouseMovedEvent& e);
		bool onMouseScrolledEvent(MouseScrolledEvent& e);
		bool onKeyPressedEvent(KeyPressedEvent& e);
		bool onKeyReleasedEvent(KeyReleasedEvent& e);
		bool onKeyTypedEvent(KeyTypedEvent& e);
		bool onWindowResizeEvent(WindowResizeEvent& e);
		bool onWindowCloseEvent(WindowCloseEvent& e);

	};

}
