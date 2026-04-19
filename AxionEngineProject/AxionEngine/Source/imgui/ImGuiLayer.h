#pragma once

#include "AxionEngine/Source/layers/Layer.h"
#include "AxionEngine/Source/core/Window.h"

#include "AxionEngine/Source/render/Renderer.h"

#include "AxionEngine/Source/events/MouseEvent.h"
#include "AxionEngine/Source/events/KeyEvent.h"
#include "AxionEngine/Source/events/ApplicationEvent.h"

#include "AxionEngine/Platform/directx/DX12Context.h"

namespace Axion {

	class ImGuiLayer : public Layer {
	public:

		ImGuiLayer(const std::function<void()>& styleSetupFunc, const std::filesystem::path& layoutFilePath);
		~ImGuiLayer();

		void onAttach() override;
		void onDetach() override;
		void onEvent(Event& ev) override;

		void beginRender();
		void endRender();

	private:

		RendererAPI m_activeAPI;
		std::function<void()> m_styleSetupFunc = nullptr;

		std::filesystem::path m_layoutFilePath;
		std::string m_layoutFilePathString;

		// directx12 specifics
		DX12Context* m_DX12Context = nullptr;
		uint32_t m_srvHeapIndex = 0;

		void setupD12();

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
