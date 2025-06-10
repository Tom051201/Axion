#include "axpch.h"
#include "D12RenderTarget.h"

#include "Axion/Core.h"
#include "AxionSettings.h"

namespace Axion {

	D12RenderTarget::~D12RenderTarget() {
		release();
	}

	void D12RenderTarget::initialize(ID3D12Device* device, IDXGISwapChain3* swapChain) {

		AX_ASSERT(device, "ID3D12Device is null");
		AX_ASSERT(swapChain, "IDXGISwapChain3 is null");

		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = m_frameCount;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		AX_THROW_IF_FAILED_HR(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_rtvHeap)), "Failed to create RTV heap");
		AX_CORE_LOG_TRACE("Successfully created RTV heap");

		m_rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		
		for (UINT i = 0; i < m_frameCount; ++i) {
			AX_THROW_IF_FAILED_HR(swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])), "Failed to create render target");

			CD3DX12_CPU_DESCRIPTOR_HANDLE handle( m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), i, m_rtvDescriptorSize );

			device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, handle);
			AX_CORE_LOG_TRACE("Successfully created render target {0}", i);

			#ifdef AX_DEBUG
			wchar_t name[64];
			swprintf_s(name, L"RTV_RenderTarget[%u]", i);
			m_renderTargets[i]->SetName(name);
			#endif
		}

		AX_CORE_LOG_TRACE("RTV Heap created with {0} descriptors", m_frameCount);
	}

	void D12RenderTarget::release() {
		resetRTVs();
		m_rtvHeap.Reset();
	}

	void D12RenderTarget::resetRTVs() {
		for (UINT i = 0; i < m_frameCount; ++i) {
			m_renderTargets[i].Reset();
		}
	}

	void D12RenderTarget::resize(ID3D12Device* device, IDXGISwapChain3* swapChain) {
		resetRTVs();
		initialize(device, swapChain);
	}

}
