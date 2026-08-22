#include "axpch.h"
#include "DX11Context.h"

#include "AxionEngine/Platform/windows/WindowsHelper.h"
#include "AxionEngine/Platform/directx11/DX11Texture.h"

#include "AxionEngine/Source/core/EngineAssets.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

#ifdef AX_DEBUG
#include "AxionEngine/Platform/directx11/DX11DebugLayer.h"
#endif

namespace Axion {

	DX11Context::~DX11Context() {
		shutdown();
	}

	void DX11Context::initialize(void* hwnd, uint32_t width, uint32_t height) {
		m_width = width;
		m_height = height;

		// -- Setup Swap Chain --
		DXGI_SWAP_CHAIN_DESC scd = {};
		scd.BufferCount = 2;
		scd.BufferDesc.Width = width;
		scd.BufferDesc.Height = height;
		scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scd.BufferDesc.RefreshRate.Numerator = 60;
		scd.BufferDesc.RefreshRate.Denominator = 1;
		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scd.OutputWindow = (HWND)hwnd;
		scd.SampleDesc.Count = 1;
		scd.SampleDesc.Quality = 0;
		scd.Windowed = TRUE;
		scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

		// -- Setup Device --
		UINT createDeviceFlags = 0;

		#ifdef AX_DEBUG
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
		#endif

		D3D_FEATURE_LEVEL featureLevel;
		const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };

