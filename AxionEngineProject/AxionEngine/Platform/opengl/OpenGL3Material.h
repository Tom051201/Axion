#pragma once

#include "AxionEngine/Source/render/Material.h"
#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/core/Math.h"

#include "AxionEngine/Platform/opengl/OpenGL3Shader.h"

namespace Axion {

	class OpenGL3Material : public Material {
	public:

		OpenGL3Material(const std::string& name, const Vec4& color, const Ref<Shader>& shader);
		~OpenGL3Material();

		void release() override;

		void use() const override;

		const Vec4& getColor() const override { return m_color; }
		Ref<Shader> getShader() const override { return m_shader; }

		const std::string& getName() const override { return m_name; }

	private:

		std::string m_name;
		Vec4 m_color;
		Ref<OpenGL3Shader> m_shader;


	};

}
