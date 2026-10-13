#include "axpch.h"
#include "DX12Shader.h"

#include "AxionEngine/Source/graphics/Vertex.h"
#include "AxionEngine/Source/graphics/GraphicsContext.h"

#include "AxionEngine/Platform/directx12/DX12Context.h"
#include "AxionEngine/Platform/directx12/DX12Helpers.h"

#include <d3d12shader.h>

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
		m_pixelShaderBlob.Reset();
		m_vertexShaderBlob.Reset();
	}

	void DX12Shader::compileFromFile(const std::filesystem::path& filePath) {
		std::string source = Shader::readShaderFile(filePath);

		compileStage(source, "VSMain", SHADER_MODEL_VS, m_vertexShaderBlob);
		compileStage(source, "PSMain", SHADER_MODEL_PS, m_pixelShaderBlob);

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
		Microsoft::WRL::ComPtr<IDxcUtils> utils;
		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));

		Microsoft::WRL::ComPtr<IDxcBlobEncoding> vsEncoding;
		utils->CreateBlob(vsData, (uint32_t)vsSize, DXC_CP_ACP, &vsEncoding);
		vsEncoding.As(&m_vertexShaderBlob);

		if (psData != nullptr && psSize > 0) {
			Microsoft::WRL::ComPtr<IDxcBlobEncoding> psEncoding;
			utils->CreateBlob(psData, (uint32_t)psSize, DXC_CP_ACP, &psEncoding);
			psEncoding.As(&m_pixelShaderBlob);
		}

		AX_CORE_LOG_TRACE("Shader '{}' loaded from bytecode", m_specification.name);
	}

	void DX12Shader::compileStage(const std::string& source, const std::string& entryPoint, const std::string& target, Microsoft::WRL::ComPtr<IDxcBlob>& outblob) {
		Microsoft::WRL::ComPtr<IDxcUtils> utils;
		Microsoft::WRL::ComPtr<IDxcCompiler3> compiler;
		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
		DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));

		Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler;
		utils->CreateDefaultIncludeHandler(&includeHandler);

		Microsoft::WRL::ComPtr<IDxcBlobEncoding> sourceBlob;
		utils->CreateBlob(source.c_str(), (uint32_t)source.length(), CP_UTF8, &sourceBlob);

		std::wstring wEntryPoint(entryPoint.begin(), entryPoint.end());
		std::wstring wTarget(target.begin(), target.end());

		std::vector<LPCWSTR> arguments = {
			L"-E", wEntryPoint.c_str(),
			L"-T", wTarget.c_str(),
			L"-HV", L"2021",			// HLSL 2021 standard
			L"-Zpr",					// Row major matrices (matches standard DX12 workflow)
			#if defined(AX_DEBUG)
			L"-Zi",						// Debug info
			L"-Qembed_debug",			// Embed PDB in shader container
			L"-Od"						// Disable optimizations
			#else
			L"-O3"						// Max optimization
			#endif
		};

		DxcBuffer sourceBuffer;
		sourceBuffer.Ptr = sourceBlob->GetBufferPointer();
		sourceBuffer.Size = sourceBlob->GetBufferSize();
		sourceBuffer.Encoding = DXC_CP_UTF8;

		Microsoft::WRL::ComPtr<IDxcResult> result;
		HRESULT hr = compiler->Compile(&sourceBuffer, arguments.data(), (uint32_t)arguments.size(), includeHandler.Get(), IID_PPV_ARGS(&result));

		Microsoft::WRL::ComPtr<IDxcBlobUtf8> errors;
		result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
		if (errors != nullptr && errors->GetStringLength() > 0) {
			AX_CORE_LOG_ERROR("Shader Compilation Error: {0}", errors->GetStringPointer());
		}

		result->GetStatus(&hr);
		if (FAILED(hr)) {
			std::string errorMsg = "Shader compilation failed: " + m_specification.name;
			AX_THROW_IF_FAILED_HR(hr, errorMsg);
		}

		result->GetResult(&outblob);
	}

	void DX12Shader::bind(RenderContext* renderContext) const {}
	void DX12Shader::unbind(RenderContext* renderContext) const {}

	ShaderBytecode DX12Shader::compileToBytecode(const std::filesystem::path& filepath) {
		ShaderBytecode result;

		Microsoft::WRL::ComPtr<IDxcUtils> utils;
		Microsoft::WRL::ComPtr<IDxcCompiler3> compiler;
		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
		DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));

		Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler;
		utils->CreateDefaultIncludeHandler(&includeHandler);

		std::wstring wFilePath = filepath.wstring();
		Microsoft::WRL::ComPtr<IDxcBlobEncoding> sourceBlob;
		if (FAILED(utils->LoadFile(wFilePath.c_str(), nullptr, &sourceBlob))) {
			AX_CORE_LOG_ERROR("Failed to load shader file: {}", filepath.string());
			return result;
		}

		auto compileInternal = [&](const wchar_t* entryPoint, const wchar_t* target) -> Microsoft::WRL::ComPtr<IDxcBlob> {
			std::vector<LPCWSTR> args = {
				wFilePath.c_str(),
				L"-E", entryPoint,
				L"-T", target,
				L"-HV", L"2021",
				L"-O3"
			};

			DxcBuffer sourceBuffer;
			sourceBuffer.Ptr = sourceBlob->GetBufferPointer();
			sourceBuffer.Size = sourceBlob->GetBufferSize();
			sourceBuffer.Encoding = DXC_CP_ACP;

			Microsoft::WRL::ComPtr<IDxcResult> compileResult;
			compiler->Compile(&sourceBuffer, args.data(), (uint32_t)args.size(), includeHandler.Get(), IID_PPV_ARGS(&compileResult));

			Microsoft::WRL::ComPtr<IDxcBlobUtf8> errors;
			compileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
			if (errors != nullptr && errors->GetStringLength() > 0) {
				AX_CORE_LOG_ERROR("Compile Error in {}: {}", filepath.string(), errors->GetStringPointer());
			}

			HRESULT hr;
			compileResult->GetStatus(&hr);
			if (FAILED(hr)) return nullptr;

			Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob;
			compileResult->GetResult(&shaderBlob);
			return shaderBlob;
		};

		auto vsBlob = compileInternal(L"VSMain", L"vs_6_5");
		auto psBlob = compileInternal(L"PSMain", L"ps_6_5");

		if (vsBlob) {
			result.vertex.assign((uint8_t*)vsBlob->GetBufferPointer(), (uint8_t*)vsBlob->GetBufferPointer() + vsBlob->GetBufferSize());
		}
		if (psBlob) {
			result.pixel.assign((uint8_t*)psBlob->GetBufferPointer(), (uint8_t*)psBlob->GetBufferPointer() + psBlob->GetBufferSize());
		}

		return result;
	}

}
