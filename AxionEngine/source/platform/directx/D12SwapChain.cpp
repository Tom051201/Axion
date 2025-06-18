#include "axpch.h"
#include "D12SwapChain.h"

#include "platform/directx/D12Context.h"

namespace Axion {

	D12SwapChain::~D12SwapChain() {
		release();
	}

	void D12SwapChain::initialize(HWND hwnd, IDXGIFactory6* factory, ID3D12CommandQueue* cmdQueue, UINT width, UINT height) {

		m_context = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext());
		auto* device = m_context->getDevice();
		auto& rtvHeap = m_context->getRtvHeapWrapper();

		AX_ASSERT(hwnd, "HWMD is null");
		AX_ASSERT(factory, "IDXGIFactory6 is null");
		AX_ASSERT(cmdQueue, "Command queue is null");

		// create swap chain
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


		// create RTVs
		for (UINT i = 0; i < m_frameCount; ++i) {
			AX_THROW_IF_FAILED_HR(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_backBuffers[i])), "Failed to get back buffer");

			m_rtvHeapIndices[i] = rtvHeap.allocate();
			D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeap.getCpuHandle(m_rtvHeapIndices[i]);

			device->CreateRenderTargetView(m_backBuffers[i].Get(), nullptr, handle);

			#ifdef AX_DEBUG
			wchar_t name[64];
			swprintf_s(name, L"SwapChainBuffer[%u]", i);
			m_backBuffers[i]->SetName(name);
			#endif
		}
		AX_CORE_LOG_TRACE("Successfully created {0} RTVs for the swap chain", m_frameCount);

	}

	void D12SwapChain::release() {
		for (auto& buffer : m_backBuffers) { buffer.Reset(); }
		m_swapChain.Reset();
	}

	void D12SwapChain::resize(UINT width, UINT height) {
		if (width == 0 || height == 0) return;

		auto* device = m_context->getDevice();
		auto& rtvHeap = m_context->getRtvHeapWrapper();

		for (auto& rt : m_backBuffers) {
			rt.Reset();
		}

		// Get current swap chain description to preserve format/flags
		DXGI_SWAP_CHAIN_DESC1 desc = {};
		HRESULT hr = m_swapChain->GetDesc1(&desc);
		if (FAILED(hr)) {
			AX_CORE_LOG_ERROR("Failed to get swap chain description");
			return;
		}

		hr = m_swapChain->ResizeBuffers(m_frameCount, width, height, desc.Format, desc.Flags);
		AX_THROW_IF_FAILED_HR(hr, "Failed to resize swap chain buffers");
		
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

		// Recreate render target views
		for (UINT i = 0; i < m_frameCount; ++i) {
			Microsoft::WRL::ComPtr<ID3D12Resource> backBuffer;
			hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer));
			AX_THROW_IF_FAILED_HR(hr, "Failed to get buffer from swap chain");
			
			//CD3DX12_CPU_DESCRIPTOR_HANDLE handle(rtvHeap.getHeap()->GetCPUDescriptorHandleForHeapStart(), i, rtvHeap.getDescriptorSize());
			D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeap.getCpuHandle(m_rtvHeapIndices[i]);

			device->CreateRenderTargetView(backBuffer.Get(), nullptr, handle);
			m_backBuffers[i] = backBuffer;
		}

	}

	void D12SwapChain::present(UINT syncInterval, UINT flags) {
		AX_THROW_IF_FAILED_HR(m_swapChain->Present(syncInterval, flags), "Failed to present swap chain");
	}
	
	void D12SwapChain::advanceFrame() {
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D12SwapChain::getBackBufferRtv(uint32_t index) const {
		return m_context->getRtvHeapWrapper().getCpuHandle(m_rtvHeapIndices[index]);
	}

}
