#include "axpch.h"
#include "D12Shader.h"

#include "Axion/render/DataTypes.h"
#include "Axion/render/GraphicsContext.h"

#include "platform/directx/D12Context.h"

namespace Axion {

	D12Shader::D12Shader() {
		m_vertexShaderBlob = nullptr;
		m_pixelShaderBlob = nullptr;
	}



	D12Shader::D12Shader(const std::string& name) : m_name(name) {
		m_vertexShaderBlob = nullptr;
		m_pixelShaderBlob = nullptr;
	}



	void D12Shader::compileFromString(const std::string& vertexSrc, const std::string& pixelSrc) {

		compileStage(vertexSrc, "main", SHADER_MODEL_VS, m_vertexShaderBlob);
		compileStage(pixelSrc, "main", SHADER_MODEL_PS, m_pixelShaderBlob);

		createRootSignature();
		createPipelineState();

		AX_CORE_LOG_INFO("Compiled Vertex shader from string input");
		AX_CORE_LOG_INFO("Compiled Pixel shader from string input");
	}



	void D12Shader::compileFromFile(const std::string& vertexPath, const std::string& pixelPath) {
		compileStage(Shader::readShaderFile(vertexPath), "main", SHADER_MODEL_VS, m_vertexShaderBlob);
		compileStage(Shader::readShaderFile(pixelPath), "main", SHADER_MODEL_PS, m_pixelShaderBlob);

		createRootSignature();
		createPipelineState();

		AX_CORE_LOG_INFO("Compiled Vertex shader from file: {0}", vertexPath);
		AX_CORE_LOG_INFO("Compiled Pixel shader from file: {0}", pixelPath);
	}



	void D12Shader::compileStage(const std::string& source, const std::string& entryPoint, const std::string& target, Microsoft::WRL::ComPtr<ID3DBlob>& outblob) {

		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

		HRESULT hr = D3DCompile(
			source.c_str(),
			source.length(),
			nullptr, nullptr, nullptr,
			entryPoint.c_str(),
			target.c_str(),
			D3DCOMPILE_ENABLE_STRICTNESS,
			0,
			&outblob,
			&errorBlob
		);

		if (errorBlob) {
			std::string errorMsg = (char*)errorBlob->GetBufferPointer();
			AX_CORE_LOG_ERROR("Shader Compilation Error: {0}", errorMsg);
		}
		AX_THROW_IF_FAILED_HR(hr, "Failed to create Shader");

	}



	void D12Shader::bind() const {
		auto* cmdList = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext())->getCommandList();
		cmdList->SetGraphicsRootSignature(m_rootSignature.Get());
		cmdList->SetPipelineState(m_pipelineState.Get());
	}



	void D12Shader::unbind() const {
		// not required
	}



	void D12Shader::createRootSignature() {
		auto* device = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext())->getDevice();

		D3D12_ROOT_PARAMETER rootParameters[2] = {};
		// slot index 0
		rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParameters[0].Descriptor.ShaderRegister = 0;
		rootParameters[0].Descriptor.RegisterSpace = 0;
		rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		// slot index 1
		rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParameters[1].Descriptor.ShaderRegister = 1;
		rootParameters[1].Descriptor.RegisterSpace = 0;
		rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

		// rootsignature description
		D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
		rootSignatureDesc.NumParameters = _countof(rootParameters);
		rootSignatureDesc.pParameters = rootParameters;
		rootSignatureDesc.NumStaticSamplers = 0;
		rootSignatureDesc.pStaticSamplers = nullptr;
		rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		Microsoft::WRL::ComPtr<ID3DBlob> signature;
		Microsoft::WRL::ComPtr<ID3DBlob> error;
		HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
		if (error) { AX_THROW_IF_FAILED_HR(hr, "Failed to serialize root signature: {0}", (char*)error->GetBufferPointer()); }
		else { AX_THROW_IF_FAILED_HR(hr, "Failed to serialize root signature: unkown error", (char*)error->GetBufferPointer()); }
		
		hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
		AX_THROW_IF_FAILED_HR(hr, "Failed to create root signature");
		
		AX_CORE_LOG_INFO("Successfully created root signature");
	}



	void D12Shader::createPipelineState() {
		auto* device = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext())->getDevice();

		// Input layout
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
				offsetof(Vertex, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
				offsetof(Vertex, color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		psoDesc.pRootSignature = m_rootSignature.Get();
		psoDesc.VS = { m_vertexShaderBlob->GetBufferPointer(), m_vertexShaderBlob->GetBufferSize() };
		psoDesc.PS = { m_pixelShaderBlob->GetBufferPointer(), m_pixelShaderBlob->GetBufferSize() };

		// Rasterizer state
		D3D12_RASTERIZER_DESC rasterDesc = {};
		rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
		rasterDesc.CullMode = D3D12_CULL_MODE_BACK; //  D3D12_CULL_MODE_NONE
		rasterDesc.FrontCounterClockwise = FALSE;
		rasterDesc.DepthClipEnable = TRUE;
		rasterDesc.MultisampleEnable = FALSE;
		rasterDesc.AntialiasedLineEnable = FALSE;

		psoDesc.RasterizerState = rasterDesc;

		// Blend state
		D3D12_BLEND_DESC blendDesc = {};
		blendDesc.AlphaToCoverageEnable = FALSE;
		blendDesc.IndependentBlendEnable = FALSE;
		const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc = {
			FALSE,FALSE,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_LOGIC_OP_NOOP,
			D3D12_COLOR_WRITE_ENABLE_ALL,
		};

		for (int i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
			blendDesc.RenderTarget[i] = defaultRenderTargetBlendDesc;

		psoDesc.BlendState = blendDesc;

		// Depth stencil state
		D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
		depthStencilDesc.DepthEnable = FALSE;
		depthStencilDesc.StencilEnable = FALSE;

		psoDesc.DepthStencilState = depthStencilDesc;

		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;

		HRESULT hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));

		AX_THROW_IF_FAILED_HR(hr, "Failed to create graphics pipeline state");
		AX_CORE_LOG_INFO("Successfully created graphics pipeline state");

	}

}
