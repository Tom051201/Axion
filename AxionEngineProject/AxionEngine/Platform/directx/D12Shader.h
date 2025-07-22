#pragma once
#include "axpch.h"

#include "AxionEngine/Source/render/Shader.h"

namespace Axion {

	constexpr const char* SHADER_MODEL_VS = "vs_5_0";
	constexpr const char* SHADER_MODEL_PS = "ps_5_0";

	class D12Shader : public Shader {
	public:

		D12Shader();
		D12Shader(const ShaderSpecification& spec);
		~D12Shader() override;

		void release() override;

		void bind() const override;
		void unbind() const override;

		const std::string& getName() const override { return m_specification.name; }

		void compileFromFile(const std::string& filePath) override;

		const Microsoft::WRL::ComPtr<ID3DBlob>& getVertexBlob() const { return m_vertexShaderBlob; }
		const Microsoft::WRL::ComPtr<ID3DBlob>& getPixelBlob() const { return m_pixelShaderBlob; }
		const ID3D12RootSignature* getRootSignature() const { return m_rootSignature.Get(); }
		const ID3D12PipelineState* getPipelineState() const { return m_pipelineState.Get(); }

	private:

		ShaderSpecification m_specification;

		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;

		Microsoft::WRL::ComPtr<ID3DBlob> m_vertexShaderBlob;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pixelShaderBlob;

		void compileStage(const std::string& source, const std::string& entryPoint, const std::string& target, Microsoft::WRL::ComPtr<ID3DBlob>& outblob);
		
		void createRootSignature();
		void createPipelineState();
	};

}
