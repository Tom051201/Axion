#include "axpch.h"
#include "D12Shader.h"

#include "AxionEngine/Source/render/Vertex.h"
#include "AxionEngine/Source/render/GraphicsContext.h"

#include "AxionEngine/Platform/directx/D12Context.h"
#include "AxionEngine/Platform/directx/D12Helpers.h"

namespace Axion {

	D12Shader::D12Shader() : m_vertexShaderBlob(nullptr), m_pixelShaderBlob(nullptr) {}

	D12Shader::D12Shader(const ShaderSpecification& spec)
		: m_vertexShaderBlob(nullptr), m_pixelShaderBlob(nullptr), m_specification(spec), m_shaderFileLocation("") {}

	D12Shader::D12Shader(const ShaderSpecification& spec, const std::string& filePath)
		: m_vertexShaderBlob(nullptr), m_pixelShaderBlob(nullptr), m_specification(spec), m_shaderFileLocation(filePath) {}

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

		AX_CORE_LOG_TRACE("Shader '{}' compiled", m_specification.name);
	}



	void D12Shader::recompile() {
		if (m_shaderFileLocation.empty()) {
			AX_CORE_LOG_WARN("Cannot recompile a shader that has no file path cached");
			return;
		}

		compileFromFile(m_shaderFileLocation);
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
			std::string errorMsg = "Shader compilation failed: " + m_specification.name;
			AX_THROW_IF_FAILED_HR(hr, errorMsg);
		}

	}



	void D12Shader::bind() const {
		auto* cmdList = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext())->getCommandList();
		cmdList->SetGraphicsRootSignature(m_rootSignature.Get());
		cmdList->SetPipelineState(m_pipelineState.Get());
		cmdList->IASetPrimitiveTopology(D12Helpers::toD12Topology(m_specification.topology));
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
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, m_specification.batchTextures, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE); // D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC

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

		HRESULT hr = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error);	
		if (error) {
			AX_CORE_LOG_ERROR("Root signature error: {0}", (char*)error->GetBufferPointer());
		}
		AX_THROW_IF_FAILED_HR(hr, "Failed to serialize root signature");
		

		hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
		AX_THROW_IF_FAILED_HR(hr, "Failed to create root signature");

		#ifdef AX_DEBUG
		m_rootSignature->SetName(L"RootSignature");
		#endif
	}



	void D12Shader::createPipelineState() {
		auto* device = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext())->getDevice();

		std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements;
		const auto& layout = m_specification.vertexLayout.getElements();
		std::unordered_map<std::string, uint32_t> semanticCounts;

		AX_CORE_ASSERT(layout.size() > 0, "No buffer layout specified");

		for (const auto& element : layout) {
			D3D12_INPUT_ELEMENT_DESC desc = {};
			desc.SemanticName = element.name.c_str();
			desc.SemanticIndex = semanticCounts[element.name]++;
			desc.Format = D12Helpers::ShaderDataTypeToDXGIFormat(element.type);
			desc.InputSlot = 0;
			desc.AlignedByteOffset = element.offset;
			desc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			desc.InstanceDataStepRate = 0;

			inputElements.push_back(desc);
		}

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputElements.data(), (UINT)inputElements.size() };
		psoDesc.pRootSignature = m_rootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_vertexShaderBlob.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_pixelShaderBlob.Get());

		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.RasterizerState.CullMode = D12Helpers::toD12CullMode(m_specification.cullMode);

		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthEnable = m_specification.depthTest ? TRUE : FALSE;
		psoDesc.DepthStencilState.DepthWriteMask = m_specification.depthWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
		psoDesc.DepthStencilState.DepthFunc = D12Helpers::toD12DepthComparisonFunction(m_specification.depthFunction);
		psoDesc.DSVFormat = D12Helpers::toD12DepthStencilFormat(m_specification.depthStencilFormat);
		psoDesc.DepthStencilState.StencilEnable = m_specification.stencilEnabled ? TRUE : FALSE;

		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D12Helpers::toD12ToplogyType(m_specification.topology);
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = D12Helpers::toD12ColorFormat(m_specification.colorFormat);
		psoDesc.SampleDesc.Count = m_specification.sampleCount;

		HRESULT hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
		AX_THROW_IF_FAILED_HR(hr, "Failed to create graphics pipeline state");

		#ifdef AX_DEBUG
		m_pipelineState->SetName(L"PipelineState");
		#endif
	}

}
