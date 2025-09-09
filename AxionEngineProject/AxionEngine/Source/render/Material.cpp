#include "axpch.h"
#include "Material.h"

#include "AxionEngine/Source/render/Renderer.h"

namespace Axion {

	Ref<Material> Material::create(const std::string& name, const Vec4& color, const Ref<Shader>& shader) {
		return std::make_shared<Material>(name, color, shader);
	}

	Material::Material(const std::string& name, const Vec4& color, const Ref<Shader>& shader)
	: m_name(name), m_color(color), m_shader(shader) {}

	Material::~Material() {
		release();
	}

	void Material::release() {}

	void Material::use() const {
		m_shader->bind();
		Renderer::getSceneDataBuffer()->bind(0);
	}

}
