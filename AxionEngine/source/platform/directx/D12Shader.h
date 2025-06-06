#pragma once

#include "axpch.h"

#include "Axion/render/Shader.h"

namespace Axion {

	constexpr const char* SHADER_MODEL_VS = "vs_5_0";
	constexpr const char* SHADER_MODEL_PS = "ps_5_0";

	class D12Shader : public Shader {
	public:

		D12Shader();
		D12Shader(const std::string& name);

		void bind() const override;
		void unbind() const override;

		inline const std::string& getName() const override { return m_name; }

		void compileFromFile(const std::string& vertexPath, const std::string& pixelPath) override;
		void compileFromString(const std::string& vertexSrc, const std::string& pixelSrc) override;

		inline const Microsoft::WRL::ComPtr<ID3DBlob>& getVertexBlob() const { return m_vertexShaderBlob; }
		inline const Microsoft::WRL::ComPtr<ID3DBlob>& getPixelBlob() const { return m_pixelShaderBlob; }
		inline const ID3D12RootSignature* getRootSignature() const { return m_rootSignature.Get(); }
		inline const ID3D12PipelineState* getPipelineState() const { return m_pipelineState.Get(); }

	private:

		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;

		Microsoft::WRL::ComPtr<ID3DBlob> m_vertexShaderBlob;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pixelShaderBlob;

		std::string m_name;

		void compileStage(const std::string& source, const std::string& entryPoint, const std::string& target, Microsoft::WRL::ComPtr<ID3DBlob>& outblob);
		
		void createRootSignature();
		void createPipelineState();
	};

}
