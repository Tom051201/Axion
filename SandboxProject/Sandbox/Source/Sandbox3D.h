#pragma once

#include "Axion.h"

#include "SandboxCamera.h"

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

	SandboxCamera m_camera;

	Axion::Mat4 m_transform;
	Axion::Ref<Axion::Mesh> m_mesh;
	Axion::Ref<Axion::Material> m_material;
	Axion::Ref<Axion::ConstantBuffer> m_buffer;
	
	Axion::Mat4 m_transform1;
	Axion::Ref<Axion::Material> m_material1;
	Axion::Ref<Axion::ConstantBuffer> m_buffer1;

};
