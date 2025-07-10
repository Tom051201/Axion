#pragma once

#include "Axion.h"

class Sandbox3D : public Axion::Layer {
public:

	Sandbox3D();
	~Sandbox3D();

	void onAttach() override;
	void onDetach() override;

	void onUpdate(Axion::Timestep ts) override;
	void onEvent(Axion::Event& e) override;
	void onGuiRender() override;

private:

	Axion::OrthographicCameraController m_camController;

	Axion::Ref<Axion::ConstantBuffer> m_buffer;

};
