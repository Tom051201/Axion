#include "axpch.h"
#include "DX12Shader.h"

#include "AxionEngine/Source/render/Vertex.h"
#include "AxionEngine/Source/render/GraphicsContext.h"

#include "AxionEngine/Platform/directx/DX12Context.h"
#include "AxionEngine/Platform/directx/DX12Helpers.h"

#include <d3D12Shader.h>

namespace Axion {

	DX12Shader::DX12Shader() : m_vertexShaderBlob(nullptr), m_pixelShaderBlob(nullptr) {}

	DX12Shader::DX12Shader(const ShaderSpecification& spec)
		: m_vertexShaderBlob(nullptr), m_pixelShaderBlob(nullptr), m_specification(spec), m_shaderFileLocation("") {}

	DX12Shader::DX12Shader(const ShaderSpecification& spec, const std::filesystem::path& filePath)
		: m_vertexShaderBlob(nullptr), m_pixelShaderBlob(nullptr), m_specification(spec), m_shaderFileLocation(filePath) {}

	DX12Shader::~DX12Shader() {
		release();
	}

	void DX12Shader::release() {
		m_rootSignature.Reset();
		m_pixelShaderBlob.Reset();
		m_vertexShaderBlob.Reset();
	}



	void DX12Shader::compileFromFile(const std::filesystem::path& filePath) {
		std::string source = Shader::readShaderFile(filePath);

		compileStage(source, "VSMain", SHADER_MODEL_VS, m_vertexShaderBlob);
		compileStage(source, "PSMain", SHADER_MODEL_PS, m_pixelShaderBlob);

		createRootSignature();

		AX_CORE_LOG_TRACE("Shader '{}' compiled", m_specification.name);
	}



	void DX12Shader::recompile() {
		if (m_shaderFileLocation.empty()) {
			AX_CORE_LOG_WARN("Cannot recompile a shader that has no file path cached");
			return;
		}

		compileFromFile(m_shaderFileLocation);
	}



	void DX12Shader::loadFromBytecode(const uint8_t* vsData, size_t vsSize, const uint8_t* psData, size_t psSize) {

		HRESULT hr = D3DCreateBlob(vsSize, &m_vertexShaderBlob);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create Vertex Shader Blob for bytecode");
		memcpy(m_vertexShaderBlob->GetBufferPointer(), vsData, vsSize);

		if (psData != nullptr && psSize > 0) {
			hr = D3DCreateBlob(psSize, &m_pixelShaderBlob);
			AX_THROW_IF_FAILED_HR(hr, "Failed to create Pixel Shader Blob for bytecode");
			memcpy(m_pixelShaderBlob->GetBufferPointer(), psData, psSize);
		}

		createRootSignature();

		AX_CORE_LOG_TRACE("Shader '{}' loaded from bytecode", m_specification.name);
	}



