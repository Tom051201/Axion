#include "axpch.h"
#include "D12Material.h"

#include "AxionEngine/Source/render/Renderer.h"

namespace Axion {

	D12Material::D12Material(const std::string& name, const Vec4& color, const Ref<Shader>& shader)
		: m_name(name), m_color(color) {
		
		m_shader = std::dynamic_pointer_cast<D12Shader>(shader);
	}

	D12Material::~D12Material() {
		release();
	}

	void D12Material::release() {
	
	}

	void D12Material::use() const {
		m_shader->bind();

		Renderer::getSceneDataBuffer()->bind(0);
	}

}
