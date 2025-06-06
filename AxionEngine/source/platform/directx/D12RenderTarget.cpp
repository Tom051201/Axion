#include "axpch.h"
#include "D12RenderTarget.h"

#include "Axion/Core.h"
#include "AxionSettings.h"

namespace Axion {

	D12RenderTarget::~D12RenderTarget() {
		release();
	}

	void D12RenderTarget::initialize(ID3D12Device* device, IDXGISwapChain3* swapChain) {
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = m_frameCount;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		AX_THROW_IF_FAILED_HR(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_rtvHeap)), "Failed to create RTV heap");
		AX_CORE_LOG_INFO("Successfully created RTV heap");

		m_rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		D3D12_CPU_DESCRIPTOR_HANDLE handle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();

		for (UINT i = 0; i < m_frameCount; ++i) {
			AX_THROW_IF_FAILED_HR(swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])), "Failed to create render target");
			device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, handle);
			handle.ptr += m_rtvDescriptorSize;
			AX_CORE_LOG_INFO("Successfully created render target {0}", i);
		}
	}

	void D12RenderTarget::release() {
		if (m_rtvHeap.Get()) { m_rtvHeap.Reset(); }
		for (UINT i = 0; i < m_frameCount; ++i) {
			if (m_renderTargets[i].Get()) { m_renderTargets[i].Reset(); }
		}
		
	}

	void D12RenderTarget::resetRTVs() {
		for (UINT i = 0; i < m_frameCount; ++i) {
			if (m_renderTargets[i].Get()) { m_renderTargets[i].Reset(); }
		}
	}

}