	void DX12Shader::compileStage(const std::string& source, const std::string& entryPoint, const std::string& target, Microsoft::WRL::ComPtr<ID3DBlob>& outblob) {

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



	void DX12Shader::bind() const {}



	// not required
	void DX12Shader::unbind() const {}



	void DX12Shader::createRootSignature() {
		auto* device = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext())->getDevice();

		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
		if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)))) {
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		struct CbvInfo {
			std::string name;
			D3D12_SHADER_VISIBILITY visibility;
		};

		struct SrvInfo {
			std::string name;
			D3D12_SHADER_VISIBILITY visibility;
		};

		std::map<UINT, CbvInfo> cbvRegisters;
		std::map<UINT, SrvInfo> rootSrvRegisters;
		UINT maxSrvRegister = 0;
		bool hasSrvs = false;

		// -- Reflect --
		auto reflectStage = [&](Microsoft::WRL::ComPtr<ID3DBlob>& blob, D3D12_SHADER_VISIBILITY visibility) {
			if (!blob) return;
			Microsoft::WRL::ComPtr< ID3D12ShaderReflection> reflector;
			HRESULT hr = D3DReflect(blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&reflector));
			if (FAILED(hr)) return;

			D3D12_SHADER_DESC desc;
			reflector->GetDesc(&desc);

			for (UINT i = 0; i < desc.BoundResources; i++) {
				D3D12_SHADER_INPUT_BIND_DESC bindDesc;
				reflector->GetResourceBindingDesc(i, &bindDesc);

				if (bindDesc.Type == D3D_SIT_CBUFFER) {
					if (cbvRegisters.find(bindDesc.BindPoint) != cbvRegisters.end()) {
						cbvRegisters[bindDesc.BindPoint].visibility = D3D12_SHADER_VISIBILITY_ALL;
					}
					else {
						cbvRegisters[bindDesc.BindPoint] = { bindDesc.Name, visibility };
					}
				}
				else if (bindDesc.Type == D3D_SIT_TEXTURE) {
					m_resourceMap[bindDesc.Name] = bindDesc.BindPoint;
					maxSrvRegister = std::max(maxSrvRegister, bindDesc.BindPoint + bindDesc.BindCount - 1);
					hasSrvs = true;
				}
				else if (bindDesc.Type == D3D_SIT_STRUCTURED) {
					if (rootSrvRegisters.find(bindDesc.BindPoint) != rootSrvRegisters.end()) {
						rootSrvRegisters[bindDesc.BindPoint].visibility = D3D12_SHADER_VISIBILITY_ALL;
					}
					else {
						rootSrvRegisters[bindDesc.BindPoint] = { bindDesc.Name, visibility };
					}
				}
			}
		};

		reflectStage(m_vertexShaderBlob, D3D12_SHADER_VISIBILITY_VERTEX);
		reflectStage(m_pixelShaderBlob, D3D12_SHADER_VISIBILITY_PIXEL);

		// -- Root params --
		std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters;

		for (auto const& [reg, info] : cbvRegisters) {
			CD3DX12_ROOT_PARAMETER1 param;
			param.InitAsConstantBufferView(reg, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, info.visibility);

			uint32_t rootParamIndex = static_cast<uint32_t>(rootParameters.size());
			m_resourceMap[info.name] = rootParamIndex;
			rootParameters.push_back(param);
		}

		for (auto const& [reg, info] : rootSrvRegisters) {
			CD3DX12_ROOT_PARAMETER1 param;
			param.InitAsShaderResourceView(reg, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, info.visibility);

			uint32_t rootParamIndex = static_cast<uint32_t>(rootParameters.size());
			m_resourceMap[info.name] = rootParamIndex;
			rootParameters.push_back(param);
		}

		UINT srvCount = std::max((UINT)m_specification.batchTextures, hasSrvs ? (maxSrvRegister + 1) : 0);
		if (srvCount > 0) {
			CD3DX12_DESCRIPTOR_RANGE1 srvRange;
			srvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, srvCount, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);

			CD3DX12_ROOT_PARAMETER1 tableParam;
			tableParam.InitAsDescriptorTable(1, &srvRange, D3D12_SHADER_VISIBILITY_PIXEL);

			m_textureTableSlot = static_cast<uint32_t>(rootParameters.size());
			rootParameters.push_back(tableParam);
		}

		// -- Static sampler --
		D3D12_STATIC_SAMPLER_DESC sampler = {};
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		sampler.MipLODBias = 0;
		sampler.MaxAnisotropy = 0;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		sampler.MinLOD = 0.0f;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 0;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// -- Serialize --
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(
			static_cast<UINT>(rootParameters.size()),
			rootParameters.data(),
			1,
			&sampler,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		);

		Microsoft::WRL::ComPtr<ID3DBlob> signature;
		Microsoft::WRL::ComPtr<ID3DBlob> error;

		HRESULT hr = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error);
		if (error) { AX_CORE_LOG_ERROR("Root signature error: {0}", (char*)error->GetBufferPointer()); }
		AX_THROW_IF_FAILED_HR(hr, "Failed to serialize root signature");

		hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
		AX_THROW_IF_FAILED_HR(hr, "Failed to create root signature");

		#ifdef AX_DEBUG
		m_rootSignature->SetName(L"RootSignature");
		#endif
	}

	int DX12Shader::getBindPoint(const std::string& name) const {
		auto it = m_resourceMap.find(name);
		if (it != m_resourceMap.end()) {
			return static_cast<int>(it->second);
		}
		AX_CORE_LOG_WARN("Shader resource '{}' not found in reflection data!", name);
		return -1;
	}

	ShaderBytecode DX12Shader::compileToBytecode(const std::filesystem::path& filepath) {
		ShaderBytecode result;
		std::wstring wFilePath = filepath.wstring();

		ID3DBlob* vsBlob = nullptr;
		ID3DBlob* psBlob = nullptr;
		ID3DBlob* errorBlob = nullptr;
		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3;

		// -- Compile Vertex Shader --
		HRESULT hr = D3DCompileFromFile(wFilePath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", flags, 0, &vsBlob, &errorBlob);
		if (FAILED(hr)) {
			if (errorBlob) AX_CORE_LOG_ERROR("VS Compile Error: {}", (char*)errorBlob->GetBufferPointer());
			if (errorBlob) errorBlob->Release();
			return result;
		}

		// -- Compile Pixel Shader --
		hr = D3DCompileFromFile(wFilePath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", flags, 0, &psBlob, &errorBlob);
		if (FAILED(hr)) {
			if (errorBlob) AX_CORE_LOG_ERROR("PS Compile Error: {}", (char*)errorBlob->GetBufferPointer());
			if (errorBlob) errorBlob->Release();
			vsBlob->Release();
			return result;
		}

		// --  Copy data into abstract vectors --
		result.vertex.assign((uint8_t*)vsBlob->GetBufferPointer(), (uint8_t*)vsBlob->GetBufferPointer() + vsBlob->GetBufferSize());
		result.pixel.assign((uint8_t*)psBlob->GetBufferPointer(), (uint8_t*)psBlob->GetBufferPointer() + psBlob->GetBufferSize());

		vsBlob->Release();
		psBlob->Release();

		return result;
	}

}
