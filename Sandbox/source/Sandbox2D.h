#pragma once

#include "Axion.h"

class Sandbox2D : public Axion::Layer {
public:

	Sandbox2D();
	~Sandbox2D() = default;

	void onAttach() override;
	void onDetach() override;

	void onUpdate(Axion::Timestep ts) override;
	void onEvent(Axion::Event& e) override;

private:

	Axion::OrthographicCameraController m_cameraController;
	Axion::Ref<Axion::ConstantBuffer> m_cameraCB;

	Axion::Ref<Axion::Shader> m_shader;
	Axion::Ref<Axion::Mesh> m_quad;
	Axion::Ref<Axion::ConstantBuffer> m_quadCB;
	Axion::Mat4 m_quadTransform;

};
