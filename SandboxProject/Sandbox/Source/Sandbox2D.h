#pragma once

#include "Axion.h"

// TEMP
#include "AxionEngine/Platform/directx/D12Context.h"

class Sandbox2D : public Axion::Layer {
public:

	Sandbox2D();
	~Sandbox2D() = default;

	void onAttach() override;
	void onDetach() override;

	void onUpdate(Axion::Timestep ts) override;
	void onEvent(Axion::Event& e) override;
	void onGuiRender() override;

private:

	Axion::OrthographicCameraController m_cameraController;

	Axion::Ref<Axion::Texture2D> m_texture;

	Axion::Ref<Axion::FrameBuffer> m_frameBuffer;

	// TEMP
	Axion::Ref<Axion::ConstantBuffer> m_buffer1;
	Axion::Ref<Axion::ConstantBuffer> m_buffer2;
	Axion::D12Context* m_context = nullptr;

};
