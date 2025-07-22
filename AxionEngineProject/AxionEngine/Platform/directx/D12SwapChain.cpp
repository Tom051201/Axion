#include "axpch.h"
#include "D12SwapChain.h"

#include "AxionEngine/Platform/directx/D12Context.h"
#include "AxionEngine/Platform/directx/D12Helpers.h"

namespace Axion {

	D12SwapChain::~D12SwapChain() {
		release();
	}

	void D12SwapChain::initialize(HWND hwnd, IDXGIFactory6* factory, ID3D12CommandQueue* cmdQueue, const SwapChainSpecification& spec) {

		AX_CORE_ASSERT(hwnd, "HWMD is null");
		AX_CORE_ASSERT(factory, "IDXGIFactory6 is null");
		AX_CORE_ASSERT(cmdQueue, "Command queue is null");

		m_specification = spec;
		m_context = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext());
		auto* device = m_context->getDevice();
		auto& rtvHeap = m_context->getRtvHeapWrapper();
		auto& dsvHeap = m_context->getDsvHeapWrapper();

		// create swap chain
		DXGI_SWAP_CHAIN_DESC1 swapDesc = {};
		swapDesc.BufferCount = spec.bufferCount;
		swapDesc.Width = spec.width;
		swapDesc.Height = spec.height;
		swapDesc.Format = D12Helpers::toD12ColorFormat(spec.backBufferFormat);
		swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapDesc.SampleDesc.Count = 1;

		Microsoft::WRL::ComPtr<IDXGISwapChain1> tempSwapChain;
		AX_THROW_IF_FAILED_HR(factory->CreateSwapChainForHwnd(cmdQueue, hwnd, &swapDesc, nullptr, nullptr, &tempSwapChain), "Failed to create temp swap chain");
		AX_THROW_IF_FAILED_HR(tempSwapChain.As(&m_swapChain), "Failed to transfer swap chain");
		
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
		AX_CORE_LOG_TRACE("Successfully created swap chain");