		HRESULT hr = D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			createDeviceFlags,
			featureLevelArray,
			2,
			D3D11_SDK_VERSION,
			&scd,
			&m_swapChain,
			&m_device,
			&featureLevel,
			&m_deviceContext
		);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create DirectX 11 Device and SwapChain");

		#ifdef AX_DEBUG
		DX11DebugLayer::initialize(m_device.Get());
		#endif

		createRenderTarget();

		// -- Create Global Sampler State --
		D3D11_SAMPLER_DESC sampDesc = {};
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		hr = m_device->CreateSamplerState(&sampDesc, &m_samplerState);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create DX11 Global Sampler State");

		#ifdef AX_DEBUG
		DX11DebugLayer::setName(m_samplerState.Get(), "DX11 Global SamplerState");
		#endif

		AX_CORE_LOG_INFO("DirectX 11 Backend initialized! GPU {}", getGpuName());
	}

	void DX11Context::shutdown() {
		if (!m_device) return;

		cleanupRenderTarget();
		m_samplerState.Reset();
		m_swapChain.Reset();
		m_deviceContext.Reset();

		#ifdef AX_DEBUG
		DX11DebugLayer::reportLiveObjects(m_device.Get());
		#endif

		m_device.Reset();
	}

	void DX11Context::prepareRendering() {
		// -- Set Viewport --
		D3D11_VIEWPORT vp = {};
		vp.Width = (float)m_width;
		vp.Height = (float)m_height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		m_deviceContext->RSSetViewports(1, &vp);

		m_deviceContext->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
	}

	void DX11Context::finishRendering() {
		m_swapChain->Present(m_vsyncInterval, 0);
	}

	void DX11Context::setClearColor(const Vec4& color) {
		m_clearColor = color;
	}

	void DX11Context::clear() {
		float color[4] = { m_clearColor.x, m_clearColor.y, m_clearColor.z, m_clearColor.w };
		m_deviceContext->ClearRenderTargetView(m_backBufferRTV.Get(), color);
	}

	void DX11Context::bindSwapChainRenderTarget() {
		m_deviceContext->OMSetRenderTargets(1, m_backBufferRTV.GetAddressOf(), nullptr);
	}

	void DX11Context::bindDepthOnlyRenderTarget(const Ref<Texture2D>& depthTexture) {
		auto* depthTex = static_cast<DX11DepthTexture*>(depthTexture.get());
		ID3D11DepthStencilView* dsv = depthTex->getDsv();

		ID3D11ShaderResourceView* nullSRVs[16] = { nullptr };
		m_deviceContext->PSSetShaderResources(0, 16, nullSRVs);

		m_deviceContext->OMSetRenderTargets(0, nullptr, dsv);
		m_deviceContext->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

		D3D11_VIEWPORT vp{};
		vp.Width = static_cast<float>(depthTex->getWidth());
		vp.Height = static_cast<float>(depthTex->getHeight());
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		m_deviceContext->RSSetViewports(1, &vp);
	}

	void DX11Context::unbindDepthOnlyRenderTarget(const Ref<Texture2D>& depthTexture) {
		m_deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	}

	void* DX11Context::getImGuiTextureID(const Ref<Texture2D>& texture) {
		// TODO remove this in the future because imgui was removed from the engine!
		return nullptr;
	}

	void DX11Context::bindSrvTable(uint32_t rootIndex, const std::array<Ref<Texture2D>, 16>& textures, uint32_t count) {
		if (count == 0) return;

		ID3D11ShaderResourceView* srvs[16] = { nullptr };

		for (uint32_t i = 0; i < count; i++) {
			if (textures[i]) {
				srvs[i] = static_cast<ID3D11ShaderResourceView*>(textures[i]->getHandle());
			}
			else if (textures[0]) {
				srvs[i] = static_cast<ID3D11ShaderResourceView*>(textures[0]->getHandle());
			}
			else {
				srvs[i] = static_cast<ID3D11ShaderResourceView*>(EngineAssets::getWhiteTexture()->getHandle());
			}
		}

		// ---> IGNORE rootIndex! DX11 maps directly to HLSL register t0 <---
		m_deviceContext->PSSetShaderResources(0, count, srvs);
		m_deviceContext->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
	}

	void DX11Context::resize(uint32_t width, uint32_t height) {
		if (width == 0 || height == 0) return;
		m_width = width;
		m_height = height;

		cleanupRenderTarget();

		HRESULT hr = m_swapChain->ResizeBuffers(2, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
		AX_THROW_IF_FAILED_HR(hr, "Failed to resize DX11 SwapChain");

		createRenderTarget();
	}

	void DX11Context::drawIndexed(const Ref<VertexBuffer>& vb, const Ref<IndexBuffer>& ib, uint32_t instanceCount) {
		m_deviceContext->DrawIndexedInstanced(ib->getIndexCount(), instanceCount, 0, 0, 0);
	}

	void DX11Context::drawIndexed(const Ref<IndexBuffer>& ib, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation) {
		m_deviceContext->DrawIndexedInstanced(indexCount, instanceCount, startIndexLocation, baseVertexLocation, 0);
	}

	void DX11Context::draw(uint32_t vertexCount) {
		m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		m_deviceContext->DrawInstanced(vertexCount, 1, 0, 0);
		m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	std::string DX11Context::getGpuName() const {
		Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
		m_device.As(&dxgiDevice);
		Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
		dxgiDevice->GetAdapter(&adapter);
		DXGI_ADAPTER_DESC desc;
		adapter->GetDesc(&desc);
		return WindowsHelper::WStringToString(std::wstring(desc.Description));
	}

	std::string DX11Context::getGpuDriverVersion() const {
		return "DX11 Auto";
	}

	uint64_t DX11Context::getVramMB() const {
		Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
		m_device.As(&dxgiDevice);
		Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
		dxgiDevice->GetAdapter(&adapter);
		DXGI_ADAPTER_DESC desc;
		adapter->GetDesc(&desc);
		return desc.DedicatedVideoMemory / (1024 * 1024);
	}

	void DX11Context::createRenderTarget() {
		Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;

		HRESULT hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
		AX_THROW_IF_FAILED_HR(hr, "Failed to get DX11 SwapChain BackBuffer");

		hr = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_backBufferRTV);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create DX11 SwapChain RTV");

		#ifdef AX_DEBUG
		DX11DebugLayer::setName(backBuffer.Get(), "DX11 SwapChain BackBuffer");
		DX11DebugLayer::setName(m_backBufferRTV.Get(), "DX11 SwapChain RTV");
		#endif
	}

	void DX11Context::cleanupRenderTarget() {
		m_backBufferRTV.Reset();
	}

}
