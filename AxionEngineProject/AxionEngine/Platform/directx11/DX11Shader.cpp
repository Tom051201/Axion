#include "axpch.h"
#include "DX11Shader.h"

#include <d3dcompiler.h>

#include "AXionEngine/Platform/directx11/DX11Context.h"
#include "AxionEngine/Platform/directx11/DX11DebugLayer.h"

namespace Axion {

	constexpr const char* SHADER_MODEL_VS = "vs_5_0";
	constexpr const char* SHADER_MODEL_PS = "ps_5_0";

	DX11Shader::DX11Shader()
		: m_vertexShaderBlob(nullptr), m_pixelShaderBlob(nullptr) {}

	DX11Shader::DX11Shader(const ShaderSpecification& spec)
		: m_vertexShaderBlob(nullptr), m_pixelShaderBlob(nullptr), m_specification(spec), m_shaderFileLocation("") {}

	DX11Shader::DX11Shader(const ShaderSpecification& spec, const std::filesystem::path& filePath)
		: m_vertexShaderBlob(nullptr), m_pixelShaderBlob(nullptr), m_specification(spec), m_shaderFileLocation(filePath) {}

	DX11Shader::~DX11Shader() {
		release();
	}

	void DX11Shader::release() {
		m_vertexShader.Reset();
		m_pixelShader.Reset();
		m_pixelShaderBlob.Reset();
		m_vertexShaderBlob.Reset();
	}

	void DX11Shader::bind() const {
		auto* ctx = static_cast<DX11Context*>(GraphicsContext::get()->getNativeContext())->getDeviceContext();
		ctx->VSSetShader(m_vertexShader.Get(), nullptr, 0);
		ctx->PSSetShader(m_pixelShader.Get(), nullptr, 0);
	}

	void DX11Shader::unbind() const {
		auto* ctx = static_cast<DX11Context*>(GraphicsContext::get()->getNativeContext())->getDeviceContext();
		ctx->VSSetShader(nullptr, nullptr, 0);
		ctx->PSSetShader(nullptr, nullptr, 0);
	}

	void DX11Shader::compileFromFile(const std::filesystem::path& filePath) {
		std::string source = Shader::readShaderFile(filePath);

		compileStage(source, "VSMain", SHADER_MODEL_VS, m_vertexShaderBlob);
		compileStage(source, "PSMain", SHADER_MODEL_PS, m_pixelShaderBlob);

		reflectAndCreateShaders();

		AX_CORE_LOG_TRACE("Shader '{}' compiled for DX11", m_specification.name);
	}

	void DX11Shader::recompile() {
		if (m_shaderFileLocation.empty()) {
			AX_CORE_LOG_WARN("Cannot recompile a shader that has no file path cached");
			return;
		}
		compileFromFile(m_shaderFileLocation);
	}

	void DX11Shader::loadFromBytecode(const uint8_t* vsData, size_t vsSize, const uint8_t* psData, size_t psSize) {
		HRESULT hr = D3DCreateBlob(vsSize, &m_vertexShaderBlob);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create Vertex Shader Blob for bytecode");
		memcpy(m_vertexShaderBlob->GetBufferPointer(), vsData, vsSize);

		if (psData != nullptr && psSize > 0) {
			hr = D3DCreateBlob(psSize, &m_pixelShaderBlob);
			AX_THROW_IF_FAILED_HR(hr, "Failed to create Pixel Shader Blob for bytecode");
			memcpy(m_pixelShaderBlob->GetBufferPointer(), psData, psSize);
		}

		reflectAndCreateShaders();

		AX_CORE_LOG_TRACE("Shader '{}' loaded from bytecode for DX11", m_specification.name);
	}

	int DX11Shader::getBindPoint(const std::string& name) const {
		auto it = m_resourceMap.find(name);
		if (it != m_resourceMap.end()) {
			return static_cast<int>(it->second);
		}
		AX_CORE_LOG_WARN("DX11 Shader resource '{}' not found in reflection data!", name);
		return -1;
	}

	void DX11Shader::compileStage(const std::string& source, const std::string& entryPoint, const std::string& target, Microsoft::WRL::ComPtr<ID3DBlob>& outblob) {
		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

		HRESULT hr = D3DCompile(
			source.c_str(), source.length(),
			nullptr, nullptr, nullptr,
			entryPoint.c_str(), target.c_str(),
			D3DCOMPILE_ENABLE_STRICTNESS, 0,
			&outblob, &errorBlob
		);

		if (FAILED(hr)) {
			if (errorBlob) {
				std::string errorMsg = (char*)errorBlob->GetBufferPointer();
				AX_CORE_LOG_ERROR("Shader Compilation Error: {0}", errorMsg);
			}
			std::string errorMsg = "Shader compilation failed: " + m_specification.name;
			AX_THROW_IF_FAILED_HR(hr, errorMsg);
		}
	}

	void DX11Shader::reflectAndCreateShaders() {
		auto* device = static_cast<DX11Context*>(GraphicsContext::get()->getNativeContext())->getDevice();

		// -- Create Hardware Shaders --
		HRESULT hr = device->CreateVertexShader(m_vertexShaderBlob->GetBufferPointer(), m_vertexShaderBlob->GetBufferSize(), nullptr, &m_vertexShader);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create DX11 Vertex Shader");

		if (m_pixelShaderBlob) {
			hr = device->CreatePixelShader(m_pixelShaderBlob->GetBufferPointer(), m_pixelShaderBlob->GetBufferSize(), nullptr, &m_pixelShader);
			AX_THROW_IF_FAILED_HR(hr, "Failed to create DX11 Pixel Shader");
		}

		// -- Reflect To Find Bind Slots --
		auto reflectStage = [&](Microsoft::WRL::ComPtr<ID3DBlob>& blob) {
			if (!blob) return;
			Microsoft::WRL::ComPtr<ID3D11ShaderReflection> reflector;

			HRESULT reflectHr = D3DReflect(blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&reflector));
			AX_THROW_IF_FAILED_HR(reflectHr, "Failed to reflect DX11 Shader");

			D3D11_SHADER_DESC desc;
			reflector->GetDesc(&desc);

			for (UINT i = 0; i < desc.BoundResources; i++) {
				D3D11_SHADER_INPUT_BIND_DESC bindDesc;
				reflector->GetResourceBindingDesc(i, &bindDesc);

				m_resourceMap[bindDesc.Name] = bindDesc.BindPoint;

				if (bindDesc.Type == D3D_SIT_TEXTURE && m_textureTableSlot == 0) {
					m_textureTableSlot = bindDesc.BindPoint;
				}
			}
			};

		reflectStage(m_vertexShaderBlob);
		reflectStage(m_pixelShaderBlob);

		// -- Debug Naming --
		#ifdef AX_DEBUG
		DX11DebugLayer::setName(m_vertexShader.Get(), m_specification.name + " VS");
		if (m_pixelShader) DX11DebugLayer::setName(m_pixelShader.Get(), m_specification.name + " PS");
		#endif
	}

}
