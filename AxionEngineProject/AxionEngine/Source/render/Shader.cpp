#include "axpch.h"
#include "Shader.h"

#include "AxionEngine/Source/render/Renderer.h"

#include "AxionEngine/Platform/directx/D12Shader.h"
#include "AxionEngine/Platform/opengl/OpenGL3Shader.h"

namespace Axion {

	////////////////////////////////////////////////////////////////////////////////
	///// Shader ///////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	Ref<Shader> Shader::create(const ShaderSpecification& spec) {
		
		switch (Renderer::getAPI()) {

			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet"); break; }
			case RendererAPI::DirectX12: { return std::make_shared<D12Shader>(spec); }
			case RendererAPI::OpenGL3: { return std::make_shared<OpenGL3Shader>(spec); }

		}
		return nullptr;

	}

	std::string Shader::readShaderFile(const std::string& filePath) {
		std::ifstream file(filePath);
		if (!file.is_open()) {
			AX_CORE_LOG_ERROR("Failed to open Shader file: {0}", filePath);
			return 0;
		}

		std::stringstream buffer;
		buffer << file.rdbuf();

		return buffer.str();
	}

}
