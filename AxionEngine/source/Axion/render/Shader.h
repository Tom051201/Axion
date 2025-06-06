#pragma once

#include "axpch.h"

namespace Axion {

	class Shader {
	public:

		virtual ~Shader() = default;

		virtual void bind() const = 0;
		virtual void unbind() const = 0;
		virtual const std::string& getName() const = 0;

		virtual void compileFromFile(const std::string& name, const std::string& vertexPath, const std::string& pixelPath) = 0;
		virtual void compileFromString(const std::string& name, const std::string& vertexSrc, const std::string& pixelSrc) = 0;

		static Shader* create(const std::string& name, const std::string& vertexSrc, const std::string& pixelSrc);
		static std::string readShaderFile(const std::string& filePath);
	};

}
