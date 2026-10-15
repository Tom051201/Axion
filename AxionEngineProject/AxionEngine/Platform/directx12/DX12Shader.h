#pragma once

#include <string>
#include <filesystem>
#include <unordered_map>
#include <d3d12.h>
#include <wrl/client.h>
#include <dxcapi.h>

#include "AxionEngine/Source/graphics/Shader.h"

namespace Axion {

	constexpr const char* SHADER_MODEL_VS = "vs_6_5";
	constexpr const char* SHADER_MODEL_PS = "ps_6_5";

	class RenderContext;

	class DX12Shader : public Shader {
	public:

		DX12Shader();
		DX12Shader(const ShaderSpecification& spec);
		DX12Shader(const ShaderSpecification& spec, const std::filesystem::path& filePath);
		~DX12Shader() override;

		void release() override;

		void bind(RenderContext* renderContext) const override;
		void unbind(RenderContext* renderContext) const override;

		const std::string& getName() const override { return m_specification.name; }

		void compileFromFile(const std::filesystem::path& filePath) override;
		void recompile() override;
		void loadFromBytecode(const uint8_t* vsData, size_t vsSize, const uint8_t* psData, size_t psSize) override;
		void loadComputeFromBytecode(const uint8_t* csData, size_t csSize) override;

		static ShaderBytecode compileToBytecode(const std::filesystem::path& filePath);

		const Microsoft::WRL::ComPtr<IDxcBlob>& getVertexBlob() const { return m_vertexShaderBlob; }
		const Microsoft::WRL::ComPtr<IDxcBlob>& getPixelBlob() const { return m_pixelShaderBlob; }
		const Microsoft::WRL::ComPtr<IDxcBlob>& getComputeBlob() const { return m_computeShaderBlob; }

	private:

		ShaderSpecification m_specification;
		std::filesystem::path m_shaderFileLocation;

		Microsoft::WRL::ComPtr<IDxcBlob> m_vertexShaderBlob;
		Microsoft::WRL::ComPtr<IDxcBlob> m_pixelShaderBlob;
		Microsoft::WRL::ComPtr<IDxcBlob> m_computeShaderBlob;

		void compileStage(const std::string& source, const std::string& entryPoint, const std::string& target, Microsoft::WRL::ComPtr<IDxcBlob>& outblob);
		
	};

}