		// create RTVs
		for (UINT i = 0; i < spec.bufferCount; ++i) {
			AX_THROW_IF_FAILED_HR(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_backBuffers[i])), "Failed to get back buffer");

			m_rtvHeapIndices[i] = rtvHeap.allocate();
			D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeap.getCpuHandle(m_rtvHeapIndices[i]);

			device->CreateRenderTargetView(m_backBuffers[i].Get(), nullptr, handle);

			#ifdef AX_DEBUG
			wchar_t name[64];
			swprintf_s(name, L"SwapChainBackBuffer[%u]", i);
			m_backBuffers[i]->SetName(name);
			#endif
		}
		AX_CORE_LOG_TRACE("Successfully created {0} RTVs for the swap chain", spec.bufferCount);

		// create DSVs
		DXGI_FORMAT depthFormat = D12Helpers::toD12DepthStencilFormat(spec.depthBufferFormat);
		for (UINT i = 0; i < spec.bufferCount; ++i) {
			CD3DX12_RESOURCE_DESC depthDesc = CD3DX12_RESOURCE_DESC::Tex2D(
				depthFormat,
				spec.width,
				spec.height,
				1, 1
			);
			depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

			D3D12_CLEAR_VALUE depthClearValue = {};
			depthClearValue.Format = depthFormat;
			depthClearValue.DepthStencil.Depth = 1.0f;
			depthClearValue.DepthStencil.Stencil = 0;

			CD3DX12_HEAP_PROPERTIES depthHeapProps(D3D12_HEAP_TYPE_DEFAULT);
			HRESULT hr = device->CreateCommittedResource(
				&depthHeapProps,
				D3D12_HEAP_FLAG_NONE,
				&depthDesc,
				D3D12_RESOURCE_STATE_DEPTH_WRITE,
				&depthClearValue,
				IID_PPV_ARGS(&m_depthBuffers[i])
			);
			AX_THROW_IF_FAILED_HR(hr, "Failed to create depth buffer");

			m_dsvHeapIndices[i] = dsvHeap.allocate();
			D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap.getCpuHandle(m_dsvHeapIndices[i]);

			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
			dsvDesc.Format = depthFormat;
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

			device->CreateDepthStencilView(
				m_depthBuffers[i].Get(),
				&dsvDesc,
				dsvHandle
			);

			#ifdef AX_DEBUG
			wchar_t name[64];
			swprintf_s(name, L"SwapChainDepthBuffer[%u]", i);
			m_depthBuffers[i]->SetName(name);
			#endif
		}
		AX_CORE_LOG_TRACE("Successfully created {0} DSVs for the swap chain", spec.bufferCount);

	}

	void D12SwapChain::release() {
		for (auto& buffer : m_backBuffers) { buffer.Reset(); }
		for (auto& buffer : m_depthBuffers) { buffer.Reset(); }

		auto& rtvHeap = m_context->getRtvHeapWrapper();
		auto& dsvHeap = m_context->getDsvHeapWrapper();
		for (UINT i = 0; i < m_specification.bufferCount; ++i) {
			rtvHeap.free(m_rtvHeapIndices[i]);
			dsvHeap.free(m_dsvHeapIndices[i]);
		}

		m_swapChain.Reset();
	}

	void D12SwapChain::resize(UINT width, UINT height) {
		if (width == 0 || height == 0) return;
		m_specification.width = width;
		m_specification.height = height;

		auto* device = m_context->getDevice();
		auto& rtvHeap = m_context->getRtvHeapWrapper();
		auto& dsvHeap = m_context->getDsvHeapWrapper();

		// release existing resources
		for (auto& buffer : m_backBuffers) { buffer.Reset(); }
		for (auto& buffer : m_depthBuffers) { buffer.Reset(); }

		// get current swap chain description to preserve format/flags
		DXGI_SWAP_CHAIN_DESC1 desc = {};
		if (FAILED(m_swapChain->GetDesc1(&desc))) {
			AX_CORE_LOG_ERROR("Failed to get swap chain description");
			return;
		}

		// resize swap chain buffers
		HRESULT hr = m_swapChain->ResizeBuffers(m_specification.bufferCount, width, height, desc.Format, desc.Flags);
		AX_THROW_IF_FAILED_HR(hr, "Failed to resize swap chain buffers");

		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

		// recreate RTVs
		for (UINT i = 0; i < m_specification.bufferCount; ++i) {
			Microsoft::WRL::ComPtr<ID3D12Resource> backBuffer;
			hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer));
			AX_THROW_IF_FAILED_HR(hr, "Failed to get buffer from swap chain");

			D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeap.getCpuHandle(m_rtvHeapIndices[i]);
			device->CreateRenderTargetView(backBuffer.Get(), nullptr, handle);
			m_backBuffers[i] = backBuffer;
		}

		// Recreate DSVs
		DXGI_FORMAT depthFormat = D12Helpers::toD12DepthStencilFormat(m_specification.depthBufferFormat);
		for (UINT i = 0; i < m_specification.bufferCount; ++i) {
			CD3DX12_RESOURCE_DESC depthDesc = CD3DX12_RESOURCE_DESC::Tex2D(
				depthFormat,
				width,
				height,
				1, 1
			);
			depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

			D3D12_CLEAR_VALUE depthClearValue = {};
			depthClearValue.Format = depthFormat;
			depthClearValue.DepthStencil.Depth = 1.0f;
			depthClearValue.DepthStencil.Stencil = 0;

			CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
			hr = device->CreateCommittedResource(
				&heapProps,
				D3D12_HEAP_FLAG_NONE,
				&depthDesc,
				D3D12_RESOURCE_STATE_DEPTH_WRITE,
				&depthClearValue,
				IID_PPV_ARGS(&m_depthBuffers[i])
			);
			AX_THROW_IF_FAILED_HR(hr, "Failed to create resized depth buffer");

			D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap.getCpuHandle(m_dsvHeapIndices[i]);

			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
			dsvDesc.Format = depthFormat;
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

			device->CreateDepthStencilView(m_depthBuffers[i].Get(), &dsvDesc, dsvHandle);
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

	D3D12_CPU_DESCRIPTOR_HANDLE D12SwapChain::getDepthBufferDsv(uint32_t index) const {
		return m_context->getDsvHeapWrapper().getCpuHandle(m_dsvHeapIndices[index]);
	}


	void D12SwapChain::setAsRenderTarget() {
		auto* cmdList = m_context->getCommandList();
		auto rtvHandle = m_context->getRtvHeapWrapper().getCpuHandle(m_rtvHeapIndices[m_frameIndex]);
		auto dsvHandle = m_context->getDsvHeapWrapper().getCpuHandle(m_dsvHeapIndices[m_frameIndex]);
		cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
	}

	void D12SwapChain::clear(const float clearColor[4]) {
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = getBackBufferRtv(m_frameIndex);
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = getDepthBufferDsv(m_frameIndex);

		auto* cmdList = m_context->getCommandList();

		cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
		cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}

}
