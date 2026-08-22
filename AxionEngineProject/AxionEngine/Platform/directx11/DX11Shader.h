#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "AxionEngine/Source/graphics/Shader.h"

namespace Axion {

	class DX11Shader : public Shader {
	public:

		DX11Shader();
		DX11Shader(const ShaderSpecification& spec);
		DX11Shader(const ShaderSpecification& spec, const std::filesystem::path& filePath);
		~DX11Shader() override;

		void release() override;

		void bind() const override;
		void unbind() const override;

		const std::string& getName() const override { return m_specification.name; }

		void compileFromFile(const std::filesystem::path& filePath) override;
		void recompile() override;
		void loadFromBytecode(const uint8_t* vsData, size_t vsSize, const uint8_t* psData, size_t psSize) override;

		int getBindPoint(const std::string& name) const override;
		uint32_t getTextureTableBindSlot() const override { return m_textureTableSlot; }

		const Microsoft::WRL::ComPtr<ID3DBlob>& getVertexBlob() const { return m_vertexShaderBlob; }

	private:

		ShaderSpecification m_specification;
		std::filesystem::path m_shaderFileLocation;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;

		Microsoft::WRL::ComPtr<ID3DBlob> m_vertexShaderBlob;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pixelShaderBlob;

		std::unordered_map<std::string, uint32_t> m_resourceMap;
		uint32_t m_textureTableSlot = 0;

		void compileStage(const std::string& source, const std::string& entryPoint, const std::string& target, Microsoft::WRL::ComPtr<ID3DBlob>& outblob);
		void reflectAndCreateShaders();

	};

}
