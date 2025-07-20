#pragma once

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/render/Shader.h"

namespace Axion {

	class Material {
	public:

		virtual ~Material() = default;

		virtual void release() = 0;

		virtual void use() const = 0;

		virtual const Vec4& getColor() const = 0;
		virtual Ref<Shader> getShader() const = 0;

		virtual const std::string& getName() const = 0;


		static Ref<Material> create(const std::string& name, const Vec4& color, const Ref<Shader>& shader);

	};

}
