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
		m_rootSignature.Reset();
		m_pixelShaderBlob.Reset();
		m_vertexShaderBlob.Reset();
	}



	void D12Shader::compileFromFile(const std::string& filePath) {
		std::string source = Shader::readShaderFile(filePath);

		compileStage(source, "VSMain", SHADER_MODEL_VS, m_vertexShaderBlob);
		compileStage(source, "PSMain", SHADER_MODEL_PS, m_pixelShaderBlob);

		createRootSignature();

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



	void D12Shader::bind() const {}



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

		// TODO: rework this and automate it
		CD3DX12_ROOT_PARAMETER1 rootParameters[4];
		rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);		// b0 - slot 0, vertex shader CBV
		rootParameters[1].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);	// b1 - slot 1, vertex shader CBV
		rootParameters[2].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);								// t0 - texture descriptor table for pixel shader
		rootParameters[3].InitAsConstantBufferView(2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);	// b2 - slot 2, pixel shader CBV
		//rootParameters[4].InitAsConstantBufferView(3, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);	// b3 - slot 3, pixel shader CBV

		D3D12_STATIC_SAMPLER_DESC sampler = {};
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;	// D3D12_FILTER_MIN_MAG_MIP_POINT
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // D3D12_TEXTURE_ADDRESS_MODE_BORDER
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // D3D12_TEXTURE_ADDRESS_MODE_BORDER
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // D3D12_TEXTURE_ADDRESS_MODE_BORDER
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

}
