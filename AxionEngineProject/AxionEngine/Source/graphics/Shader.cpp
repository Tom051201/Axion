#include "axpch.h"
#include "Shader.h"

#include "AxionEngine/Source/graphics/Renderer.h"

#include "AxionEngine/Platform/directx12/DX12Shader.h"
#include "AxionEngine/Platform/directx11/DX11Shader.h"

namespace Axion {

	////////////////////////////////////////////////////////////////////////////////
	///// Shader ///////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	Ref<Shader> Shader::create(const ShaderSpecification& spec) {
		
		switch (Renderer::getAPI()) {

			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet"); break; }
			case RendererAPI::DirectX12: { return MakeRef<DX12Shader>(spec); }
			case RendererAPI::DirectX11: { return MakeRef<DX11Shader>(spec); }

		}
		return nullptr;

	}

	Ref<Shader> Shader::create(const ShaderSpecification& spec, const std::filesystem::path& filePath) {

		switch (Renderer::getAPI()) {

			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet"); break; }
			case RendererAPI::DirectX12: { return MakeRef<DX12Shader>(spec, filePath); }
			case RendererAPI::DirectX11: { return MakeRef<DX11Shader>(spec, filePath); }

		}
		return nullptr;

	}

	std::string Shader::readShaderFile(const std::filesystem::path& filePath) {
		std::ifstream file(filePath);
		if (!file.is_open()) {
			AX_CORE_LOG_ERROR("Failed to open Shader file: {0}", filePath.string());
			return 0;
		}

		std::stringstream buffer;
		buffer << file.rdbuf();

		return buffer.str();
	}

	ShaderBytecode Shader::compileToBytecode(const std::filesystem::path& filePath) {
		switch (Renderer::getAPI()) {

			case RendererAPI::None: { AX_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); break; }
			case RendererAPI::DirectX12: { return DX12Shader::compileToBytecode(filePath); }

		}
		return {};

	}

}
