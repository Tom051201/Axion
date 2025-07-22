#pragma once

#include "Axion.h"

class FrameBufferExample : public Axion::Layer {
public:

	FrameBufferExample();
	~FrameBufferExample() = default;

	void onAttach() override;
	void onDetach() override;

	void onUpdate(Axion::Timestep ts) override;
	void onEvent(Axion::Event& e) override;
	void onGuiRender() override;

private:

	Axion::OrthographicCameraController m_cameraController;

	Axion::Ref<Axion::FrameBuffer> m_frameBuffer;
	Axion::Ref<Axion::Material> m_material;
	Axion::Ref<Axion::Material> m_material2;
	Axion::Ref<Axion::ConstantBuffer> m_uploadBuffer;
	Axion::Ref<Axion::ConstantBuffer> m_uploadBuffer2;

};
