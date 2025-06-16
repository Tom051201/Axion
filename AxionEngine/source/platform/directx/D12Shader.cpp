#include "axpch.h"
#include "D12Shader.h"

#include "Axion/render/DataTypes.h"
#include "Axion/render/GraphicsContext.h"

#include "platform/directx/D12Context.h"

namespace Axion {

	D12Shader::D12Shader() : m_vertexShaderBlob(nullptr), m_pixelShaderBlob(nullptr), m_name("UNSET") {}

	D12Shader::D12Shader(const std::string& name) : m_vertexShaderBlob(nullptr), m_pixelShaderBlob(nullptr), m_name(name) {}

	D12Shader::~D12Shader() {
		release();
	}

	void D12Shader::release() {
		m_pipelineState.Reset();
		m_rootSignature.Reset();
		m_pixelShaderBlob.Reset();
		m_vertexShaderBlob.Reset();
	}



	void D12Shader::compileFromFile(const std::string& filePath) {
		std::string source = Shader::readShaderFile(filePath);

		compileStage(source, "VSMain", SHADER_MODEL_VS, m_vertexShaderBlob);
		compileStage(source, "PSMain", SHADER_MODEL_PS, m_pixelShaderBlob);

		createRootSignature();
		createPipelineState();

		AX_CORE_LOG_TRACE("Shader '{0}' setup successful (root signature / pipeline state)", m_name);
		AX_CORE_LOG_TRACE("Shader '{0}' compiled ({1})", m_name, filePath);
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

		if (FAILED(hr)) {
			if (errorBlob) {
				std::string errorMsg = (char*)errorBlob->GetBufferPointer();
				AX_CORE_LOG_ERROR("Shader Compilation Error: {0}", errorMsg);
			}
			else {
				AX_CORE_LOG_ERROR("Shader Compilation failed with no error message.");
			}
			AX_THROW_IF_FAILED_HR(hr, "Shader compilation failed");
		}

	}



	void D12Shader::bind() const {
		auto* cmdList = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext())->getCommandList();
		cmdList->SetGraphicsRootSignature(m_rootSignature.Get());
		cmdList->SetPipelineState(m_pipelineState.Get());
	}



	// not required
	void D12Shader::unbind() const {}



	void D12Shader::createRootSignature() {
		auto* device = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext())->getDevice();

		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
		if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)))) {
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		CD3DX12_DESCRIPTOR_RANGE1 ranges[1]; // t0
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

		CD3DX12_ROOT_PARAMETER1 rootParameters[3];
		rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);	// b0 - slot 0, vertex shader CBV
		rootParameters[1].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);	// b1 - slot 1, vertex shader CBV
		rootParameters[2].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);								// t0 - texture descriptor table for pixel shader

		D3D12_STATIC_SAMPLER_DESC sampler = {};
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.MipLODBias = 0;
		sampler.MaxAnisotropy = 0;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		sampler.MinLOD = 0.0f;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 0;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		Microsoft::WRL::ComPtr<ID3DBlob> signature;
		Microsoft::WRL::ComPtr<ID3DBlob> error;

		//HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
		HRESULT hr = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error);	
		if (error) {
			AX_CORE_LOG_ERROR("Root signature error: {0}", (char*)error->GetBufferPointer());
		}
		AX_THROW_IF_FAILED_HR(hr, "Failed to serialize root signature");
		
		
		//hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
		hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
		AX_THROW_IF_FAILED_HR(hr, "Failed to create root signature");

		#ifdef AX_DEBUG
		m_rootSignature->SetName(L"RootSignature");
		#endif
	}



	void D12Shader::createPipelineState() {
		auto* device = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext())->getDevice();

		// Input layout
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, uv), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
		};

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		psoDesc.pRootSignature = m_rootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_vertexShaderBlob.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_pixelShaderBlob.Get());

		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK; // D3D12_CULL_MODE_NONE

		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthEnable = FALSE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;

		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;

		HRESULT hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
		AX_THROW_IF_FAILED_HR(hr, "Failed to create graphics pipeline state");

		#ifdef AX_DEBUG
		m_pipelineState->SetName(L"PipelineState");
		#endif
	}

}
