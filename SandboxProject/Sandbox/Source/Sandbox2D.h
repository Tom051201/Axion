#pragma once

#include "Axion.h"

#include "SandboxCamera.h"

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

	Axion::Ref<Axion::Material> m_material;
	Axion::Ref<Axion::Material> m_material2;
	Axion::Ref<Axion::ConstantBuffer> m_uploadBuffer;
	Axion::Ref<Axion::ConstantBuffer> m_uploadBuffer2;

};
