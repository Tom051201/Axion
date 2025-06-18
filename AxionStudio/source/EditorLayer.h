#pragma once

#include "Axion.h"

// TEMP
#include "platform/directx/D12Context.h"

namespace Axion {

	class EditorLayer : public Layer {
	public:

		EditorLayer();
		~EditorLayer() = default;

		void onAttach() override;
		void onDetach() override;

		void onUpdate(Axion::Timestep ts) override;
		void onEvent(Axion::Event& e) override;
		void onGuiRender() override;

	private:

		OrthographicCameraController m_cameraController;

		Ref<Texture2D> m_texture;

		Ref<FrameBuffer> m_frameBuffer;

		// TEMP
		Ref<ConstantBuffer> m_buffer1;
		Ref<ConstantBuffer> m_buffer2;
		D12Context* m_context = nullptr;

	};

}
