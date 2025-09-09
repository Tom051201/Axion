#pragma once

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/render/Shader.h"

namespace Axion {

	class Material {
	public:

		Material(const std::string& name, const Vec4& color, const Ref<Shader>& shader);
		~Material();

		void release();

		void use() const;

		const Vec4& getColor() const { return m_color; }
		Vec4& getColor() { return m_color; }
		Ref<Shader> getShader() const { return m_shader; }
		const std::string& getName() const { return m_name; }


		static Ref<Material> create(const std::string& name, const Vec4& color, const Ref<Shader>& shader);

	private:

		std::string m_name = "Unknown Material";
		Vec4 m_color = Vec4::one();
		Ref<Shader> m_shader = nullptr;

	};

}
