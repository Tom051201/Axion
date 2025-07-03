#include "axpch.h"
#include "Shader.h"

#include "AxionEngine/Source/render/Renderer.h"

#include "AxionEngine/Platform/directx/D12Shader.h"

namespace Axion {

	////////////////////////////////////////////////////////////////////////////////
	///// Shader ///////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	Ref<Shader> Shader::create(const std::string& name) {
		
		switch (Renderer::getAPI()) {

			case RendererAPI::API::None : { AX_ASSERT(false, "None is not supported yet"); }
			case RendererAPI::API::Direct3D12: { return std::make_shared<D12Shader>(name); }

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

	////////////////////////////////////////////////////////////////////////////////
	///// ShaderLibrary ////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	void ShaderLibrary::release() {
		for (auto& [name, shader] : m_shaders) {
			shader->release();
		}
	}

	void ShaderLibrary::add(const Ref<Shader>& shader) {

		auto& name = shader->getName();
		AX_CORE_ASSERT(m_shaders.find(name) == m_shaders.end(), "Shader already exists");
		m_shaders[name] = shader;

	}

	Ref<Shader> ShaderLibrary::load(const std::string& filePath) {
		auto shader = Shader::create(getNameFromFile(filePath));
		shader->compileFromFile(filePath);
		add(shader);

		return shader;
	}

	Ref<Shader> ShaderLibrary::load(const std::string& name, const std::string& filePath) {
		auto shader = Shader::create(name);
		shader->compileFromFile(filePath);
		add(shader);

		return shader;
	}

	Ref<Shader> ShaderLibrary::get(const std::string& name) {
		AX_CORE_ASSERT(m_shaders.find(name) != m_shaders.end(), "Shader not found");
		return m_shaders[name];
	}

	std::string ShaderLibrary::getNameFromFile(const std::string& filePath) {
		auto lastSlash = filePath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = filePath.rfind('.');
		auto count = lastDot == std::string::npos ? filePath.size() - lastSlash : lastDot - lastSlash;
		return filePath.substr(lastSlash, count);
	}

}
