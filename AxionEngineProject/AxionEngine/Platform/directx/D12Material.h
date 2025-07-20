#pragma once

#include "AxionEngine/Source/render/Material.h"

#include "AxionEngine/Platform/directx/D12Shader.h"

namespace Axion {

	class D12Material : public Material {
	public:

		D12Material(const std::string& name, const Vec4& color, const Ref<Shader>& shader);
		~D12Material();

		void release() override;

		void use() const override;

		const Vec4& getColor() const override { return m_color; }
		Ref<Shader> getShader() const override { return m_shader; }

		const std::string& getName() const override { return m_name; }

	private:

		std::string m_name;
		Vec4 m_color;
		Ref<D12Shader> m_shader;


	};

}
