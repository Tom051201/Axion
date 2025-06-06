#include "axpch.h"
#include "Shader.h"

#include "Renderer.h"

#include "platform/directx/D12Shader.h"

namespace Axion {

	Shader* Shader::create(const std::string& name, const std::string& vertexSrc, const std::string& pixelSrc) {
		
		switch (Renderer::getAPI()) {

			case RendererAPI::API::None : { AX_ASSERT(false, "None is not supported yet"); }
			case RendererAPI::API::Direct3D12: { return new D12Shader(); }

		}
		return nullptr;

	}

	std::string Shader::readShaderFile(const std::string& filePath) {
		std::ifstream file(filePath);
		if (!file.is_open()) {
			AX_CORE_LOG_ERROR("Failed to open Shader file: {0}", filePath);
			return nullptr;
		}

		std::stringstream buffer;
		buffer << file.rdbuf();

		return buffer.str();
	}

}
