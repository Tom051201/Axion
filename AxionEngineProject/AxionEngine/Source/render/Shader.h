#pragma once
#include "axpch.h"

#include "AxionEngine/Source/render/Formats.h"
#include "AxionEngine/Source/render/Buffers.h"

namespace Axion {

	struct ShaderSpecification {
		std::string name = "Unset";

		uint32_t batchTextures = 1;
	};

	////////////////////////////////////////////////////////////////////////////////
	///// Shader ///////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	class Shader {
	public:

		virtual ~Shader() = default;

		virtual void release() = 0;

		virtual void bind() const = 0;
		virtual void unbind() const = 0;

		virtual const std::string& getName() const = 0;

		virtual void compileFromFile(const std::string& filePath) = 0;
		virtual void recompile() = 0;

		virtual int getBindPoint(const std::string& name) const = 0;
		virtual uint32_t getTextureTableBindSlot() const = 0;


		static Ref<Shader> create(const ShaderSpecification& spec);
		static Ref<Shader> create(const ShaderSpecification& spec, const std::string& filePath);
		static std::string readShaderFile(const std::string& filePath);
	};

}
