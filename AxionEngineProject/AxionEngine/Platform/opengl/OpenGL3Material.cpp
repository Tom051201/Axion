#include "axpch.h"
#include "OpenGL3Material.h"

namespace Axion {

	OpenGL3Material::OpenGL3Material(const std::string& name, const Vec4& color, const Ref<Shader>& shader)
		: m_name(name), m_color(color) {

		m_shader = std::dynamic_pointer_cast<OpenGL3Shader>(shader);
	}

	OpenGL3Material::~OpenGL3Material() {
		release();
	}

	void OpenGL3Material::release() {

	}

	void OpenGL3Material::use() const {
		m_shader->bind();
	}

}
