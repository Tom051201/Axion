#pragma once
#include "axpch.h"

#include "AxionEngine/Source/render/Shader.h"

namespace Axion {

	constexpr const char* SHADER_MODEL_VS = "vs_5_1";
	constexpr const char* SHADER_MODEL_PS = "ps_5_1";

	class D12Shader : public Shader {
	public:

		D12Shader();
		D12Shader(const ShaderSpecification& spec);
		D12Shader(const ShaderSpecification& spec, const std::filesystem::path& filePath);
		~D12Shader() override;

		void release() override;

		void bind() const override;
		void unbind() const override;

		const std::string& getName() const override { return m_specification.name; }

		void compileFromFile(const std::filesystem::path& filePath) override;
		void recompile() override;
		void loadFromBytecode(const uint8_t* vsData, size_t vsSize, const uint8_t* psData, size_t psSize) override;

		static ShaderBytecode compileToBytecode(const std::filesystem::path& filePath);

		int getBindPoint(const std::string& name) const override;
		uint32_t getTextureTableBindSlot() const override { return m_textureTableSlot; }

		const Microsoft::WRL::ComPtr<ID3DBlob>& getVertexBlob() const { return m_vertexShaderBlob; }
		const Microsoft::WRL::ComPtr<ID3DBlob>& getPixelBlob() const { return m_pixelShaderBlob; }
		ID3D12RootSignature* getRootSignature() const { return m_rootSignature.Get(); }

	private:

		ShaderSpecification m_specification;
		std::filesystem::path m_shaderFileLocation;

		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;

		Microsoft::WRL::ComPtr<ID3DBlob> m_vertexShaderBlob;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pixelShaderBlob;

		std::unordered_map<std::string, uint32_t> m_resourceMap;
		uint32_t m_textureTableSlot = 0;

		void compileStage(const std::string& source, const std::string& entryPoint, const std::string& target, Microsoft::WRL::ComPtr<ID3DBlob>& outblob);
		
		void createRootSignature();
	};

}
