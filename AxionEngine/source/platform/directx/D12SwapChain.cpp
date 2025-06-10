#include "axpch.h"
#include "D12SwapChain.h"

namespace Axion {

	D12SwapChain::~D12SwapChain() {
		release();
	}

	void D12SwapChain::initialize(HWND hwnd, IDXGIFactory6* factory, ID3D12CommandQueue* cmdQueue, UINT width, UINT height) {

		AX_ASSERT(hwnd, "HWMD is null");
		AX_ASSERT(factory, "IDXGIFactory6 is null");
		AX_ASSERT(cmdQueue, "Command queue is null");
		m_hwnd = hwnd;

		DXGI_SWAP_CHAIN_DESC1 swapDesc = {};
		swapDesc.BufferCount = m_frameCount;
		swapDesc.Width = width;
		swapDesc.Height = height;
		swapDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapDesc.SampleDesc.Count = 1;

		Microsoft::WRL::ComPtr<IDXGISwapChain1> tempSwapChain;
		AX_THROW_IF_FAILED_HR(factory->CreateSwapChainForHwnd(cmdQueue, hwnd, &swapDesc, nullptr, nullptr, &tempSwapChain), "Failed to create temp swap chain");
		AX_THROW_IF_FAILED_HR(tempSwapChain.As(&m_swapChain), "Failed to transfer swap chain");
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
		AX_CORE_LOG_TRACE("Successfully created swap chain");

	}

	void D12SwapChain::release() {
		m_swapChain.Reset();
	}

	void D12SwapChain::resize(UINT width, UINT height, ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE rtvHeapStart, UINT rtvDescriptorSize,
		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>& renderTargets) {

		if (width == 0 || height == 0) return;

		for (auto& rt : renderTargets) {
			if (rt) rt.Reset();
		}

		// Get current swap chain description to preserve format/flags
		DXGI_SWAP_CHAIN_DESC1 desc = {};
		HRESULT hr = m_swapChain->GetDesc1(&desc);
		if (FAILED(hr)) {
			AX_CORE_LOG_ERROR("Failed to get swap chain description");
			return;
		}

		hr = m_swapChain->ResizeBuffers(
			m_frameCount,
			width,
			height,
			desc.Format,
			desc.Flags
		);
		if (FAILED(hr)) {
			AX_CORE_LOG_ERROR("Failed to resize swap chain buffers");
			return;
		}

		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

		// Recreate render target views
		for (UINT i = 0; i < m_frameCount; ++i) {
			Microsoft::WRL::ComPtr<ID3D12Resource> backBuffer;
			hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer));
			if (FAILED(hr)) {
				AX_CORE_LOG_ERROR("Failed to get buffer from swap chain");
				return;
			}

			CD3DX12_CPU_DESCRIPTOR_HANDLE handle(rtvHeapStart, i, rtvDescriptorSize);

			device->CreateRenderTargetView(backBuffer.Get(), nullptr, handle);
			renderTargets[i] = backBuffer;
		}

	}

	void D12SwapChain::present(UINT syncInterval, UINT flags) {
		AX_THROW_IF_FAILED_HR(m_swapChain->Present(syncInterval, flags), "Failed to present swap chain");
	}
	
	void D12SwapChain::advanceFrame() {
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
	}

}
